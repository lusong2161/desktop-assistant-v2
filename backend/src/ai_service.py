from fastapi import APIRouter, HTTPException, Depends
from pydantic import BaseModel
import openai
import os
from typing import Optional, List
import json
import speech_recognition as sr
from gtts import gTTS
import tempfile
import base64

router = APIRouter()

# Configure OpenAI
openai.api_key = os.getenv("OPENAI_API_KEY")

class ChatMessage(BaseModel):
    role: str
    content: str

class ChatRequest(BaseModel):
    messages: List[ChatMessage]

class VoiceRequest(BaseModel):
    audio_base64: str

class TextToSpeechRequest(BaseModel):
    text: str

@router.post("/chat")
async def chat_completion(request: ChatRequest):
    try:
        messages = [{"role": msg.role, "content": msg.content} 
                   for msg in request.messages]
        
        response = await openai.ChatCompletion.acreate(
            model="gpt-4",
            messages=messages
        )
        
        return {
            "response": response.choices[0].message.content,
            "status": "success"
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/speech-to-text")
async def speech_to_text(request: VoiceRequest):
    try:
        # Decode base64 audio
        audio_data = base64.b64decode(request.audio_base64)
        
        # Save to temporary file
        with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as temp_file:
            temp_file.write(audio_data)
            temp_path = temp_file.name
        
        try:
            # Convert speech to text
            recognizer = sr.Recognizer()
            with sr.AudioFile(temp_path) as source:
                audio = recognizer.record(source)
                text = recognizer.recognize_google(audio)
                
            return {
                "text": text,
                "status": "success"
            }
        finally:
            # Clean up temp file
            os.unlink(temp_path)
            
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/text-to-speech")
async def text_to_speech(request: TextToSpeechRequest):
    try:
        # Convert text to speech
        tts = gTTS(text=request.text, lang='en')
        
        # Save to temporary file
        with tempfile.NamedTemporaryFile(suffix=".mp3", delete=False) as temp_file:
            tts.save(temp_file.name)
            temp_path = temp_file.name
            
            # Read the file and convert to base64
            with open(temp_path, "rb") as audio_file:
                audio_base64 = base64.b64encode(audio_file.read()).decode()
        
        # Clean up
        os.unlink(temp_path)
        
        return {
            "audio_base64": audio_base64,
            "status": "success"
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@router.post("/analyze-command")
async def analyze_command(text: str):
    try:
        # Use OpenAI to analyze the command
        response = await openai.ChatCompletion.acreate(
            model="gpt-4",
            messages=[
                {"role": "system", "content": "You are a command analyzer. "
                 "Identify the type of command and extract relevant parameters."},
                {"role": "user", "content": text}
            ]
        )
        
        # Parse the response
        analysis = response.choices[0].message.content
        
        # Convert to structured format
        try:
            command_info = json.loads(analysis)
        except:
            command_info = {
                "type": "unknown",
                "parameters": {},
                "original_text": text
            }
        
        return command_info
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
