#include <gtest/gtest.h>
#include "NetworkService.h"
#include "FileTransferManager.h"
#include "CryptoService.h"
#include <thread>
#include <chrono>

using namespace SmartAssistant::Core;

class NetworkFeaturesTests : public ::testing::Test {
protected:
    void SetUp() override {
        networkService = std::make_unique<NetworkService>();
        fileTransferManager = std::make_unique<FileTransferManager>();
        cryptoService = std::make_unique<CryptoService>();
        
        // Initialize backend connection
        auto initResult = networkService->Initialize("ws://localhost:8080");
        ASSERT_TRUE(initResult.IsSuccess()) << "Failed to initialize network service";
        
        // Set up test user credentials
        testUserId = "testUser123";
        testAuthToken = "testToken123";
    }

    void TearDown() override {
        if (networkService) {
            networkService->Disconnect();
        }
    }

    std::unique_ptr<NetworkService> networkService;
    std::unique_ptr<FileTransferManager> fileTransferManager;
    std::unique_ptr<CryptoService> cryptoService;
    std::string testUserId;
    std::string testAuthToken;
    
    // Helper method to wait for async operations
    void WaitForAsync(int milliseconds = 1000) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }
};

TEST_F(NetworkFeaturesTests, AuthenticationFlow) {
    // Test connection with invalid credentials
    auto invalidResult = networkService->Connect("invalid", "invalid");
    ASSERT_FALSE(invalidResult.IsSuccess());
    ASSERT_EQ(networkService->GetConnectionStatus(), ConnectionStatus::Error);
    
    // Test connection with valid credentials
    auto validResult = networkService->Connect(testUserId, testAuthToken);
    ASSERT_TRUE(validResult.IsSuccess());
    WaitForAsync();
    ASSERT_EQ(networkService->GetConnectionStatus(), ConnectionStatus::Connected);
    
    // Test token refresh
    auto refreshResult = networkService->RefreshAuthToken();
    ASSERT_TRUE(refreshResult.IsSuccess());
    
    // Test disconnect
    networkService->Disconnect();
    WaitForAsync();
    ASSERT_EQ(networkService->GetConnectionStatus(), ConnectionStatus::Disconnected);
}

TEST_F(NetworkFeaturesTests, InstantMessaging) {
    // Connect to server
    auto connectResult = networkService->Connect(testUserId, testAuthToken);
    ASSERT_TRUE(connectResult.IsSuccess());
    WaitForAsync();
    
    // Create message receiver callback
    bool messageReceived = false;
    std::string receivedContent;
    auto callback = std::make_shared<TestMessageCallback>([&](const Message& msg) {
        messageReceived = true;
        receivedContent = msg.content;
    });
    networkService->RegisterCallback(callback);
    
    // Test message sending
    Message msg;
    msg.content = "测试消息";
    msg.receiverId = "testReceiver";
    auto sendResult = networkService->SendMessage(msg);
    ASSERT_TRUE(sendResult.IsSuccess());
    
    // Wait for message delivery
    WaitForAsync();
    ASSERT_TRUE(messageReceived);
    ASSERT_EQ(receivedContent, "测试消息");
    
    // Test encrypted message
    auto publicKey = cryptoService->GetPublicKey("testReceiver");
    auto encryptResult = networkService->SendEncryptedMessage(msg, publicKey);
    ASSERT_TRUE(encryptResult.IsSuccess());
}

TEST_F(NetworkFeaturesTests, FriendManagement) {
    // Connect to server
    auto connectResult = networkService->Connect(testUserId, testAuthToken);
    ASSERT_TRUE(connectResult.IsSuccess());
    WaitForAsync();

    // Setup friend request tracking
    bool requestReceived = false;
    std::string receivedUserId;
    FriendRequest receivedRequest;

    // Register friend callback
    auto callback = std::make_shared<TestFriendCallback>(
        [&](const std::string& userId, const FriendRequest& request) {
            requestReceived = true;
            receivedUserId = userId;
            receivedRequest = request;
        });
    networkService->RegisterCallback(callback);

    // Test sending friend request
    auto requestResult = networkService->AddFriend("testFriend");
    ASSERT_TRUE(requestResult.IsSuccess());
    WaitForAsync();

    // Test friend request acceptance
    auto acceptResult = networkService->AcceptFriendRequest(callback->lastRequestUserId);
    ASSERT_TRUE(acceptResult.IsSuccess());
    WaitForAsync();

    // Verify friend request handling
    ASSERT_TRUE(requestReceived);
    ASSERT_GT(callback->requestCount, 0);
    ASSERT_EQ(callback->lastRequest.status, FriendRequestStatus::Accepted);

    // Test friend status
    auto friendStatus = networkService->GetFriendStatus("testFriend");
    ASSERT_TRUE(friendStatus.IsSuccess());
    ASSERT_EQ(friendStatus.GetValue(), UserStatus::Online);
}

