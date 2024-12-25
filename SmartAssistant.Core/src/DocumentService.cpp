#include "DocumentService.h"
#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <mutex>
#include <queue>
#include <thread>

namespace SmartAssistant {
namespace Core {

using json = nlohmann::json;

class DocumentService::Impl {
public:
    Impl() : isInitialized_(false) {
        InitializeDatabase();
    }

    ~Impl() {
        if (sqlite_) {
            sqlite3_close(sqlite_);
        }
    }

private:
    void InitializeDatabase() {
        if (sqlite3_open("documents.db", &sqlite_) != SQLITE_OK) {
            return;
        }

        const char* createDocumentsTable = R"(
            CREATE TABLE IF NOT EXISTS documents (
                document_id TEXT PRIMARY KEY,
                title TEXT NOT NULL,
                type INTEGER NOT NULL,
                owner_id TEXT NOT NULL,
                created_time INTEGER NOT NULL,
                modified_time INTEGER NOT NULL,
                current_version TEXT NOT NULL
            );
        )";

        const char* createVersionsTable = R"(
            CREATE TABLE IF NOT EXISTS versions (
                version_id TEXT PRIMARY KEY,
                document_id TEXT NOT NULL,
                user_id TEXT NOT NULL,
                timestamp INTEGER NOT NULL,
                description TEXT,
                diff BLOB,
                FOREIGN KEY(document_id) REFERENCES documents(document_id)
            );
        )";

        const char* createPermissionsTable = R"(
            CREATE TABLE IF NOT EXISTS permissions (
                document_id TEXT NOT NULL,
                user_id TEXT NOT NULL,
                permission INTEGER NOT NULL,
                PRIMARY KEY(document_id, user_id),
                FOREIGN KEY(document_id) REFERENCES documents(document_id)
            );
        )";

        char* errMsg = nullptr;
        if (sqlite3_exec(sqlite_, createDocumentsTable, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            sqlite3_free(errMsg);
            return;
        }

        if (sqlite3_exec(sqlite_, createVersionsTable, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            sqlite3_free(errMsg);
            return;
        }

        if (sqlite3_exec(sqlite_, createPermissionsTable, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            sqlite3_free(errMsg);
            return;
        }

        isInitialized_ = true;
    }

public:
    Result<std::string> CreateDocument(const std::string& title, 
                                     DocumentType type,
                                     const std::vector<uint8_t>& content) {
        if (!isInitialized_) {
            return Result<std::string>(Error(static_cast<int>(StatusCode::DatabaseError), 
                "Database not initialized"));
        }

        try {
            std::string documentId = GenerateUUID();
            std::string versionId = GenerateUUID();
            int64_t timestamp = GetCurrentTimestamp();

            // Begin transaction
            char* errMsg = nullptr;
            if (sqlite3_exec(sqlite_, "BEGIN TRANSACTION", nullptr, nullptr, &errMsg) != SQLITE_OK) {
                std::string error = errMsg;
                sqlite3_free(errMsg);
                return Result<std::string>(Error(static_cast<int>(StatusCode::DatabaseError), error));
            }

            // Insert document
            sqlite3_stmt* stmt;
            const char* insertDoc = R"(
                INSERT INTO documents (document_id, title, type, owner_id, created_time, 
                                     modified_time, current_version)
                VALUES (?, ?, ?, ?, ?, ?, ?);
            )";

            if (sqlite3_prepare_v2(sqlite_, insertDoc, -1, &stmt, nullptr) != SQLITE_OK) {
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<std::string>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }

            sqlite3_bind_text(stmt, 1, documentId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, title.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 3, static_cast<int>(type));
            sqlite3_bind_text(stmt, 4, currentUserId_.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 5, timestamp);
            sqlite3_bind_int64(stmt, 6, timestamp);
            sqlite3_bind_text(stmt, 7, versionId.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<std::string>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }
            sqlite3_finalize(stmt);


            // Insert initial version
            const char* insertVersion = R"(
                INSERT INTO versions (version_id, document_id, user_id, timestamp, description, diff)
                VALUES (?, ?, ?, ?, ?, ?);
            )";

            if (sqlite3_prepare_v2(sqlite_, insertVersion, -1, &stmt, nullptr) != SQLITE_OK) {
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<std::string>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }

            sqlite3_bind_text(stmt, 1, versionId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, documentId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, currentUserId_.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 4, timestamp);
            sqlite3_bind_text(stmt, 5, "Initial version", -1, SQLITE_STATIC);
            sqlite3_bind_blob(stmt, 6, content.data(), content.size(), SQLITE_STATIC);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<std::string>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }
            sqlite3_finalize(stmt);

            // Insert owner permission
            const char* insertPerm = R"(
                INSERT INTO permissions (document_id, user_id, permission)
                VALUES (?, ?, ?);
            )";

