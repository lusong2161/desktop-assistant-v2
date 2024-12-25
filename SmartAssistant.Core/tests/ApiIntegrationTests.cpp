#include <gtest/gtest.h>
#include "../include/ApiClient.h"
#include "../include/AIService.h"
#include "../include/AuthenticationManager.h"

// 智能对话系统测试
TEST(ApiIntegrationTests, AIServiceIntegration) {
    AIService aiService;
    EXPECT_TRUE(aiService.initialize());
    
    // Test AI dialogue capabilities
    std::string response = aiService.processCommand("你好");
    EXPECT_FALSE(response.empty());
}

// 用户通信测试
TEST(ApiIntegrationTests, UserCommunication) {
    AuthenticationManager authManager;
    EXPECT_TRUE(authManager.initialize());
    
    // Test user authentication
    EXPECT_TRUE(authManager.login("testuser", "password"));
    
    // Test message sending
    ApiClient apiClient;
    EXPECT_TRUE(apiClient.sendMessage("testuser", "测试消息"));
}

// API连接测试
TEST(ApiIntegrationTests, ApiConnection) {
    ApiClient apiClient;
    EXPECT_TRUE(apiClient.initialize());
    EXPECT_TRUE(apiClient.testConnection());
}

// 文件共享API测试
TEST(ApiIntegrationTests, FileSharing) {
    ApiClient apiClient;
    EXPECT_TRUE(apiClient.initialize());
    
    // Test file upload
    EXPECT_TRUE(apiClient.uploadFile("test.txt", "测试文件内容"));
    
    // Test file download
    std::string content;
    EXPECT_TRUE(apiClient.downloadFile("test.txt", content));
    EXPECT_FALSE(content.empty());
}

// Office文档管理API测试
TEST(ApiIntegrationTests, DocumentManagement) {
    ApiClient apiClient;
    EXPECT_TRUE(apiClient.initialize());
    
    // Test document creation
    EXPECT_TRUE(apiClient.createDocument("测试文档.docx"));
    
    // Test document sharing
    EXPECT_TRUE(apiClient.shareDocument("测试文档.docx", "testuser"));
    
    // Test version control
    EXPECT_TRUE(apiClient.createDocumentVersion("测试文档.docx", "1.0"));
}
