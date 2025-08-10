# OpenSSL 인증서 생성 및 사용 가이드

이 문서는 OpenSSL을 사용한 인증서 생성과 TLS 서버에서의 사용 방법을 설명합니다.

## 인증서 생성 방법

### 1. 자동 생성 스크립트 사용 (권장)

#### 기본 사용법
```bash
cd certs
./generate_cert.sh [도메인명]
```

#### 예시
```bash
# localhost용 인증서 생성
./generate_cert.sh localhost

# 특정 도메인용 인증서 생성
./generate_cert.sh example.com
```

#### 생성되는 파일들
- `server.key`: 개인키 (RSA 2048비트)
- `server.crt`: 인증서 (PEM 형식)
- `server.csr`: 인증서 서명 요청 (CSR)
- `openssl.conf`: OpenSSL 설정 파일

### 2. 수동 생성 방법

#### 1단계: 개인키 생성
```bash
openssl genrsa -out server.key 2048
```

#### 2단계: 인증서 서명 요청(CSR) 생성
```bash
openssl req -new -key server.key -out server.csr -config openssl.conf
```

#### 3단계: 자체 서명 인증서 생성
```bash
openssl x509 -req -in server.csr -signkey server.key -out server.crt -days 365 -extensions v3_req -extfile openssl.conf
```

## 인증서 정보

### 생성된 인증서 상세 정보
```
Subject: C=KR, ST=Seoul, L=Seoul, O=Test Organization, OU=Test Unit, CN=localhost
Issuer: C=KR, ST=Seoul, L=Seoul, O=Test Organization, OU=Test Unit, CN=localhost
Not Before: Aug 10 13:30:11 2025 GMT
Not After: Aug 10 13:30:11 2026 GMT
DNS: localhost, *.localhost, localhost
IP Address: 127.0.0.1, ::1
```

### 필드 설명
- **C (Country)**: 국가 코드 (KR)
- **ST (State)**: 주/도 (Seoul)
- **L (Locality)**: 도시 (Seoul)
- **O (Organization)**: 조직명 (Test Organization)
- **OU (Organizational Unit)**: 조직 단위 (Test Unit)
- **CN (Common Name)**: 일반 이름 (localhost)

## TLS 서버에서 인증서 사용

### 1. 메모리 기반 인증서 (기존 방식)
```c
// tls_server_test.c
int create_self_signed_cert(SSL_CTX* ctx) {
    // 코드에서 직접 인증서 생성
    EVP_PKEY* pkey = EVP_PKEY_new();
    X509* x509 = X509_new();
    // ... 인증서 생성 로직
}
```

**장점:**
- 별도 파일 불필요
- 서버 시작 시마다 새로운 인증서

**단점:**
- 서버 재시작 시마다 인증서 변경
- 인증서 정보 수정 어려움

### 2. 파일 기반 인증서 (새로운 방식)
```c
// tls_server_file_test.c
SSL_CTX* create_context() {
    // 파일에서 인증서 로드
    SSL_CTX_use_certificate_file(ctx, CERT_FILE, SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx, KEY_FILE, SSL_FILETYPE_PEM);
    SSL_CTX_check_private_key(ctx);  // 일치성 확인
}
```

**장점:**
- 인증서 재사용 가능
- 인증서 정보 수정 용이
- 실제 운영 환경과 유사

**단점:**
- 파일 관리 필요
- 보안상 파일 권한 관리 중요

## 보안 고려사항

### 1. 파일 권한 설정
```bash
# 개인키는 소유자만 읽기/쓰기 가능
chmod 600 server.key

# 인증서는 읽기 가능
chmod 644 server.crt
```

### 2. 인증서 검증
```c
// 인증서와 개인키 일치 확인
if (!SSL_CTX_check_private_key(ctx)) {
    printf("인증서와 개인키가 일치하지 않습니다.\n");
    return NULL;
}
```

### 3. Subject Alternative Names (SAN)
```ini
[alt_names]
DNS.1 = localhost
DNS.2 = *.localhost
DNS.3 = localhost
IP.1 = 127.0.0.1
IP.2 = ::1
```

## 인증서 관리 명령어

### 인증서 정보 확인
```bash
# 인증서 상세 정보
openssl x509 -in server.crt -text -noout

# 인증서 요약 정보
openssl x509 -in server.crt -noout -subject -issuer -dates

# 인증서 지문
openssl x509 -in server.crt -noout -fingerprint
```

### 인증서 유효성 검증
```bash
# 인증서 체인 검증
openssl verify server.crt

# 개인키와 인증서 일치 확인
openssl x509 -noout -modulus -in server.crt | openssl md5
openssl rsa -noout -modulus -in server.key | openssl md5
```

### 인증서 변환
```bash
# PEM → DER 변환
openssl x509 -in server.crt -outform DER -out server.der

# DER → PEM 변환
openssl x509 -in server.der -inform DER -out server.pem
```

## 실제 운영 환경 고려사항

### 1. CA 서명 인증서 사용
```bash
# CA에서 서명된 인증서 사용
SSL_CTX_use_certificate_file(ctx, "ca_signed.crt", SSL_FILETYPE_PEM);
```

### 2. 인증서 체인 설정
```bash
# 중간 인증서 포함
SSL_CTX_use_certificate_chain_file(ctx, "cert_chain.pem");
```

### 3. 인증서 자동 갱신
```bash
# Let's Encrypt 등에서 자동 갱신
certbot renew
```

## 문제 해결

### 인증서 로드 실패
```bash
# 파일 존재 확인
ls -la certs/

# 파일 권한 확인
ls -la certs/server.key certs/server.crt

# OpenSSL 오류 확인
openssl x509 -in certs/server.crt -text -noout
```

### 인증서 불일치 오류
```bash
# 개인키와 인증서 일치 확인
openssl x509 -noout -modulus -in certs/server.crt | openssl md5
openssl rsa -noout -modulus -in certs/server.key | openssl md5
```

### 브라우저 보안 경고
- 자체 서명 인증서는 브라우저에서 보안 경고 표시
- 개발/테스트 환경에서만 사용
- 운영 환경에서는 CA 서명 인증서 사용

## 참고 자료

- [OpenSSL 문서](https://www.openssl.org/docs/)
- [X.509 인증서](https://tools.ietf.org/html/rfc5280)
- [TLS 프로토콜](https://tools.ietf.org/html/rfc5246)
- [Let's Encrypt](https://letsencrypt.org/) 