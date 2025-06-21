#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>
#include <cstring>
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
    KMSCrypto(const std::string& keyId = "") : m_keyId(keyId) {}

    // Encrypt a plaintext message using AWS KMS
    std::vector<uint8_t> encrypt(const std::string& plaintext) {
        if (m_keyId.empty()) {
            throw std::runtime_error("Key ID is required for encryption");
        }
        
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
        
        // Only set KeyId if provided (optional for decryption)
        if (!m_keyId.empty()) {
            request.SetKeyId(m_keyId);
        }

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

// Function to print usage information
void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n"
              << "Options:\n"
              << "  -e, --encrypt <message>    Encrypt the provided message\n"
              << "  -d, --decrypt <ciphertext> Decrypt the provided ciphertext (in hex format)\n"
              << "  -k, --key <key_id>         AWS KMS Key ID or ARN (required for encryption)\n"
              << "  -h, --help                 Display this help message\n"
              << std::endl;
}

// Function to convert hex string to bytes
std::vector<uint8_t> hexToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
        bytes.push_back(byte);
    }
    
    return bytes;
}

int main(int argc, char* argv[]) {
    // Initialize AWS SDK
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    
    // Default values
    std::string keyId;
    std::string message;
    std::string ciphertext;
    bool encrypt = false;
    bool decrypt = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            Aws::ShutdownAPI(options);
            return 0;
        }
        else if (strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--key") == 0) {
            if (i + 1 < argc) {
                keyId = argv[++i];
            } else {
                std::cerr << "Error: Missing key ID after -k/--key" << std::endl;
                printUsage(argv[0]);
                Aws::ShutdownAPI(options);
                return 1;
            }
        }
        else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--encrypt") == 0) {
            if (i + 1 < argc) {
                message = argv[++i];
                encrypt = true;
            } else {
                std::cerr << "Error: Missing message after -e/--encrypt" << std::endl;
                printUsage(argv[0]);
                Aws::ShutdownAPI(options);
                return 1;
            }
        }
        else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--decrypt") == 0) {
            if (i + 1 < argc) {
                ciphertext = argv[++i];
                decrypt = true;
            } else {
                std::cerr << "Error: Missing ciphertext after -d/--decrypt" << std::endl;
                printUsage(argv[0]);
                Aws::ShutdownAPI(options);
                return 1;
            }
        }
        else {
            std::cerr << "Error: Unknown option: " << argv[i] << std::endl;
            printUsage(argv[0]);
            Aws::ShutdownAPI(options);
            return 1;
        }
    }
    
    // Validate arguments
    if (encrypt && keyId.empty()) {
        std::cerr << "Error: KMS Key ID is required for encryption" << std::endl;
        printUsage(argv[0]);
        Aws::ShutdownAPI(options);
        return 1;
    }
    
    if (!encrypt && !decrypt) {
        std::cerr << "Error: Either encrypt or decrypt operation must be specified" << std::endl;
        printUsage(argv[0]);
        Aws::ShutdownAPI(options);
        return 1;
    }
    
    if (encrypt && decrypt) {
        std::cerr << "Error: Cannot perform both encrypt and decrypt operations at once" << std::endl;
        printUsage(argv[0]);
        Aws::ShutdownAPI(options);
        return 1;
    }
    
    try {
        // Create KMS crypto instance
        KMSCrypto kmsCrypto(keyId);
        
        if (encrypt) {
            // Encrypt the message
            std::cout << "Encrypting message..." << std::endl;
            auto encrypted = kmsCrypto.encrypt(message);
            
            // Display encrypted data as hex
            std::cout << "Encrypted (hex): ";
            for (const auto& byte : encrypted) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') 
                          << static_cast<int>(byte);
            }
            std::cout << std::dec << std::endl;
        }
        else if (decrypt) {
            // Convert hex string to bytes
            std::vector<uint8_t> encryptedBytes;
            try {
                encryptedBytes = hexToBytes(ciphertext);
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid hex format in ciphertext" << std::endl;
                Aws::ShutdownAPI(options);
                return 1;
            }
            
            // Decrypt the message
            std::cout << "Decrypting ciphertext..." << std::endl;
            auto decrypted = kmsCrypto.decrypt(encryptedBytes);
            
            // Display decrypted message
            std::cout << "Decrypted message: " << decrypted << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        Aws::ShutdownAPI(options);
        return 1;
    }
    
    // Clean up AWS SDK resources
    Aws::ShutdownAPI(options);
    return 0;
}
