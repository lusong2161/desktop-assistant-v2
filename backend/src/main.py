from fastapi import FastAPI, WebSocket, HTTPException, Depends, UploadFile, File
from fastapi.security import OAuth2PasswordBearer
from fastapi.middleware.cors import CORSMiddleware
import jwt
import sqlite3
import asyncio
import json
import os
from datetime import datetime, timedelta
from typing import List, Dict, Optional
import aiofiles
import hashlib
from pathlib import Path

app = FastAPI(title="Smart Assistant Backend")

# CORS configuration
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Configuration
SECRET_KEY = os.getenv("JWT_SECRET_KEY", "your-secret-key")
DATABASE_URL = os.getenv("DATABASE_URL", "sqlite:///./smartassistant.db")
UPLOAD_DIR = Path("uploads")
UPLOAD_DIR.mkdir(exist_ok=True)

# OAuth2 scheme
oauth2_scheme = OAuth2PasswordBearer(tokenUrl="token")

# WebSocket connections
active_connections: Dict[str, WebSocket] = {}
document_sessions: Dict[str, List[str]] = {}

# Database initialization
def init_db():
    conn = sqlite3.connect("./data/smartassistant.db")
    c = conn.cursor()
    
    # Users table
    c.execute("""
    CREATE TABLE IF NOT EXISTS users (
        id TEXT PRIMARY KEY,
        username TEXT UNIQUE,
        password_hash TEXT,
        created_at TIMESTAMP
    )
    """)
    
    # Documents table
    c.execute("""
    CREATE TABLE IF NOT EXISTS documents (
        id TEXT PRIMARY KEY,
        title TEXT,
        owner_id TEXT,
        content BLOB,
        created_at TIMESTAMP,
        updated_at TIMESTAMP,
        FOREIGN KEY (owner_id) REFERENCES users (id)
    )
    """)
    
    # Document versions
    c.execute("""
    CREATE TABLE IF NOT EXISTS document_versions (
        id TEXT PRIMARY KEY,
        document_id TEXT,
        content BLOB,
        created_at TIMESTAMP,
        created_by TEXT,
        FOREIGN KEY (document_id) REFERENCES documents (id),
        FOREIGN KEY (created_by) REFERENCES users (id)
    )
    """)
    
    # Document permissions
    c.execute("""
    CREATE TABLE IF NOT EXISTS document_permissions (
        document_id TEXT,
        user_id TEXT,
        permission TEXT,
        FOREIGN KEY (document_id) REFERENCES documents (id),
        FOREIGN KEY (user_id) REFERENCES users (id),
        PRIMARY KEY (document_id, user_id)
    )
    """)
    
    # Friend relationships
    c.execute("""
    CREATE TABLE IF NOT EXISTS friends (
        user_id TEXT,
        friend_id TEXT,
        status TEXT,
        created_at TIMESTAMP,
        FOREIGN KEY (user_id) REFERENCES users (id),
        FOREIGN KEY (friend_id) REFERENCES users (id),
        PRIMARY KEY (user_id, friend_id)
    )
    """)
    
    conn.commit()
    conn.close()

# Initialize database on startup
@app.on_event("startup")
async def startup_event():
    init_db()

# Authentication endpoints
@app.post("/token")
async def login(username: str, password: str):
    conn = sqlite3.connect("./data/smartassistant.db")
    c = conn.cursor()
    c.execute("SELECT id, password_hash FROM users WHERE username = ?", (username,))
    result = c.fetchone()
    conn.close()
    
    if not result or result[1] != hashlib.sha256(password.encode()).hexdigest():
        raise HTTPException(status_code=401, detail="Invalid credentials")
    
    token = jwt.encode(
        {"sub": result[0], "exp": datetime.utcnow() + timedelta(days=1)},
        SECRET_KEY,
        algorithm="HS256"
    )
    return {"access_token": token, "token_type": "bearer"}

# WebSocket connection handler
@app.websocket("/ws/{client_id}")
async def websocket_endpoint(websocket: WebSocket, client_id: str):
    await websocket.accept()
    active_connections[client_id] = websocket
    
    try:
        while True:
            data = await websocket.receive_json()
            if data["type"] == "document_update":
                # Broadcast document updates to all connected clients
                for user_id in document_sessions.get(data["document_id"], []):
                    if user_id != client_id and user_id in active_connections:
                        await active_connections[user_id].send_json(data)
    except:
        if client_id in active_connections:
            del active_connections[client_id]
        # Remove client from all document sessions
        for sessions in document_sessions.values():
            if client_id in sessions:
                sessions.remove(client_id)

# Document endpoints
@app.post("/documents")
async def create_document(
    title: str,
    content: bytes = File(...),
    current_user: str = Depends(oauth2_scheme)
):
    conn = sqlite3.connect("./data/smartassistant.db")
    c = conn.cursor()
    document_id = hashlib.sha256(f"{title}{datetime.now()}".encode()).hexdigest()
    
    c.execute("""
    INSERT INTO documents (id, title, owner_id, content, created_at, updated_at)
    VALUES (?, ?, ?, ?, ?, ?)
    """, (document_id, title, current_user, content, datetime.now(), datetime.now()))
    
    conn.commit()
    conn.close()
    return {"document_id": document_id}

@app.put("/documents/{document_id}/permissions")
async def set_document_permission(
    document_id: str,
    user_id: str,
    permission: str,
    current_user: str = Depends(oauth2_scheme)
):
    conn = sqlite3.connect("./data/smartassistant.db")
    c = conn.cursor()
    
    # Verify ownership
    c.execute("SELECT owner_id FROM documents WHERE id = ?", (document_id,))
    result = c.fetchone()
    if not result or result[0] != current_user:
        conn.close()
        raise HTTPException(status_code=403, detail="Not authorized")
    
    c.execute("""
    INSERT OR REPLACE INTO document_permissions (document_id, user_id, permission)
    VALUES (?, ?, ?)
    """, (document_id, user_id, permission))
    
    conn.commit()
    conn.close()
    return {"status": "success"}

# File transfer endpoints
@app.post("/upload")
async def upload_file(
    file: UploadFile = File(...),
    current_user: str = Depends(oauth2_scheme)
):
    file_path = UPLOAD_DIR / f"{current_user}_{file.filename}"
    async with aiofiles.open(file_path, 'wb') as out_file:
        content = await file.read()
        await out_file.write(content)
    
    return {"filename": file.filename, "path": str(file_path)}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
