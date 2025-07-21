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
- `build.sh`: 자동화된 빌드 스크립트
- `CMakeLists.txt`: CMake 빌드 설정 (선택사항)
- `README.md`: 프로젝트 설명서 