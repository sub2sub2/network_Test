#!/bin/bash

# OpenSSL 인증서 생성 스크립트
# 사용법: ./generate_cert.sh [도메인명]

# 색상 정의
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 함수: 색상 출력
print_color() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

print_success() {
    print_color $GREEN "✅ $1"
}

print_info() {
    print_color $BLUE "ℹ️  $1"
}

print_warning() {
    print_color $YELLOW "⚠️  $1"
}

print_error() {
    print_color $RED "❌ $1"
}

# 기본값 설정
DOMAIN=${1:-"localhost"}
CERT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KEY_FILE="$CERT_DIR/server.key"
CERT_FILE="$CERT_DIR/server.crt"
CONFIG_FILE="$CERT_DIR/openssl.conf"

print_info "OpenSSL 인증서 생성 스크립트"
print_info "도메인: $DOMAIN"
print_info "인증서 디렉토리: $CERT_DIR"
echo ""

# OpenSSL 설정 파일 생성
print_info "OpenSSL 설정 파일 생성 중..."
cat > "$CONFIG_FILE" << EOF
[req]
distinguished_name = req_distinguished_name
req_extensions = v3_req
prompt = no

[req_distinguished_name]
C = KR
ST = Seoul
L = Seoul
O = Test Organization
OU = Test Unit
CN = $DOMAIN

[v3_req]
keyUsage = keyEncipherment, dataEncipherment
extendedKeyUsage = serverAuth
subjectAltName = @alt_names

[alt_names]
DNS.1 = $DOMAIN
DNS.2 = *.$DOMAIN
DNS.3 = localhost
IP.1 = 127.0.0.1
IP.2 = ::1
EOF

print_success "설정 파일 생성 완료: $CONFIG_FILE"

# 개인키 생성 (RSA 2048비트)
print_info "개인키 생성 중..."
if openssl genrsa -out "$KEY_FILE" 2048 2>/dev/null; then
    print_success "개인키 생성 완료: $KEY_FILE"
else
    print_error "개인키 생성 실패"
    exit 1
fi

# 인증서 서명 요청(CSR) 생성
print_info "인증서 서명 요청(CSR) 생성 중..."
if openssl req -new -key "$KEY_FILE" -out "$CERT_DIR/server.csr" -config "$CONFIG_FILE" 2>/dev/null; then
    print_success "CSR 생성 완료: $CERT_DIR/server.csr"
else
    print_error "CSR 생성 실패"
    exit 1
fi

# 자체 서명 인증서 생성
print_info "자체 서명 인증서 생성 중..."
if openssl x509 -req -in "$CERT_DIR/server.csr" -signkey "$KEY_FILE" -out "$CERT_FILE" -days 365 -extensions v3_req -extfile "$CONFIG_FILE" 2>/dev/null; then
    print_success "인증서 생성 완료: $CERT_FILE"
else
    print_error "인증서 생성 실패"
    exit 1
fi

# 파일 권한 설정
chmod 600 "$KEY_FILE"
chmod 644 "$CERT_FILE"

# 인증서 정보 출력
print_info "생성된 인증서 정보:"
echo "----------------------------------------"
openssl x509 -in "$CERT_FILE" -text -noout | grep -E "(Subject:|Issuer:|Not Before|Not After|DNS:|IP Address:)"
echo "----------------------------------------"

# 파일 목록 출력
print_info "생성된 파일 목록:"
ls -la "$CERT_DIR"/*.key "$CERT_DIR"/*.crt "$CERT_DIR"/*.csr "$CERT_DIR"/*.conf 2>/dev/null

echo ""
print_success "인증서 생성 완료!"
print_info "사용법:"
echo "  개인키: $KEY_FILE"
echo "  인증서: $CERT_FILE"
echo "  설정파일: $CONFIG_FILE"
echo ""
print_warning "주의: 이 인증서는 테스트 목적으로만 사용하세요." 