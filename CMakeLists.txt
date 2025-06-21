cmake_minimum_required(VERSION 3.16)
project(aws_kms_crypto CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Release)
set(ZLIB_USE_STATIC_LIBS ON)

# Find AWS SDK packages - static version
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "/tmp/aws-sdk/lib/cmake" "/tmp/aws-sdk/lib/s2n/cmake" "/tmp/aws-sdk/lib/crypto/cmake")
find_package(ZLIB REQUIRED)
find_package(CRYPTO REQUIRED)
find_package(AWSSDK REQUIRED COMPONENTS kms)
find_package(aws-cpp-sdk-core REQUIRED)

# Add executable
add_executable(aws_kms_crypto src/main.cpp)

# Link AWS SDK libraries statically
target_link_libraries(aws_kms_crypto
    ${AWSSDK_LINK_LIBRARIES}
    ${ZLIB_LIBRARIES}
    -static
    -ldl
    -lpthread
    -lm
    -lrt
)

# Include directories
target_include_directories(aws_kms_crypto PRIVATE ${AWSSDK_INCLUDE_DIRS})
