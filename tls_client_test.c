#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BUFFER_SIZE 4096
#define DEFAULT_PORT 443

// OpenSSL 초기화 함수
void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

// OpenSSL 정리 함수
void cleanup_openssl() {
    EVP_cleanup();
}

// SSL 컨텍스트 생성 함수
SSL_CTX* create_context() {
    const SSL_METHOD* method = SSLv23_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    
    if (!ctx) {
        perror("SSL 컨텍스트 생성 실패");
        ERR_print_errors_fp(stderr);
        return NULL;
    }
    
    return ctx;
}

// DNS 해결 함수
int resolve_hostname(const char* hostname, char* ip_address) {
    struct addrinfo hints, *result, *rp;
    int status;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4만 사용
    hints.ai_socktype = SOCK_STREAM;
    
    status = getaddrinfo(hostname, NULL, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "DNS 해결 실패: %s\n", gai_strerror(status));
        return -1;
    }
    
    // 첫 번째 IPv4 주소 사용
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if (rp->ai_family == AF_INET) {
            struct sockaddr_in* addr = (struct sockaddr_in*)rp->ai_addr;
            strcpy(ip_address, inet_ntoa(addr->sin_addr));
            freeaddrinfo(result);
            return 0;
        }
    }
    
    freeaddrinfo(result);
    return -1;
}

// SSL 연결 설정 함수
SSL* connect_to_server(SSL_CTX* ctx, const char* hostname, int port) {
    int sock;
    struct sockaddr_in addr;
    SSL* ssl;
    char ip_address[INET_ADDRSTRLEN];
    
    // DNS 해결
    if (resolve_hostname(hostname, ip_address) != 0) {
        fprintf(stderr, "호스트명 해결 실패: %s\n", hostname);
        return NULL;
    }
    
    printf("DNS 해결: %s -> %s\n", hostname, ip_address);
    
    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("소켓 생성 실패");
        return NULL;
    }
    
    // 서버 주소 설정
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip_address);
    
    // 연결
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        perror("서버 연결 실패");
        close(sock);
        return NULL;
    }
    
    // SSL 생성
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    
    // SNI (Server Name Indication) 설정
    SSL_set_tlsext_host_name(ssl, hostname);
    
    // SSL 핸드셰이크
    if (SSL_connect(ssl) != 1) {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sock);
        return NULL;
    }
    
    return ssl;
}

// SSL 정보 출력 함수
void print_ssl_info(SSL* ssl, const char* hostname) {
    printf("=== TLS 연결 정보 ===\n");
    printf("호스트: %s\n", hostname);
    printf("프로토콜: %s\n", SSL_get_version(ssl));
    printf("암호화 스위트: %s\n", SSL_get_cipher(ssl));
    printf("인증서 검증: %s\n", SSL_get_verify_result(ssl) == X509_V_OK ? "성공" : "실패");
    
    // 인증서 정보 출력
    X509* cert = SSL_get_peer_certificate(ssl);
    if (cert) {
        char* subject = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        char* issuer = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        
        printf("서버 인증서 주체: %s\n", subject);
        printf("인증서 발급자: %s\n", issuer);
        
        free(subject);
        free(issuer);
        X509_free(cert);
    }
    printf("=====================\n\n");
}

// HTTP GET 요청 전송 함수
int send_http_request(SSL* ssl, const char* hostname, const char* path) {
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int bytes;
    
    // HTTP GET 요청 생성
    snprintf(request, sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: OpenSSL-TLS-Test/1.0\r\n"
        "Connection: close\r\n"
        "\r\n",
        path, hostname);
    
    printf("=== HTTP 요청 전송 ===\n");
    printf("요청:\n%s", request);
    
    // 요청 전송
    if (SSL_write(ssl, request, strlen(request)) <= 0) {
        ERR_print_errors_fp(stderr);
        return -1;
    }
    
    // 응답 수신
    printf("\n=== HTTP 응답 수신 ===\n");
    while ((bytes = SSL_read(ssl, response, sizeof(response) - 1)) > 0) {
        response[bytes] = '\0';
        printf("%s", response);
    }
    
    if (bytes < 0) {
        ERR_print_errors_fp(stderr);
        return -1;
    }
    
    printf("\n========================\n");
    return 0;
}

// TLS 연결 테스트 함수
int test_tls_connection(const char* hostname, int port, const char* path) {
    SSL_CTX* ctx;
    SSL* ssl;
    int result = 0;
    
    printf("=== TLS 연결 테스트 시작 ===\n");
    printf("호스트: %s:%d\n", hostname, port);
    printf("경로: %s\n\n", path);
    
    // SSL 컨텍스트 생성
    ctx = create_context();
    if (!ctx) {
        return -1;
    }
    
    // 서버에 연결
    ssl = connect_to_server(ctx, hostname, port);
    if (!ssl) {
        SSL_CTX_free(ctx);
        return -1;
    }
    
    // SSL 정보 출력
    print_ssl_info(ssl, hostname);
    
    // HTTP 요청 전송
    if (send_http_request(ssl, hostname, path) != 0) {
        result = -1;
    }
    
    // 정리
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    
    return result;
}

int main(int argc, char* argv[]) {
    const char* hostname;
    int port = DEFAULT_PORT;
    const char* path = "/";
    
    if (argc < 2) {
        printf("사용법: %s <hostname> [port] [path]\n", argv[0]);
        printf("예시: %s www.google.com 443 /\n", argv[0]);
        printf("예시: %s httpbin.org 443 /get\n", argv[0]);
        return 1;
    }
    
    hostname = argv[1];
    
    if (argc > 2) {
        port = atoi(argv[2]);
    }
    
    if (argc > 3) {
        path = argv[3];
    }
    
    // OpenSSL 초기화
    init_openssl();
    
    // TLS 연결 테스트
    if (test_tls_connection(hostname, port, path) == 0) {
        printf("✅ TLS 연결 테스트 성공!\n");
    } else {
        printf("❌ TLS 연결 테스트 실패!\n");
    }
    
    // OpenSSL 정리
    cleanup_openssl();
    
    return 0;
} 