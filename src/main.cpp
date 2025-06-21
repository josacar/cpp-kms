#include <iostream>
#include <string>
#include <vector>
#include <aws/core/Aws.h>
#include <aws/kms/KMSClient.h>
#include <aws/kms/model/EncryptRequest.h>
#include <aws/kms/model/DecryptRequest.h>
#include <aws/core/utils/Array.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/core/utils/Outcome.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>
#include <aws/core/utils/HashingUtils.h>

// Helper function to convert string to binary data (Aws::Utils::ByteBuffer)
Aws::Utils::ByteBuffer stringToByteBuffer(const std::string& str) {
    return Aws::Utils::ByteBuffer(
        reinterpret_cast<const unsigned char*>(str.data()),
        str.size());
}

// Helper function to convert binary data to string
std::string byteBufferToString(const Aws::Utils::ByteBuffer& buffer) {
    return std::string(reinterpret_cast<const char*>(buffer.GetUnderlyingData()), 
                      buffer.GetLength());
}

// Helper function to convert binary data to hex string for display
std::string byteBufferToHexString(const Aws::Utils::ByteBuffer& buffer) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < buffer.GetLength(); ++i) {
        ss << std::setw(2) << static_cast<int>(buffer[i]);
    }
    return ss.str();
}

class KMSCrypto {
private:
    Aws::KMS::KMSClient m_kmsClient;
    std::string m_keyId;

public:
    KMSCrypto(const std::string& keyId) : m_keyId(keyId) {}

    // Encrypt a plaintext message using AWS KMS
    std::vector<uint8_t> encrypt(const std::string& plaintext) {
        Aws::KMS::Model::EncryptRequest request;
        request.SetKeyId(m_keyId);
        request.SetPlaintext(stringToByteBuffer(plaintext));

        auto outcome = m_kmsClient.Encrypt(request);
        if (!outcome.IsSuccess()) {
            throw std::runtime_error("Failed to encrypt: " + 
                outcome.GetError().GetMessage());
        }

        // Get the ciphertext blob
        auto cipherBlob = outcome.GetResult().GetCiphertextBlob();
        
        // Convert to std::vector for easier handling
        std::vector<uint8_t> result(cipherBlob.GetLength());
        std::memcpy(result.data(), cipherBlob.GetUnderlyingData(), cipherBlob.GetLength());
        
        return result;
    }

    // Decrypt a ciphertext using AWS KMS
    std::string decrypt(const std::vector<uint8_t>& ciphertext) {
        Aws::KMS::Model::DecryptRequest request;
        
        // Convert vector to ByteBuffer
        Aws::Utils::ByteBuffer cipherBlob(ciphertext.data(), ciphertext.size());
        request.SetCiphertextBlob(cipherBlob);

        auto outcome = m_kmsClient.Decrypt(request);
        if (!outcome.IsSuccess()) {
            throw std::runtime_error("Failed to decrypt: " + 
                outcome.GetError().GetMessage());
        }

        // Get the plaintext and convert to string
        auto plaintext = outcome.GetResult().GetPlaintext();
        return byteBufferToString(plaintext);
    }
};

int main() {
    // Initialize AWS SDK
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        try {
            // Replace with your actual KMS key ID or ARN
            std::string keyId = "YOUR_KMS_KEY_ID";
            
            // Create KMS crypto instance
            KMSCrypto kmsCrypto(keyId);
            
            // Message to encrypt
            std::string message = "Hello, this is a secret message!";
            std::cout << "Original message: " << message << std::endl;
            
            // Encrypt the message
            std::cout << "Encrypting..." << std::endl;
            auto encrypted = kmsCrypto.encrypt(message);
            
            // Display encrypted data as hex
            std::cout << "Encrypted (hex): ";
            for (const auto& byte : encrypted) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') 
                          << static_cast<int>(byte);
            }
            std::cout << std::dec << std::endl;
            
            // Decrypt the message
            std::cout << "Decrypting..." << std::endl;
            auto decrypted = kmsCrypto.decrypt(encrypted);
            
            // Display decrypted message
            std::cout << "Decrypted message: " << decrypted << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            Aws::ShutdownAPI(options);
            return 1;
        }
    }
    
    // Clean up AWS SDK resources
    Aws::ShutdownAPI(options);
    return 0;
}
