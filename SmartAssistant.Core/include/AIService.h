#pragma once
#include "Common.h"

#include <string>
#include <memory>
#include <vector>

namespace SmartAssistant {
namespace Core {

struct SMARTASSISTANT_API AIResponse {
    std::wstring text;
    double confidence;
    std::vector<std::wstring> suggestions;
};

class SMARTASSISTANT_API AIService {
public:
    static bool Initialize(const std::wstring& apiKey);
    static void Shutdown();
    
    // Text Processing
    static bool ProcessTextCommand(const std::wstring& command, AIResponse& response);
    static bool ProcessVoiceCommand(const std::wstring& audioPath, AIResponse& response);
    
    // Command Suggestions
    static bool GetCommandSuggestions(const std::wstring& partialCommand, 
                                    std::vector<std::wstring>& suggestions);
    
    // Voice Synthesis
    static bool SynthesizeVoice(const std::wstring& text, const std::wstring& outputPath);
};

} // namespace Core
} // namespace SmartAssistant
