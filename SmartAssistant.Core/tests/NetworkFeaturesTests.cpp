#include <gtest/gtest.h>
#include "../include/NetworkService.h"
#include "../include/FileTransferManager.h"

// 网络连接测试
TEST(NetworkFeaturesTests, NetworkConnectivity) {
    NetworkService networkService;
    EXPECT_TRUE(networkService.initialize());
    EXPECT_TRUE(networkService.testConnection());
}

// P2P文件传输测试
TEST(NetworkFeaturesTests, P2PFileTransfer) {
    FileTransferManager transferManager;
    EXPECT_TRUE(transferManager.initialize());
    
    // Test file transfer initialization
    auto transfer = transferManager.createTransfer("test.txt", "testuser");
    EXPECT_TRUE(transfer != nullptr);
    
    // Test pause/resume functionality
    EXPECT_TRUE(transferManager.pauseTransfer(transfer));
    EXPECT_TRUE(transferManager.resumeTransfer(transfer));
    
    // Test cancel functionality
    EXPECT_TRUE(transferManager.cancelTransfer(transfer));
}

// 用户发现测试
TEST(NetworkFeaturesTests, UserDiscovery) {
    NetworkService networkService;
    EXPECT_TRUE(networkService.initialize());
    
    // Test user discovery
    auto users = networkService.discoverUsers();
    EXPECT_FALSE(users.empty());
}

// 文件共享网络功能测试
TEST(NetworkFeaturesTests, FileSharing) {
    NetworkService networkService;
    FileTransferManager transferManager;
    
    EXPECT_TRUE(networkService.initialize());
    EXPECT_TRUE(transferManager.initialize());
    
    // Test file sharing setup
    EXPECT_TRUE(networkService.enableFileSharing());
    
    // Test file transfer progress
    auto transfer = transferManager.createTransfer("test.txt", "testuser");
    EXPECT_TRUE(transfer != nullptr);
    EXPECT_GE(transferManager.getTransferProgress(transfer), 0.0f);
    EXPECT_LE(transferManager.getTransferProgress(transfer), 100.0f);
}

// 协同办公网络功能测试
TEST(NetworkFeaturesTests, CollaborativeFeatures) {
    NetworkService networkService;
    EXPECT_TRUE(networkService.initialize());
    
    // Test document collaboration session
    EXPECT_TRUE(networkService.createCollaborationSession("测试文档.docx"));
    
    // Test user joining collaboration
    EXPECT_TRUE(networkService.joinCollaborationSession("测试文档.docx", "testuser"));
    
    // Test real-time updates
    EXPECT_TRUE(networkService.sendDocumentUpdate("测试文档.docx", "update content"));
}
