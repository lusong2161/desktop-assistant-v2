#include <gtest/gtest.h>
#include "CoreAPI.h"

namespace SmartAssistant {
namespace Core {
namespace Tests {

class CoreAPITests : public ::testing::Test {
protected:
    void SetUp() override {
        api = std::make_unique<CoreAPI>();
    }

    void TearDown() override {
        api.reset();
    }

    std::unique_ptr<CoreAPI> api;
};

TEST_F(CoreAPITests, InitializeTest) {
    EXPECT_TRUE(api->Initialize());
}

TEST_F(CoreAPITests, GetVersionTest) {
    EXPECT_FALSE(api->GetVersion().empty());
    EXPECT_EQ(api->GetVersion(), "1.0.0");
}

} // namespace Tests
} // namespace Core
} // namespace SmartAssistant

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