            if (sqlite3_prepare_v2(sqlite_, insertPerm, -1, &stmt, nullptr) != SQLITE_OK) {
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<std::string>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }

            sqlite3_bind_text(stmt, 1, documentId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, currentUserId_.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 3, static_cast<int>(DocumentPermission::Write));

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<std::string>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }
            sqlite3_finalize(stmt);

            // Commit transaction
            if (sqlite3_exec(sqlite_, "COMMIT", nullptr, nullptr, &errMsg) != SQLITE_OK) {
                std::string error = errMsg;
                sqlite3_free(errMsg);
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<std::string>(Error(static_cast<int>(StatusCode::DatabaseError), error));
            }

            return Result<std::string>(documentId);
        }
        catch (const std::exception& e) {
            sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
            return Result<std::string>(Error(static_cast<int>(StatusCode::UnknownError), e.what()));
        }
    }

    Result<bool> OpenDocument(const std::string& documentId) {
        if (!isInitialized_) {
            return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), 
                "Database not initialized"));
        }

        try {
            // Check permissions
            auto permResult = CheckPermission(documentId, currentUserId_);
            if (!permResult.IsSuccess()) {
                return Result<bool>(permResult.GetError());
            }

            auto permission = permResult.GetValue();
            if (permission < DocumentPermission::Read) {
                return Result<bool>(Error(static_cast<int>(StatusCode::PermissionDenied), 
                    "No permission to open document"));
            }

            openDocuments_.insert(documentId);
            return Result<bool>(true);
        }
        catch (const std::exception& e) {
            return Result<bool>(Error(static_cast<int>(StatusCode::UnknownError), e.what()));
        }
    }

    Result<bool> CloseDocument(const std::string& documentId) {
        openDocuments_.erase(documentId);
        return Result<bool>(true);
    }

    Result<std::vector<uint8_t>> GetContent(const std::string& documentId) {
        if (!isInitialized_) {
            return Result<std::vector<uint8_t>>(
                Error(static_cast<int>(StatusCode::DatabaseError), "Database not initialized"));
        }

        try {
            // Check if document is open
            if (openDocuments_.find(documentId) == openDocuments_.end()) {
                return Result<std::vector<uint8_t>>(
                    Error(static_cast<int>(StatusCode::InvalidOperation), "Document not open"));
            }

            // Get current version
            sqlite3_stmt* stmt;
            const char* query = R"(
                SELECT v.diff
                FROM documents d
                JOIN versions v ON d.current_version = v.version_id
                WHERE d.document_id = ?;
            )";

            if (sqlite3_prepare_v2(sqlite_, query, -1, &stmt, nullptr) != SQLITE_OK) {
                return Result<std::vector<uint8_t>>(
                    Error(static_cast<int>(StatusCode::DatabaseError), sqlite3_errmsg(sqlite_)));
            }

            sqlite3_bind_text(stmt, 1, documentId.c_str(), -1, SQLITE_STATIC);

            std::vector<uint8_t> content;
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                const void* data = sqlite3_column_blob(stmt, 0);
                int size = sqlite3_column_bytes(stmt, 0);
                content.assign(static_cast<const uint8_t*>(data),
                             static_cast<const uint8_t*>(data) + size);
            }

            sqlite3_finalize(stmt);
            return Result<std::vector<uint8_t>>(content);
        }
        catch (const std::exception& e) {
            return Result<std::vector<uint8_t>>(
                Error(static_cast<int>(StatusCode::UnknownError), e.what()));
        }
    }

    Result<bool> UpdateContent(const std::string& documentId, 
                             const std::vector<uint8_t>& diff) {
        if (!isInitialized_) {
            return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), 
                "Database not initialized"));
        }

        try {
            // Check permissions
            auto permResult = CheckPermission(documentId, currentUserId_);
            if (!permResult.IsSuccess()) {
                return Result<bool>(permResult.GetError());
            }

            auto permission = permResult.GetValue();
            if (permission < DocumentPermission::Write) {
                return Result<bool>(Error(static_cast<int>(StatusCode::PermissionDenied), 
                    "No permission to modify document"));
            }

            // Create new version
            std::string versionId = GenerateUUID();
            int64_t timestamp = GetCurrentTimestamp();

            // Begin transaction
            char* errMsg = nullptr;
            if (sqlite3_exec(sqlite_, "BEGIN TRANSACTION", nullptr, nullptr, &errMsg) != SQLITE_OK) {
                std::string error = errMsg;
                sqlite3_free(errMsg);
                return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), error));
            }

            // Insert new version
            sqlite3_stmt* stmt;
            const char* insertVersion = R"(
                INSERT INTO versions (version_id, document_id, user_id, timestamp, description, diff)
                VALUES (?, ?, ?, ?, ?, ?);
            )";

            if (sqlite3_prepare_v2(sqlite_, insertVersion, -1, &stmt, nullptr) != SQLITE_OK) {
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }

            sqlite3_bind_text(stmt, 1, versionId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, documentId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, currentUserId_.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 4, timestamp);
            sqlite3_bind_text(stmt, 5, "Content update", -1, SQLITE_STATIC);
            sqlite3_bind_blob(stmt, 6, diff.data(), diff.size(), SQLITE_STATIC);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }
            sqlite3_finalize(stmt);

            // Update document
            const char* updateDoc = R"(
                UPDATE documents
                SET current_version = ?, modified_time = ?
                WHERE document_id = ?;
            )";

            if (sqlite3_prepare_v2(sqlite_, updateDoc, -1, &stmt, nullptr) != SQLITE_OK) {
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }

            sqlite3_bind_text(stmt, 1, versionId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 2, timestamp);
            sqlite3_bind_text(stmt, 3, documentId.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }
            sqlite3_finalize(stmt);

            // Commit transaction
            if (sqlite3_exec(sqlite_, "COMMIT", nullptr, nullptr, &errMsg) != SQLITE_OK) {
                std::string error = errMsg;
                sqlite3_free(errMsg);
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), error));
            }

            // Notify callbacks
            NotifyDocumentChanged(documentId, diff);

            return Result<bool>(true);
        }
        catch (const std::exception& e) {
            sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
            return Result<bool>(Error(static_cast<int>(StatusCode::UnknownError), e.what()));
        }
    }

    Result<DocumentMetadata> GetMetadata(const std::string& documentId) {
        if (!isInitialized_) {
            return Result<DocumentMetadata>(Error(static_cast<int>(StatusCode::DatabaseError), 
                "Database not initialized"));
        }

        try {
            sqlite3_stmt* stmt;
            const char* query = R"(
                SELECT title, type, owner_id, created_time, modified_time, current_version
                FROM documents
                WHERE document_id = ?;
            )";

            if (sqlite3_prepare_v2(sqlite_, query, -1, &stmt, nullptr) != SQLITE_OK) {
                return Result<DocumentMetadata>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }

            sqlite3_bind_text(stmt, 1, documentId.c_str(), -1, SQLITE_STATIC);

            DocumentMetadata metadata;
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                metadata.documentId = documentId;
                metadata.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                metadata.type = static_cast<DocumentType>(sqlite3_column_int(stmt, 1));
                metadata.ownerId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                metadata.createdTime = sqlite3_column_int64(stmt, 3);
                metadata.modifiedTime = sqlite3_column_int64(stmt, 4);
                metadata.currentVersion = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            }

            sqlite3_finalize(stmt);

            // Get permissions
            const char* permQuery = R"(
                SELECT user_id, permission
                FROM permissions
                WHERE document_id = ?;
            )";

            if (sqlite3_prepare_v2(sqlite_, permQuery, -1, &stmt, nullptr) != SQLITE_OK) {
                return Result<DocumentMetadata>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }

            sqlite3_bind_text(stmt, 1, documentId.c_str(), -1, SQLITE_STATIC);

            while (sqlite3_step(stmt) == SQLITE_ROW) {
                std::string userId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                DocumentPermission perm = static_cast<DocumentPermission>(sqlite3_column_int(stmt, 1));
                metadata.userPermissions[userId] = perm;
            }

            sqlite3_finalize(stmt);
            return Result<DocumentMetadata>(metadata);
        }
        catch (const std::exception& e) {
            return Result<DocumentMetadata>(Error(static_cast<int>(StatusCode::UnknownError), 
                e.what()));
        }
    }

    Result<std::vector<DocumentVersion>> GetVersionHistory(const std::string& documentId) {
        if (!isInitialized_) {
            return Result<std::vector<DocumentVersion>>(
                Error(static_cast<int>(StatusCode::DatabaseError), "Database not initialized"));
        }

        try {
            sqlite3_stmt* stmt;
            const char* query = R"(
                SELECT version_id, user_id, timestamp, description, diff
                FROM versions
                WHERE document_id = ?
                ORDER BY timestamp DESC;
            )";

            if (sqlite3_prepare_v2(sqlite_, query, -1, &stmt, nullptr) != SQLITE_OK) {
                return Result<std::vector<DocumentVersion>>(
                    Error(static_cast<int>(StatusCode::DatabaseError), sqlite3_errmsg(sqlite_)));
            }

            sqlite3_bind_text(stmt, 1, documentId.c_str(), -1, SQLITE_STATIC);

            std::vector<DocumentVersion> versions;
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                DocumentVersion version;
                version.versionId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                version.userId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                version.timestamp = sqlite3_column_int64(stmt, 2);
                version.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

                const void* data = sqlite3_column_blob(stmt, 4);
                int size = sqlite3_column_bytes(stmt, 4);
                version.diff.assign(static_cast<const uint8_t*>(data),
                                  static_cast<const uint8_t*>(data) + size);

                versions.push_back(version);
            }

            sqlite3_finalize(stmt);
            return Result<std::vector<DocumentVersion>>(versions);
        }
        catch (const std::exception& e) {
            return Result<std::vector<DocumentVersion>>(
                Error(static_cast<int>(StatusCode::UnknownError), e.what()));
        }
    }

    Result<bool> RevertToVersion(const std::string& documentId, 
                               const std::string& versionId) {
        if (!isInitialized_) {
            return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), 
                "Database not initialized"));
        }

        try {
            // Check permissions
            auto permResult = CheckPermission(documentId, currentUserId_);
            if (!permResult.IsSuccess()) {
                return Result<bool>(permResult.GetError());
            }

            auto permission = permResult.GetValue();
            if (permission < DocumentPermission::Write) {
                return Result<bool>(Error(static_cast<int>(StatusCode::PermissionDenied), 
                    "No permission to modify document"));
            }

            // Begin transaction
            char* errMsg = nullptr;
            if (sqlite3_exec(sqlite_, "BEGIN TRANSACTION", nullptr, nullptr, &errMsg) != SQLITE_OK) {
                std::string error = errMsg;
                sqlite3_free(errMsg);
                return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), error));
            }

            // Update document
            sqlite3_stmt* stmt;
            const char* updateDoc = R"(
                UPDATE documents
                SET current_version = ?, modified_time = ?
                WHERE document_id = ?;
            )";

            if (sqlite3_prepare_v2(sqlite_, updateDoc, -1, &stmt, nullptr) != SQLITE_OK) {
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }

            int64_t timestamp = GetCurrentTimestamp();
            sqlite3_bind_text(stmt, 1, versionId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 2, timestamp);
            sqlite3_bind_text(stmt, 3, documentId.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }
            sqlite3_finalize(stmt);

            // Get version content
            const char* getVersion = R"(
                SELECT diff
                FROM versions
                WHERE version_id = ?;
            )";

            if (sqlite3_prepare_v2(sqlite_, getVersion, -1, &stmt, nullptr) != SQLITE_OK) {
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }

            sqlite3_bind_text(stmt, 1, versionId.c_str(), -1, SQLITE_STATIC);

            std::vector<uint8_t> diff;
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                const void* data = sqlite3_column_blob(stmt, 0);
                int size = sqlite3_column_bytes(stmt, 0);
                diff.assign(static_cast<const uint8_t*>(data),
                          static_cast<const uint8_t*>(data) + size);
            }

            sqlite3_finalize(stmt);

            // Commit transaction
            if (sqlite3_exec(sqlite_, "COMMIT", nullptr, nullptr, &errMsg) != SQLITE_OK) {
                std::string error = errMsg;
                sqlite3_free(errMsg);
                sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
                return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), error));
            }

            // Notify callbacks
            NotifyDocumentChanged(documentId, diff);

            return Result<bool>(true);
        }
        catch (const std::exception& e) {
            sqlite3_exec(sqlite_, "ROLLBACK", nullptr, nullptr, nullptr);
            return Result<bool>(Error(static_cast<int>(StatusCode::UnknownError), e.what()));
        }
    }

    Result<bool> SetPermission(const std::string& documentId,
                             const std::string& userId,
                             DocumentPermission permission) {
        if (!isInitialized_) {
            return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), 
                "Database not initialized"));
        }

        try {
            // Check if current user is owner
            sqlite3_stmt* stmt;
            const char* checkOwner = R"(
                SELECT 1
                FROM documents
                WHERE document_id = ? AND owner_id = ?;
            )";

            if (sqlite3_prepare_v2(sqlite_, checkOwner, -1, &stmt, nullptr) != SQLITE_OK) {
                return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }

            sqlite3_bind_text(stmt, 1, documentId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, currentUserId_.c_str(), -1, SQLITE_STATIC);

            bool isOwner = sqlite3_step(stmt) == SQLITE_ROW;
            sqlite3_finalize(stmt);

            if (!isOwner) {
                return Result<bool>(Error(static_cast<int>(StatusCode::PermissionDenied), 
                    "Only owner can set permissions"));
            }

            // Update permission
            const char* upsertPerm = R"(
                INSERT INTO permissions (document_id, user_id, permission)
                VALUES (?, ?, ?)
                ON CONFLICT(document_id, user_id) DO UPDATE SET permission = ?;
            )";

            if (sqlite3_prepare_v2(sqlite_, upsertPerm, -1, &stmt, nullptr) != SQLITE_OK) {
                return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }

            sqlite3_bind_text(stmt, 1, documentId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, userId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 3, static_cast<int>(permission));
            sqlite3_bind_int(stmt, 4, static_cast<int>(permission));


            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                return Result<bool>(Error(static_cast<int>(StatusCode::DatabaseError), 
                    sqlite3_errmsg(sqlite_)));
            }

            sqlite3_finalize(stmt);

            // Notify callbacks
            NotifyPermissionChanged(documentId, userId, permission);

            return Result<bool>(true);
        }
        catch (const std::exception& e) {
            return Result<bool>(Error(static_cast<int>(StatusCode::UnknownError), e.what()));
        }
    }

    Result<bool> ShareDocument(const std::string& documentId,
                             const std::string& userId,
                             DocumentPermission permission) {
        return SetPermission(documentId, userId, permission);
    }

    void RegisterCallback(std::shared_ptr<IDocumentCallback> callback) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callbacks_.push_back(callback);
    }

    void UnregisterCallback(std::shared_ptr<IDocumentCallback> callback) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callbacks_.erase(
            std::remove_if(callbacks_.begin(), callbacks_.end(),
                [&](const std::shared_ptr<IDocumentCallback>& cb) {
                    return cb == callback;
                }
            ),
            callbacks_.end()
        );
    }

