# libcurl 테스트 프로젝트

이 프로젝트는 libcurl을 사용해서 HTTP 요청을 보내는 C++ 예제입니다.

## 요구사항

- C++17 이상
- libcurl 라이브러리
- CMake 3.10 이상 (선택사항)

## 설치

### macOS에서 libcurl 설치

```bash
# Homebrew를 사용해서 libcurl 설치
brew install curl
```

### Ubuntu/Debian에서 libcurl 설치

```bash
sudo apt-get update
sudo apt-get install libcurl4-openssl-dev
```

## 빌드 및 실행

### 빌드 스크립트 사용 (권장)

```bash
# 모든 테스트 빌드
./build.sh

# 기본 테스트만 빌드
./build.sh -s

# 고급 테스트만 빌드
./build.sh -a

# 빌드 후 실행
./build.sh -r

# 기본 테스트 빌드 후 실행
./build.sh -s -r

# 고급 테스트 빌드 후 실행
./build.sh -a -r

# 디버그 모드로 빌드
./build.sh -d

# 이전 빌드 파일 정리
./build.sh -c

# 도움말 보기
./build.sh --help
```

### 직접 컴파일

```bash
# 기본 GET 요청 테스트
clang++ -o curl_cpp_simple curl_cpp_simple.cpp -lcurl
./curl_cpp_simple

# 고급 HTTP 메서드 테스트 (GET, POST, PUT, DELETE)
clang++ -o advanced_curl_cpp advanced_curl_cpp.cpp -lcurl
./advanced_curl_cpp
```

### CMake 사용 (선택사항)

```bash
# 빌드 디렉토리 생성
mkdir build
cd build

# CMake로 빌드 파일 생성
cmake ..

# 빌드
make

# 실행
./curl_test
```

## 코드 설명

### 기본 테스트 (`curl_cpp_simple.cpp`)
- libcurl을 사용해서 JSONPlaceholder API에 GET 요청을 보내는 간단한 예제
- C 스타일 문자열을 사용하여 C++ 표준 라이브러리 의존성 최소화

### 고급 테스트 (`advanced_curl_cpp.cpp`)
- GET, POST, PUT, DELETE 등 다양한 HTTP 메서드 테스트
- JSON 데이터 전송 및 헤더 설정
- 함수형 프로그래밍 스타일로 HTTP 요청 함수화

### HTTP/3 테스트 (`curl_http3_test.c`)
- libcurl의 HTTP/3 지원 기능을 테스트하는 C 예제
- Cloudflare 등 HTTP/3를 지원하는 사이트에 요청을 보내 실제로 HTTP/3로 통신되는지 확인
- SSL, ALPN, QUIC 등 프로토콜 협상 관련 디버그 로그를 출력
- **직접 빌드한 openssl, nghttp3, curl 환경에서 테스트**

---

## 직접 빌드한 openssl/nghttp3/curl 환경에서의 HTTP/3 테스트

이 프로젝트의 `curl_http3_test.c`는 HTTP/3 지원을 위해 openssl, nghttp3, curl을 직접 소스에서 빌드한 환경에서 테스트되었습니다.

### 의존성 직접 빌드 예시 (macOS 기준)

```bash
# OpenSSL 빌드
wget https://www.openssl.org/source/openssl-3.2.1.tar.gz
# ... 압축 해제 및 ./config --prefix=... && make && make install

# nghttp3 빌드
git clone https://github.com/ngtcp2/nghttp3.git
cd nghttp3 && cmake . && make && make install

# curl (HTTP/3 지원) 빌드
wget https://curl.se/download/curl-8.7.1.tar.gz
# ... 압축 해제 후
./configure --with-openssl=... --with-nghttp3=... --enable-alt-svc --with-ssl --enable-http3
make && make install
```

### curl_http3_test.c 빌드 및 실행

```bash
gcc -o curl_http3_test curl_http3_test.c -I/usr/local/include -L/usr/local/lib -lcurl
./curl_http3_test
```

- 환경에 따라 include/library 경로는 다를 수 있습니다.
- 직접 빌드한 curl이 시스템 기본 curl과 다를 수 있으니, `which curl-config` 등으로 경로 확인 필요

---

### 빌드 스크립트 (`build.sh`)
- 자동화된 빌드 및 실행 스크립트
- 색상 출력 및 에러 처리
- 다양한 빌드 옵션 지원
- 디버그 모드 지원

### 주요 함수
- `WriteCallback`: libcurl의 응답 데이터를 받아서 저장하는 콜백 함수
- `SendRequest`: 다양한 HTTP 메서드로 요청을 보내는 함수

## 예상 출력

### 기본 테스트
```
HTTP 응답 코드: 200
응답 데이터:
{
  "userId": 1,
  "id": 1,
  "title": "sunt aut facere repellat provident occaecati excepturi optio reprehenderit",
  "body": "quia et suscipit suscipit recusandae consequuntur expedita et cum reprehenderit molestiae ut ut quas totam nostrum rerum est autem sunt rem eveniet architecto"
}
```

### 고급 테스트
```
libcurl 고급 C++ 테스트 시작
============================

=== GET https://jsonplaceholder.typicode.com/posts/1 ===
HTTP 응답 코드: 200
응답 데이터: {...}

=== POST https://jsonplaceholder.typicode.com/posts ===
HTTP 응답 코드: 201
응답 데이터: {...}

=== PUT https://jsonplaceholder.typicode.com/posts/1 ===
HTTP 응답 코드: 200
응답 데이터: {...}

=== DELETE https://jsonplaceholder.typicode.com/posts/1 ===
HTTP 응답 코드: 200
응답 데이터: {}

C++ 테스트 완료!
```

## 파일 구조

- `curl_cpp_simple.cpp`: 기본 GET 요청 테스트
- `advanced_curl_cpp.cpp`: 다양한 HTTP 메서드 테스트
- `curl_http3_test.c`: HTTP/3 프로토콜 테스트 (직접 빌드한 openssl/nghttp3/curl 환경 필요)
- `build.sh`: 자동화된 빌드 스크립트
- `CMakeLists.txt`: CMake 빌드 설정 (선택사항)
- `README.md`: 프로젝트 설명서 