#include "CoreAPI.h"

namespace SmartAssistant {
namespace Core {

class CoreAPI::Implementation {
public:
    Implementation() = default;
    ~Implementation() = default;

    bool Initialize() {
        return true;
    }

    std::string GetVersion() const {
        return "1.0.0";
    }
};

CoreAPI::CoreAPI() : impl_(std::make_unique<Implementation>()) {}
CoreAPI::~CoreAPI() = default;

bool CoreAPI::Initialize() {
    return impl_->Initialize();
}

std::string CoreAPI::GetVersion() const {
    return impl_->GetVersion();
}

} // namespace Core
} // namespace SmartAssistant
