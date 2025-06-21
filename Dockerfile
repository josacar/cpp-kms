FROM alpine:3.22

WORKDIR /tmp

RUN apk add --no-cache git cmake make clang-dev zlib-dev zlib-static

# Build AWS SDK with AWS-LC and AWS CRT instead of OpenSSL and curl
RUN git clone --recurse-submodules --shallow-submodules --depth 10 https://github.com/aws/aws-sdk-cpp && \
  mkdir build-aws-sdk && \
  cd build-aws-sdk && \
  cmake ../aws-sdk-cpp -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_ONLY=kms \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_UNITY_BUILD=ON \
    -DBUILD_SHARED_LIBS=OFF \
    -DCUSTOM_MEMORY_MANAGEMENT=OFF \
    -DENABLE_TESTING=OFF \
    -DCMAKE_INSTALL_PREFIX:PATH=/tmp/aws-sdk \
    -DUSE_OPENSSL=OFF \
    -DUSE_CRT_HTTP_CLIENT=ON \
    -DENABLE_AWS_COMMON_RUNTIME=ON && \
  cmake --build . -j 4 && \
  cmake --install . && \
  cd .. && \
  rm -rf build-aws-sdk aws-sdk-cpp

WORKDIR /tmp/code

ADD CMakeLists.txt .
ADD src src

RUN mkdir build && \
  cd build && \
  cmake .. && \
  cmake --build .
