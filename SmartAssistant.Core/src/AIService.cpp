#include "../include/AIService.h"
#include <windows.h>
#include <winhttp.h>
#include <wincred.h>
#include <sapi.h>
#include <sphelper.h>
#include <nlohmann/json.hpp>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "wincred.lib")
#pragma comment(lib, "sapi.lib")

namespace SmartAssistant {
namespace Core {

namespace {
    const wchar_t* CREDENTIAL_TARGET = L"SmartAssistant_OpenAI_Key";
    const wchar_t* OPENAI_HOST = L"api.openai.com";
    const wchar_t* OPENAI_PATH = L"/v1/chat/completions";
}

bool AIService::Initialize(const std::wstring& apiKey) {
    CREDENTIAL cred = { 0 };
    cred.Type = CRED_TYPE_GENERIC;
    cred.TargetName = const_cast<LPWSTR>(CREDENTIAL_TARGET);
    cred.CredentialBlobSize = static_cast<DWORD>(apiKey.size() * sizeof(wchar_t));
    cred.CredentialBlob = reinterpret_cast<LPBYTE>(const_cast<wchar_t*>(apiKey.c_str()));
    cred.Persist = CRED_PERSIST_LOCAL_MACHINE;

    // Initialize COM for speech
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return false;

    return CredWriteW(&cred, 0);
}

void AIService::Shutdown() {
    CredDeleteW(CREDENTIAL_TARGET, CRED_TYPE_GENERIC, 0);
    CoUninitialize();
}

bool AIService::ProcessTextCommand(const std::wstring& command, AIResponse& response) {
    CREDENTIAL* cred;
    if (!CredReadW(CREDENTIAL_TARGET, CRED_TYPE_GENERIC, 0, &cred)) {
        return false;
    }

    std::wstring apiKey(reinterpret_cast<wchar_t*>(cred->CredentialBlob),
                       cred->CredentialBlobSize / sizeof(wchar_t));
    CredFree(cred);

    // Initialize WinHTTP
    HINTERNET hSession = WinHttpOpen(L"SmartAssistant/1.0",
                                   WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                   WINHTTP_NO_PROXY_NAME,
                                   WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return false;

    // Connect to OpenAI API
    HINTERNET hConnect = WinHttpConnect(hSession, OPENAI_HOST,
                                      INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Create HTTP request
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", OPENAI_PATH,
                                          nullptr, WINHTTP_NO_REFERER,
                                          WINHTTP_DEFAULT_ACCEPT_TYPES,
                                          WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Prepare request headers and body
    std::wstring headers = L"Content-Type: application/json\r\n";
    headers += L"Authorization: Bearer " + apiKey + L"\r\n";

    nlohmann::json requestJson = {
        {"model", "gpt-3.5-turbo"},
        {"messages", {{
            {"role", "user"},
            {"content", std::string(command.begin(), command.end())}
        }}},
        {"temperature", 0.7},
        {"max_tokens", 150}
    };

    std::string requestBody = requestJson.dump();

    // Send request
    if (!WinHttpSendRequest(hRequest, headers.c_str(), -1L,
                           (LPVOID)requestBody.c_str(),
                           static_cast<DWORD>(requestBody.size()),
                           static_cast<DWORD>(requestBody.size()), 0)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    if (!WinHttpReceiveResponse(hRequest, nullptr)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Read response
    std::string responseData;
    DWORD bytesAvailable;
    while (WinHttpQueryDataAvailable(hRequest, &bytesAvailable) && bytesAvailable > 0) {
        std::vector<char> buffer(bytesAvailable + 1);
        DWORD bytesRead;
        if (WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead)) {
            buffer[bytesRead] = '\0';
            responseData += buffer.data();
        }
    }

    // Parse JSON response
    try {
        auto responseJson = nlohmann::json::parse(responseData);
        std::string content = responseJson["choices"][0]["message"]["content"];
        response.text = std::wstring(content.begin(), content.end());
        response.confidence = 1.0;
    } catch (const std::exception&) {
        response.text = L"Error processing response";
        response.confidence = 0.0;
    }

    // Clean up
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return true;
}

bool AIService::ProcessVoiceCommand(const std::wstring& audioPath, AIResponse& response) {
    // Initialize Speech Recognition
    ISpRecognizer* recognizer = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_SpInprocRecognizer, nullptr,
                                CLSCTX_ALL, IID_ISpRecognizer,
                                reinterpret_cast<void**>(&recognizer));
    if (FAILED(hr)) return false;

    // Create audio stream
    ISpStream* stream = nullptr;
    hr = SPBindToFile(audioPath.c_str(), SPFM_OPEN_READONLY, &stream);
    if (FAILED(hr)) {
        recognizer->Release();
        return false;
    }

    // Set input
    hr = recognizer->SetInput(stream, TRUE);
    if (FAILED(hr)) {
        stream->Release();
        recognizer->Release();
        return false;
    }

    // Create context
    ISpRecoContext* context = nullptr;
    hr = recognizer->CreateRecoContext(&context);
    if (FAILED(hr)) {
        stream->Release();
        recognizer->Release();
        return false;
    }

    // Get recognition result
    ISpRecoResult* result = nullptr;
    hr = context->RecognizeStream(SPREF_AutoPause, nullptr, &result);
    if (SUCCEEDED(hr) && result) {
        WCHAR* text = nullptr;
        hr = result->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &text, nullptr);
        if (SUCCEEDED(hr)) {
            // Process recognized text
            ProcessTextCommand(text, response);
            CoTaskMemFree(text);
        }
        result->Release();
    }

    // Clean up
    context->Release();
    stream->Release();
    recognizer->Release();

    return SUCCEEDED(hr);
}

bool AIService::GetCommandSuggestions(const std::wstring& partialCommand,
                                    std::vector<std::wstring>& suggestions) {
    AIResponse response;
    std::wstring prompt = L"Suggest commands similar to: " + partialCommand;
    if (ProcessTextCommand(prompt, response)) {
        // Parse suggestions from response
        std::wstring text = response.text;
        size_t pos = 0;
        while ((pos = text.find(L'\n')) != std::wstring::npos) {
            if (pos > 0) {
                suggestions.push_back(text.substr(0, pos));
            }
            text = text.substr(pos + 1);
        }
        if (!text.empty()) {
            suggestions.push_back(text);
        }
        return true;
    }
    return false;
}

bool AIService::SynthesizeVoice(const std::wstring& text, const std::wstring& outputPath) {
    // Initialize Speech Synthesis
    ISpVoice* voice = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_SpVoice, nullptr,
                                CLSCTX_ALL, IID_ISpVoice,
                                reinterpret_cast<void**>(&voice));
    if (FAILED(hr)) return false;

    // Create output stream
    ISpStream* stream = nullptr;
    hr = SPBindToFile(outputPath.c_str(), SPFM_CREATE_ALWAYS,
                     &stream, nullptr, FALSE);
    if (FAILED(hr)) {
        voice->Release();
        return false;
    }

    // Set output
    hr = voice->SetOutput(stream, TRUE);
    if (SUCCEEDED(hr)) {
        // Speak text to file
        hr = voice->Speak(text.c_str(), SPF_IS_XML, nullptr);
    }

    // Clean up
    stream->Release();
    voice->Release();

    return SUCCEEDED(hr);
}

} // namespace Core
} // namespace SmartAssistant
