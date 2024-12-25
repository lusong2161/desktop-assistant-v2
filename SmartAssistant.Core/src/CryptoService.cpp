#include "CryptoService.h"
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <fstream>
#include <vector>
#include <memory>

namespace SmartAssistant {
namespace Core {

class CryptoService::Impl {
public:
    Impl() {
        // Initialize OpenSSL
        OpenSSL_add_all_algorithms();
    }

    ~Impl() {
        if (privateKey_) EVP_PKEY_free(privateKey_);
        EVP_cleanup();
    }

    bool GenerateKeyPair() {
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
        if (!ctx) return false;

        if (EVP_PKEY_keygen_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return false;
        }

        if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return false;
        }

        if (privateKey_) EVP_PKEY_free(privateKey_);
        if (EVP_PKEY_keygen(ctx, &privateKey_) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return false;
        }

        EVP_PKEY_CTX_free(ctx);
        return true;
    }

    std::string GetPublicKey() const {
        if (!privateKey_) return "";

        BIO* bio = BIO_new(BIO_s_mem());
        PEM_write_bio_PUBKEY(bio, privateKey_);
        
        char* data = nullptr;
        long len = BIO_get_mem_data(bio, &data);
        std::string result(data, len);
        
        BIO_free(bio);
        return result;
    }

    std::vector<uint8_t> EncryptMessage(const std::string& message, const std::string& recipientPublicKey) {
        BIO* bio = BIO_new_mem_buf(recipientPublicKey.c_str(), -1);
        EVP_PKEY* pubkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);

        if (!pubkey) return std::vector<uint8_t>();

        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pubkey, nullptr);
        if (!ctx) {
            EVP_PKEY_free(pubkey);
            return std::vector<uint8_t>();
        }

        if (EVP_PKEY_encrypt_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pubkey);
            return std::vector<uint8_t>();
        }

        size_t outlen;
        if (EVP_PKEY_encrypt(ctx, nullptr, &outlen,
                            reinterpret_cast<const unsigned char*>(message.c_str()),
                            message.length()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pubkey);
            return std::vector<uint8_t>();
        }

        std::vector<uint8_t> encrypted(outlen);
        if (EVP_PKEY_encrypt(ctx, encrypted.data(), &outlen,
                            reinterpret_cast<const unsigned char*>(message.c_str()),
                            message.length()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pubkey);
            return std::vector<uint8_t>();
        }

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pubkey);
        return encrypted;
    }

    std::string DecryptMessage(const std::vector<uint8_t>& encryptedData) {
        if (!privateKey_) return "";

        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(privateKey_, nullptr);
        if (!ctx) return "";

        if (EVP_PKEY_decrypt_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return "";
        }

        size_t outlen;
        if (EVP_PKEY_decrypt(ctx, nullptr, &outlen,
                            encryptedData.data(),
                            encryptedData.size()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return "";
        }

        std::vector<uint8_t> decrypted(outlen);
        if (EVP_PKEY_decrypt(ctx, decrypted.data(), &outlen,
                            encryptedData.data(),
                            encryptedData.size()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return "";
        }

        EVP_PKEY_CTX_free(ctx);
        return std::string(reinterpret_cast<char*>(decrypted.data()), outlen);
    }

    bool EncryptFile(const std::string& inputPath, const std::string& outputPath,
                    const std::string& recipientPublicKey) {
        // Implementation using AES for file encryption and RSA for key exchange
        // This is a simplified version - production code would need more robust error handling
        std::ifstream inFile(inputPath, std::ios::binary);
        std::ofstream outFile(outputPath, std::ios::binary);
        if (!inFile || !outFile) return false;

        // Generate random AES key
        unsigned char aesKey[32];
        if (RAND_bytes(aesKey, sizeof(aesKey)) != 1) return false;

        // Encrypt AES key with recipient's public key
        auto encryptedKey = EncryptMessage(
            std::string(reinterpret_cast<char*>(aesKey), sizeof(aesKey)),
            recipientPublicKey
        );
        
        // Write encrypted key size and key
        uint32_t keySize = static_cast<uint32_t>(encryptedKey.size());
        outFile.write(reinterpret_cast<char*>(&keySize), sizeof(keySize));
        outFile.write(reinterpret_cast<char*>(encryptedKey.data()), encryptedKey.size());

        // Encrypt file content
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, aesKey, nullptr);

        std::vector<unsigned char> buffer(4096);
        std::vector<unsigned char> outBuffer(4096 + EVP_MAX_BLOCK_LENGTH);
        int outLen;

        while (inFile) {
            inFile.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
            std::streamsize bytesRead = inFile.gcount();
            if (bytesRead <= 0) break;

            EVP_EncryptUpdate(ctx, outBuffer.data(), &outLen,
                            buffer.data(), static_cast<int>(bytesRead));
            outFile.write(reinterpret_cast<char*>(outBuffer.data()), outLen);
        }

        EVP_EncryptFinal_ex(ctx, outBuffer.data(), &outLen);
        outFile.write(reinterpret_cast<char*>(outBuffer.data()), outLen);

        EVP_CIPHER_CTX_free(ctx);
        return true;
    }

private:
    EVP_PKEY* privateKey_ = nullptr;
};

// CryptoService implementation
CryptoService::CryptoService() : pImpl(std::make_unique<Impl>()) {}
CryptoService::~CryptoService() = default;

bool CryptoService::GenerateKeyPair() {
    return pImpl->GenerateKeyPair();
}

std::string CryptoService::GetPublicKey() const {
    return pImpl->GetPublicKey();
}

std::vector<uint8_t> CryptoService::EncryptMessage(const std::string& message,
                                                  const std::string& recipientPublicKey) {
    return pImpl->EncryptMessage(message, recipientPublicKey);
}

std::string CryptoService::DecryptMessage(const std::vector<uint8_t>& encryptedData) {
    return pImpl->DecryptMessage(encryptedData);
}

bool CryptoService::EncryptFile(const std::string& inputPath,
                               const std::string& outputPath,
                               const std::string& recipientPublicKey) {
    return pImpl->EncryptFile(inputPath, outputPath, recipientPublicKey);
}

} // namespace Core
} // namespace SmartAssistant
