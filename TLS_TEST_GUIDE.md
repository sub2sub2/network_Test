# OpenSSL TLS 테스트 가이드

이 프로젝트는 OpenSSL을 사용한 TLS 클라이언트와 서버 테스트 코드를 포함합니다.

## 빌드

### 모든 테스트 빌드
```bash
./build.sh
```

### TLS 테스트만 빌드
```bash
./build.sh -t
```

### TLS 테스트 빌드 후 실행
```bash
./build.sh -t -r
```

## TLS 클라이언트 테스트

### 기본 사용법
```bash
./tls_client_test <hostname> [port] [path]
```

### 예시
```bash
# Google 홈페이지 테스트
./tls_client_test www.google.com 443 /

# HTTPBin API 테스트
./tls_client_test httpbin.org 443 /get

# JSONPlaceholder API 테스트
./tls_client_test jsonplaceholder.typicode.com 443 /posts/1
```

### 출력 정보
- DNS 해결 결과
- TLS 프로토콜 버전 (TLSv1.2, TLSv1.3 등)
- 암호화 스위트 (ECDHE-RSA-AES128-GCM-SHA256 등)
- 인증서 정보 (주체, 발급자)
- HTTP 요청/응답 데이터

## TLS 서버 테스트

### 기본 사용법
```bash
./tls_server_test [port]
```

### 예시
```bash
# 기본 포트(8443)로 서버 시작
./tls_server_test

# 사용자 정의 포트로 서버 시작
./tls_server_test 9443
```

### 서버 기능
- 자체 서명 인증서 자동 생성
- TLS 핸드셰이크 처리
- HTTP 요청 처리 및 응답
- 간단한 HTML 페이지 제공

### 클라이언트에서 서버 테스트
```bash
# 브라우저에서 접속
https://localhost:8443

# curl로 테스트
curl -k https://localhost:8443

# TLS 클라이언트로 테스트
./tls_client_test localhost 8443 /
```

## 주요 기능

### TLS 클라이언트
- DNS 해결 (도메인 → IP 주소)
- TLS 핸드셰이크
- SNI (Server Name Indication) 지원
- 인증서 정보 출력
- HTTP 요청/응답 처리

### TLS 서버
- 자체 서명 인증서 생성
- TLS 핸드셰이크 처리
- HTTP 요청 처리
- 간단한 웹 페이지 제공

## 보안 주의사항

1. **자체 서명 인증서**: 테스트 서버는 자체 서명 인증서를 사용합니다.
   - 브라우저에서 보안 경고가 표시될 수 있습니다.
   - `curl -k` 옵션으로 인증서 검증을 건너뛸 수 있습니다.

2. **인증서 검증**: 클라이언트는 서버 인증서를 검증하지 않습니다.
   - 실제 운영 환경에서는 적절한 인증서 검증이 필요합니다.

3. **테스트 목적**: 이 코드는 교육 및 테스트 목적으로만 사용하세요.

## 문제 해결

### 빌드 오류
```bash
# OpenSSL 경로 확인
brew --prefix openssl@3

# OpenSSL 설치
brew install openssl@3
```

### 연결 오류
```bash
# DNS 해결 확인
nslookup www.google.com

# 포트 확인
telnet www.google.com 443
```

### 인증서 오류
```bash
# 인증서 검증 건너뛰기 (테스트용)
curl -k https://localhost:8443
```

## 코드 구조

### tls_client_test.c
- `init_openssl()`: OpenSSL 초기화
- `create_context()`: SSL 컨텍스트 생성
- `resolve_hostname()`: DNS 해결
- `connect_to_server()`: TLS 연결 설정
- `print_ssl_info()`: SSL 정보 출력
- `send_http_request()`: HTTP 요청 전송

### tls_server_test.c
- `init_openssl()`: OpenSSL 초기화
- `create_self_signed_cert()`: 자체 서명 인증서 생성
- `create_context()`: SSL 컨텍스트 생성
- `handle_http_request()`: HTTP 요청 처리
- `send_http_response()`: HTTP 응답 생성
- `run_tls_server()`: TLS 서버 실행

## 참고 자료

- [OpenSSL 문서](https://www.openssl.org/docs/)
- [TLS 프로토콜](https://tools.ietf.org/html/rfc5246)
- [HTTP/1.1](https://tools.ietf.org/html/rfc2616) 