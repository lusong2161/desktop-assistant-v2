#include <gtest/gtest.h>
#include "DocumentService.h"
#include <thread>
#include <chrono>

using namespace SmartAssistant::Core;

class DocumentServiceTests : public ::testing::Test {
protected:
    void SetUp() override {
        documentService = std::make_unique<DocumentService>();
    }

    void TearDown() override {
        documentService.reset();
        std::filesystem::remove("documents.db");
    }

    std::unique_ptr<DocumentService> documentService;
};

TEST_F(DocumentServiceTests, CreateAndOpenDocument) {
    // Create test document
    std::string title = "Test Document";
    std::vector<uint8_t> content = {'H', 'e', 'l', 'l', 'o'};
    auto createResult = documentService->CreateDocument(title, DocumentType::Word, content);
    ASSERT_TRUE(createResult.IsSuccess());
    std::string documentId = createResult.GetValue();

    // Open document
    auto openResult = documentService->OpenDocument(documentId);
    ASSERT_TRUE(openResult.IsSuccess());

    // Get content
    auto contentResult = documentService->GetContent(documentId);
    ASSERT_TRUE(contentResult.IsSuccess());
    ASSERT_EQ(contentResult.GetValue(), content);
}

TEST_F(DocumentServiceTests, VersionControl) {
    // Create document
    std::string documentId = documentService->CreateDocument(
        "Version Test", DocumentType::Word, {'V', '1'}).GetValue();
    
    // Update content
    std::vector<uint8_t> newContent = {'V', '2'};
    auto updateResult = documentService->UpdateContent(documentId, newContent);
    ASSERT_TRUE(updateResult.IsSuccess());

    // Get version history
    auto historyResult = documentService->GetVersionHistory(documentId);
    ASSERT_TRUE(historyResult.IsSuccess());
    auto versions = historyResult.GetValue();
    ASSERT_EQ(versions.size(), 2);

    // Revert to first version
    auto revertResult = documentService->RevertToVersion(documentId, versions[1].versionId);
    ASSERT_TRUE(revertResult.IsSuccess());

    // Verify content
    auto contentResult = documentService->GetContent(documentId);
    ASSERT_TRUE(contentResult.IsSuccess());
    ASSERT_EQ(contentResult.GetValue(), std::vector<uint8_t>({'V', '1'}));
}

TEST_F(DocumentServiceTests, PermissionManagement) {
    // Create document
    std::string documentId = documentService->CreateDocument(
        "Permission Test", DocumentType::Word, {'T', 'e', 's', 't'}).GetValue();
    
    // Set permission for user
    std::string userId = "testUser";
    auto permResult = documentService->SetPermission(documentId, userId, DocumentPermission::Read);
    ASSERT_TRUE(permResult.IsSuccess());

    // Get metadata
    auto metaResult = documentService->GetMetadata(documentId);
    ASSERT_TRUE(metaResult.IsSuccess());
    auto metadata = metaResult.GetValue();
    ASSERT_TRUE(metadata.userPermissions.contains(userId));
    ASSERT_EQ(metadata.userPermissions[userId], DocumentPermission::Read);
}

TEST_F(DocumentServiceTests, CollaborativeEditing) {
    class TestCallback : public IDocumentCallback {
    public:
        void OnDocumentChanged(const std::string& documentId, 
                             const std::vector<uint8_t>& diff) override {
            lastDiff = diff;
            changeCount++;
        }

        void OnPermissionChanged(const std::string& documentId, 
                               const std::string& userId,
                               DocumentPermission permission) override {}

        void OnVersionCreated(const std::string& documentId, 
                            const DocumentVersion& version) override {}

        std::vector<uint8_t> lastDiff;
        int changeCount = 0;
    };

    // Create document
    std::string documentId = documentService->CreateDocument(
        "Collab Test", DocumentType::Word, {'V', '1'}).GetValue();

    // Register callback
    auto callback = std::make_shared<TestCallback>();
    documentService->RegisterCallback(callback);

    // Update content
    std::vector<uint8_t> newContent = {'V', '2'};
    documentService->UpdateContent(documentId, newContent);

    // Verify callback was triggered
    ASSERT_EQ(callback->changeCount, 1);
    ASSERT_EQ(callback->lastDiff, newContent);

    // Cleanup
    documentService->UnregisterCallback(callback);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
