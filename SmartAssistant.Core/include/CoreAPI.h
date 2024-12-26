#pragma once

#include <string>
#include <memory>

namespace SmartAssistant {
namespace Core {

class CoreAPI {
public:
    CoreAPI();
    ~CoreAPI();

    // 初始化核心API
    bool Initialize();
    
    // 获取版本信息
    std::string GetVersion() const;

private:
    class Implementation;
    std::unique_ptr<Implementation> impl_;
};

} // namespace Core
} // namespace SmartAssistant
