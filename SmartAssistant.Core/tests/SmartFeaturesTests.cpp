#include <gtest/gtest.h>
#include "AIService.h"
#include "SystemControl.h"
#include "NotificationService.h"
#include <thread>
#include <chrono>

using namespace SmartAssistant::Core;

class SmartFeaturesTests : public ::testing::Test {
protected:
    void SetUp() override {
        aiService = std::make_unique<AIService>();
        systemControl = std::make_unique<SystemControl>();
        notificationService = std::make_unique<NotificationService>();
    }

    std::unique_ptr<AIService> aiService;
    std::unique_ptr<SystemControl> systemControl;
    std::unique_ptr<NotificationService> notificationService;
};

TEST_F(SmartFeaturesTests, AIProcessing) {
    // Test natural language processing
    std::string query = "What's the weather like today?";
    auto result = aiService->ProcessQuery(query);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_FALSE(result.GetValue().empty());

    // Test command parsing
    query = "open notepad";
    result = aiService->ProcessCommand(query);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.GetValue().contains("command:launch"));
}

TEST_F(SmartFeaturesTests, VoiceProcessing) {
    // Test speech-to-text
    std::vector<uint8_t> audioData = LoadTestAudioData();
    auto textResult = aiService->SpeechToText(audioData);
    ASSERT_TRUE(textResult.IsSuccess());
    ASSERT_FALSE(textResult.GetValue().empty());

    // Test text-to-speech
    std::string text = "Hello, this is a test";
    auto audioResult = aiService->TextToSpeech(text);
    ASSERT_TRUE(audioResult.IsSuccess());
    ASSERT_FALSE(audioResult.GetValue().empty());
}

TEST_F(SmartFeaturesTests, SystemControl) {
    // Test system settings
    auto volumeResult = systemControl->SetVolume(50);
    ASSERT_TRUE(volumeResult.IsSuccess());

    auto brightnessResult = systemControl->SetBrightness(70);
    ASSERT_TRUE(brightnessResult.IsSuccess());

    // Test process management
    auto processResult = systemControl->LaunchProcess("notepad.exe");
    ASSERT_TRUE(processResult.IsSuccess());
    uint32_t pid = processResult.GetValue();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    auto terminateResult = systemControl->TerminateProcess(pid);
    ASSERT_TRUE(terminateResult.IsSuccess());
}

TEST_F(SmartFeaturesTests, NotificationSystem) {
    // Test reminder creation
    Reminder reminder;
    reminder.title = "Test Reminder";
    reminder.message = "This is a test reminder";
    reminder.timestamp = std::chrono::system_clock::now() + std::chrono::hours(1);

    auto createResult = notificationService->CreateReminder(reminder);
    ASSERT_TRUE(createResult.IsSuccess());
    std::string reminderId = createResult.GetValue();

    // Test reminder retrieval
    auto getResult = notificationService->GetReminder(reminderId);
    ASSERT_TRUE(getResult.IsSuccess());
    ASSERT_EQ(getResult.GetValue().title, reminder.title);

    // Test notification display
    auto notifyResult = notificationService->ShowNotification(
        "Test Title", "Test Message", NotificationType::Info);
    ASSERT_TRUE(notifyResult.IsSuccess());
}

std::vector<uint8_t> LoadTestAudioData() {
    // Load test audio file
    std::ifstream file("test_audio.wav", std::ios::binary);
    return std::vector<uint8_t>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