TEST_F(NetworkFeaturesTests, FileTransfer) {
    // Connect to server
    auto connectResult = networkService->Connect(testUserId, testAuthToken);
    ASSERT_TRUE(connectResult.IsSuccess());
    WaitForAsync();
    
    // Create test file
    std::string testFilePath = "test_file.txt";
    std::ofstream testFile(testFilePath);
    testFile << "测试文件内容" << std::endl;
    testFile.close();
    
    // Test file transfer initiation
    auto initResult = fileTransferManager->InitiateTransfer(testFilePath, "testReceiver");
    ASSERT_TRUE(initResult.IsSuccess());
    std::string transferId = initResult.GetValue();
    
    // Register transfer callback
    bool transferStarted = false;
    bool transferCompleted = false;
    auto callback = std::make_shared<TestTransferCallback>(
        [&](const std::string& id, TransferStatus status) {
            if (status == TransferStatus::Transferring) transferStarted = true;
            if (status == TransferStatus::Completed) transferCompleted = true;
        }
    );
    fileTransferManager->RegisterCallback(callback);
    
    // Test transfer control
    WaitForAsync();
    ASSERT_TRUE(transferStarted);
    
    auto pauseResult = fileTransferManager->PauseTransfer(transferId);
    ASSERT_TRUE(pauseResult.IsSuccess());
    ASSERT_EQ(fileTransferManager->GetTransferStatus(transferId), TransferStatus::Paused);
    
    auto resumeResult = fileTransferManager->ResumeTransfer(transferId);
    ASSERT_TRUE(resumeResult.IsSuccess());
    
    // Wait for transfer completion
    WaitForAsync(2000);
    ASSERT_TRUE(transferCompleted);
    
    // Verify file encryption
    auto fileData = fileTransferManager->GetTransferData(transferId);
    ASSERT_TRUE(fileData.IsSuccess());
    auto encryptResult = cryptoService->EncryptData(fileData.GetValue(), 
        cryptoService->GetPublicKey("testReceiver"));
    ASSERT_TRUE(encryptResult.IsSuccess());
    
    // Cleanup
    std::filesystem::remove(testFilePath);
}

TEST_F(NetworkFeaturesTests, TransferProgress) {
    class TestProgressCallback : public ITransferCallback {
    public:
        TestProgressCallback(std::function<void(const std::string&, uint64_t, uint64_t)> onProgress)
            : onProgress_(onProgress) {}

        void OnProgress(const std::string& transferId, 
                       uint64_t bytesTransferred,
                       uint64_t totalBytes) override {
            if (onProgress_) {
                onProgress_(transferId, bytesTransferred, totalBytes);
            }
            progress = static_cast<double>(bytesTransferred) / totalBytes;
            updateCount++;
        }

        void OnTransferStatusChanged(const std::string& transferId, 
                                   TransferStatus status) override {}

        double progress = 0.0;
        int updateCount = 0;

    private:
        std::function<void(const std::string&, uint64_t, uint64_t)> onProgress_;
    };

    class TestFriendCallback : public INetworkCallback {
    public:
        TestFriendCallback(std::function<void(const std::string&, const FriendRequest&)> onRequest)
            : onRequest_(onRequest) {}

        void OnFriendRequestReceived(const std::string& userId, 
                                   const FriendRequest& request) override {
            if (onRequest_) onRequest_(userId, request);
            lastRequestUserId = userId;
            lastRequest = request;
            requestCount++;
        }

        void OnMessageReceived(const Message& msg) override {}
        void OnConnectionStatusChanged(ConnectionStatus status) override {}
        void OnStatusChanged(const UserStatus& status) override {}

        std::string lastRequestUserId;
        FriendRequest lastRequest;
        int requestCount = 0;

    private:
        std::function<void(const std::string&, const FriendRequest&)> onRequest_;
    };

    // Connect to server
    auto connectResult = networkService->Connect(testUserId, testAuthToken);
    ASSERT_TRUE(connectResult.IsSuccess());
    WaitForAsync();

    // Create test file with Chinese content
    std::string testFilePath = "test_progress.txt";
    std::ofstream testFile(testFilePath);
    testFile << "测试传输进度文件内容" << std::endl;
    testFile.close();

    // Create progress tracking variables
    uint64_t lastBytes = 0;
    uint64_t totalBytes = 0;
    bool progressUpdated = false;
    bool transferCompleted = false;

    // Register progress callback
    auto callback = std::make_shared<TestProgressCallback>(
        [&](const std::string& id, uint64_t bytes, uint64_t total) {
            lastBytes = bytes;
            totalBytes = total;
            progressUpdated = true;
        });
    fileTransferManager->RegisterCallback(callback);

    // Enable encryption for secure transfer
    fileTransferManager->SetEncryptionEnabled(true);

    // Initiate transfer
    auto initResult = fileTransferManager->InitiateTransfer(testFilePath, "testReceiver");
    ASSERT_TRUE(initResult.IsSuccess());
    std::string transferId = initResult.GetValue();

    // Test pause and resume (断点续传)
    WaitForAsync(500);
    auto pauseResult = fileTransferManager->PauseTransfer(transferId);
    ASSERT_TRUE(pauseResult.IsSuccess());
    
    WaitForAsync(500);
    auto resumeResult = fileTransferManager->ResumeTransfer(transferId);
    ASSERT_TRUE(resumeResult.IsSuccess());

    // Wait for transfer progress
    WaitForAsync(2000);
    ASSERT_TRUE(progressUpdated);
    ASSERT_GT(lastBytes, 0);
    ASSERT_GT(totalBytes, 0);
    ASSERT_GT(callback->progress, 0.0);
    ASSERT_GT(callback->updateCount, 0);

    // Verify encryption status
    auto encryptionStatus = fileTransferManager->GetTransferEncryptionStatus(transferId);
    ASSERT_TRUE(encryptionStatus.IsSuccess());
    ASSERT_TRUE(encryptionStatus.GetValue());

    // Cleanup
    std::filesystem::remove(testFilePath);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
