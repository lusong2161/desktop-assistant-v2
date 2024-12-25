#pragma once
#include "Common.h"
#include <string>
#include <vector>

namespace SmartAssistant {
namespace Core {

class CORE_API CryptoService {
public:
    CryptoService();
    ~CryptoService();

    // Key management
    bool GenerateKeyPair();
    std::string GetPublicKey() const;
    bool ImportPublicKey(const std::string& publicKey);
    
    // Encryption/Decryption
    std::vector<uint8_t> EncryptMessage(const std::string& message, const std::string& recipientPublicKey);
    std::string DecryptMessage(const std::vector<uint8_t>& encryptedData);
    
    // File encryption
    bool EncryptFile(const std::string& inputPath, const std::string& outputPath, const std::string& recipientPublicKey);
    bool DecryptFile(const std::string& inputPath, const std::string& outputPath);
    
    // Message authentication
    std::vector<uint8_t> SignMessage(const std::string& message);
    bool VerifySignature(const std::string& message, const std::vector<uint8_t>& signature, const std::string& publicKey);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace Core
} // namespace SmartAssistant