private:
    Result<DocumentPermission> CheckPermission(const std::string& documentId, 
                                             const std::string& userId) {
        sqlite3_stmt* stmt;
        const char* query = R"(
            SELECT permission
            FROM permissions
            WHERE document_id = ? AND user_id = ?;
        )";

        if (sqlite3_prepare_v2(sqlite_, query, -1, &stmt, nullptr) != SQLITE_OK) {
            return Result<DocumentPermission>(Error(static_cast<int>(StatusCode::DatabaseError), 
                sqlite3_errmsg(sqlite_)));
        }

        sqlite3_bind_text(stmt, 1, documentId.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, userId.c_str(), -1, SQLITE_STATIC);

        DocumentPermission permission = DocumentPermission::Read;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            permission = static_cast<DocumentPermission>(sqlite3_column_int(stmt, 0));
        }

        sqlite3_finalize(stmt);
        return Result<DocumentPermission>(permission);
    }

    void NotifyDocumentChanged(const std::string& documentId, 
                             const std::vector<uint8_t>& diff) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        for (const auto& callback : callbacks_) {
            callback->OnDocumentChanged(documentId, diff);
        }
    }

    void NotifyPermissionChanged(const std::string& documentId,
                               const std::string& userId,
                               DocumentPermission permission) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        for (const auto& callback : callbacks_) {
            callback->OnPermissionChanged(documentId, userId, permission);
        }
    }

    std::string GenerateUUID() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static const char* chars = "0123456789abcdef";

        std::string uuid;
        uuid.reserve(36);

        for (int i = 0; i < 8; i++) uuid += chars[dis(gen)];
        uuid += "-";
        for (int i = 0; i < 4; i++) uuid += chars[dis(gen)];
        uuid += "-4";
        for (int i = 0; i < 3; i++) uuid += chars[dis(gen)];
        uuid += "-";
        uuid += chars[(dis(gen) & 0x3) | 0x8];
        for (int i = 0; i < 3; i++) uuid += chars[dis(gen)];
        uuid += "-";
        for (int i = 0; i < 12; i++) uuid += chars[dis(gen)];

        return uuid;
    }

    int64_t GetCurrentTimestamp() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    sqlite3* sqlite_;
    bool isInitialized_;
    std::string currentUserId_;
    std::set<std::string> openDocuments_;
    std::vector<std::shared_ptr<IDocumentCallback>> callbacks_;
    std::mutex callbackMutex_;
};

