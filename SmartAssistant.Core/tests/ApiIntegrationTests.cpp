#include <gtest/gtest.h>
#include "ApiClient.h"
#include "AuthenticationManager.h"
#include <thread>
#include <chrono>

using namespace SmartAssistant::Core;

class ApiIntegrationTests : public ::testing::Test {
protected:
    void SetUp() override {
        apiClient = std::make_unique<ApiClient>();
        authManager = std::make_unique<AuthenticationManager>();
        
        // Initialize API client
        apiClient->Initialize("http://localhost:8080/api");
    }
    
    void WaitForAsync(int milliseconds = 1000) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }
    
    std::unique_ptr<ApiClient> apiClient;
    std::unique_ptr<AuthenticationManager> authManager;
};

TEST_F(ApiIntegrationTests, AuthenticationEndpoints) {
    // Test login
    auto loginResult = authManager->Login("testUser", "testPassword");
    ASSERT_TRUE(loginResult.IsSuccess());
    auto token = loginResult.GetValue();
    ASSERT_FALSE(token.empty());
    
    // Test token validation
    auto validateResult = authManager->ValidateToken(token);
    ASSERT_TRUE(validateResult.IsSuccess());
    
    // Test refresh token
    auto refreshResult = authManager->RefreshToken(token);
    ASSERT_TRUE(refreshResult.IsSuccess());
    ASSERT_NE(refreshResult.GetValue(), token);
    
    // Test logout
    auto logoutResult = authManager->Logout(token);
    ASSERT_TRUE(logoutResult.IsSuccess());
}

TEST_F(ApiIntegrationTests, FileOperations) {
    // Login first
    auto token = authManager->Login("testUser", "testPassword").GetValue();
    apiClient->SetAuthToken(token);
    
    // Test file upload
    std::string testFilePath = "test_upload.txt";
    std::ofstream testFile(testFilePath);
    testFile << "测试上传文件内容" << std::endl;
    testFile.close();
    
    auto uploadResult = apiClient->UploadFile(testFilePath);
    ASSERT_TRUE(uploadResult.IsSuccess());
    std::string fileId = uploadResult.GetValue();
    
    // Test file metadata
    auto metadataResult = apiClient->GetFileMetadata(fileId);
    ASSERT_TRUE(metadataResult.IsSuccess());
    auto metadata = metadataResult.GetValue();
    ASSERT_EQ(metadata.fileName, "test_upload.txt");
    
    // Test file download
    std::string downloadPath = "test_download.txt";
    auto downloadResult = apiClient->DownloadFile(fileId, downloadPath);
    ASSERT_TRUE(downloadResult.IsSuccess());
    
    // Verify file contents
    std::ifstream downloadedFile(downloadPath);
    std::string content;
    std::getline(downloadedFile, content);
    ASSERT_EQ(content, "测试上传文件内容");
    
    // Cleanup
    std::filesystem::remove(testFilePath);
    std::filesystem::remove(downloadPath);
}

TEST_F(ApiIntegrationTests, DocumentCollaboration) {
    // Login first
    auto token = authManager->Login("testUser", "testPassword").GetValue();
    apiClient->SetAuthToken(token);
    
    // Create document
    Document doc;
    doc.title = "测试文档";
    doc.content = "协同办公测试内容";
    auto createResult = apiClient->CreateDocument(doc);
    ASSERT_TRUE(createResult.IsSuccess());
    std::string docId = createResult.GetValue();
    
    // Update document
    doc.content = "更新的协同办公测试内容";
    auto updateResult = apiClient->UpdateDocument(docId, doc);
    ASSERT_TRUE(updateResult.IsSuccess());
    
    // Get document version history
    auto historyResult = apiClient->GetDocumentHistory(docId);
    ASSERT_TRUE(historyResult.IsSuccess());
    auto versions = historyResult.GetValue();
    ASSERT_GE(versions.size(), 2);
    
    // Share document
    auto shareResult = apiClient->ShareDocument(docId, "testReceiver", 
        DocumentPermission::ReadWrite);
    ASSERT_TRUE(shareResult.IsSuccess());
    
    // Get shared documents
    auto sharedResult = apiClient->GetSharedDocuments();
    ASSERT_TRUE(sharedResult.IsSuccess());
    auto sharedDocs = sharedResult.GetValue();
    ASSERT_FALSE(sharedDocs.empty());
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
