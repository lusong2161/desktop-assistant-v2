#include "DocumentService.h"

namespace SmartAssistant {
    DocumentService::DocumentService() {}
    DocumentService::~DocumentService() {}
    
    bool DocumentService::OpenDocument(const std::string& path) {
        openDocuments_[path] = "1.0";
        return true;
    }
    
    bool DocumentService::ShareDocument(const std::string& docId, const std::string& userId) {
        if (openDocuments_.find(docId) == openDocuments_.end()) return false;
        return true;
    }
    
    std::string DocumentService::GetDocumentVersion(const std::string& docId) {
        if (openDocuments_.find(docId) == openDocuments_.end()) return "";
        return openDocuments_[docId];
    }
}