// DocumentService implementation
DocumentService::DocumentService() : pImpl(std::make_unique<Impl>()) {}
DocumentService::~DocumentService() = default;

Result<std::string> DocumentService::CreateDocument(const std::string& title,
                                                  DocumentType type,
                                                  const std::vector<uint8_t>& content) {
    return pImpl->CreateDocument(title, type, content);
}

Result<bool> DocumentService::OpenDocument(const std::string& documentId) {
    return pImpl->OpenDocument(documentId);
}

Result<bool> DocumentService::CloseDocument(const std::string& documentId) {
    return pImpl->CloseDocument(documentId);
}

Result<std::vector<uint8_t>> DocumentService::GetContent(const std::string& documentId) {
    return pImpl->GetContent(documentId);
}

Result<bool> DocumentService::UpdateContent(const std::string& documentId,
                                          const std::vector<uint8_t>& diff) {
    return pImpl->UpdateContent(documentId, diff);
}

Result<DocumentMetadata> DocumentService::GetMetadata(const std::string& documentId) {
    return pImpl->GetMetadata(documentId);
}

Result<std::vector<DocumentVersion>> DocumentService::GetVersionHistory(
    const std::string& documentId) {
    return pImpl->GetVersionHistory(documentId);
}

Result<bool> DocumentService::RevertToVersion(const std::string& documentId,
                                            const std::string& versionId) {
    return pImpl->RevertToVersion(documentId, versionId);
}

Result<bool> DocumentService::SetPermission(const std::string& documentId,
                                          const std::string& userId,
                                          DocumentPermission permission) {
    return pImpl->SetPermission(documentId, userId, permission);
}

Result<bool> DocumentService::ShareDocument(const std::string& documentId,
                                          const std::string& userId,
                                          DocumentPermission permission) {
    return pImpl->ShareDocument(documentId, userId, permission);
}

void DocumentService::RegisterCallback(std::shared_ptr<IDocumentCallback> callback) {
    pImpl->RegisterCallback(callback);
}

void DocumentService::UnregisterCallback(std::shared_ptr<IDocumentCallback> callback) {
    pImpl->UnregisterCallback(callback);
}

} // namespace Core
} // namespace SmartAssistant
