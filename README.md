# AWS KMS Encryption/Decryption C++20 Project

This project demonstrates how to use AWS KMS (Key Management Service) to encrypt and decrypt messages using C++20.

## Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 10+, or MSVC 19.27+)
- CMake 3.16 or higher
- AWS SDK for C++ installed
- AWS credentials configured

## Building the Project

1. Create a build directory:
   ```bash
   mkdir build && cd build
   ```

2. Configure with CMake:
   ```bash
   cmake ..
   ```

3. Build the project:
   ```bash
   cmake --build .
   ```

## Running the Application

Before running the application, make sure to:

1. Configure your AWS credentials (using `aws configure` or environment variables)
2. Update the KMS key ID in `src/main.cpp` with your own KMS key ID or ARN

Then run the application:
```bash
./aws_kms_crypto
```

## Project Structure

- `CMakeLists.txt` - CMake build configuration
- `src/main.cpp` - Main application code with KMS encryption/decryption logic

## AWS SDK Installation

If you haven't installed the AWS SDK for C++ yet, follow these steps to build it statically:

```bash
git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp
mkdir build-aws-sdk && cd build-aws-sdk
cmake ../aws-sdk-cpp -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_ONLY=kms \
    -DENABLE_UNITY_BUILD=ON \
    -DBUILD_SHARED_LIBS=OFF \
    -DCUSTOM_MEMORY_MANAGEMENT=OFF \
    -DCMAKE_INSTALL_PREFIX:PATH=/tmp/aws-sdk
cmake --build . -j 8
sudo cmake --install .
```

## AWS Credentials

Make sure your AWS credentials are properly configured with permissions to use KMS. You can configure credentials using:

```bash
aws configure
```

Or by setting environment variables:
```bash
export AWS_ACCESS_KEY_ID="your_access_key"
export AWS_SECRET_ACCESS_KEY="your_secret_key"
export AWS_REGION="your_region"
```
