#pragma once
#include "Common.h"

namespace SmartAssistant {
    class DocumentService {
    public:
        DocumentService();
        ~DocumentService();
        
        // 协同办公文档管理核心功能
        bool OpenDocument(const std::string& path);
        bool ShareDocument(const std::string& docId, const std::string& userId);
        std::string GetDocumentVersion(const std::string& docId);
        
    private:
        std::map<std::string, std::string> openDocuments_;
    };
}
