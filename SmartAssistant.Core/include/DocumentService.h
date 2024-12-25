#pragma once

#include "Common.h"
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace SmartAssistant {
namespace Core {

enum class DocumentType {
    Word,
    Excel,
    PowerPoint,
    Unknown
};

enum class DocumentPermission {
    Read,
    Write,
    Comment,
    Share
};

struct DocumentVersion {
    std::string versionId;
    std::string userId;
    int64_t timestamp;
    std::string description;
    std::vector<uint8_t> diff;
};

struct DocumentMetadata {
    std::string documentId;
    std::string title;
    DocumentType type;
    std::string ownerId;
    int64_t createdTime;
    int64_t modifiedTime;
    std::string currentVersion;
    std::map<std::string, DocumentPermission> userPermissions;
};

class IDocumentCallback {
public:
    virtual ~IDocumentCallback() = default;
    virtual void OnDocumentChanged(const std::string& documentId, 
                                 const std::vector<uint8_t>& diff) = 0;
    virtual void OnPermissionChanged(const std::string& documentId, 
                                   const std::string& userId,
                                   DocumentPermission permission) = 0;
    virtual void OnVersionCreated(const std::string& documentId, 
                                const DocumentVersion& version) = 0;
};

class DocumentService {
public:
    DocumentService();
    ~DocumentService();

    Result<std::string> CreateDocument(const std::string& title, 
                                     DocumentType type,
                                     const std::vector<uint8_t>& content);

    Result<bool> OpenDocument(const std::string& documentId);
    Result<bool> CloseDocument(const std::string& documentId);

    Result<std::vector<uint8_t>> GetContent(const std::string& documentId);
    Result<bool> UpdateContent(const std::string& documentId, 
                             const std::vector<uint8_t>& diff);

    Result<DocumentMetadata> GetMetadata(const std::string& documentId);
    Result<std::vector<DocumentVersion>> GetVersionHistory(const std::string& documentId);
    Result<bool> RevertToVersion(const std::string& documentId, 
                               const std::string& versionId);

    Result<bool> SetPermission(const std::string& documentId,
                             const std::string& userId,
                             DocumentPermission permission);

    Result<bool> ShareDocument(const std::string& documentId,
                             const std::string& userId,
                             DocumentPermission permission);

    void RegisterCallback(std::shared_ptr<IDocumentCallback> callback);
    void UnregisterCallback(std::shared_ptr<IDocumentCallback> callback);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace Core
} // namespace SmartAssistant
