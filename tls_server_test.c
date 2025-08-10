#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#define BUFFER_SIZE 4096
#define DEFAULT_PORT 8443

// OpenSSL 초기화 함수
void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_digests();
}

// OpenSSL 정리 함수
void cleanup_openssl() {
    EVP_cleanup();
}

// 자체 서명 인증서 생성 함수 (테스트용)
int create_self_signed_cert(SSL_CTX* ctx) {
    EVP_PKEY* pkey = EVP_PKEY_new();
    X509* x509 = X509_new();
    
    if (!pkey || !x509) {
        printf("인증서 생성 실패: 메모리 할당 오류\n");
        return 0;
    }
    
    // RSA 키 생성
    RSA* rsa = RSA_generate_key(2048, RSA_F4, NULL, NULL);
    if (!rsa) {
        printf("RSA 키 생성 실패\n");
        return 0;
    }
    
    EVP_PKEY_assign_RSA(pkey, rsa);
    
    // 인증서 설정
    X509_set_version(x509, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 365 * 24 * 60 * 60); // 1년
    
    X509_NAME* name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char*)"KR", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "ST", MBSTRING_ASC, (unsigned char*)"Seoul", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "L", MBSTRING_ASC, (unsigned char*)"Seoul", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char*)"Test Organization", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "OU", MBSTRING_ASC, (unsigned char*)"Test Unit", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)"localhost", -1, -1, 0);
    
    X509_set_issuer_name(x509, name);
    X509_set_pubkey(x509, pkey);
    
    // 인증서 서명
    if (!X509_sign(x509, pkey, EVP_sha256())) {
        printf("인증서 서명 실패\n");
        return 0;
    }
    
    // SSL 컨텍스트에 설정
    SSL_CTX_use_certificate(ctx, x509);
    SSL_CTX_use_PrivateKey(ctx, pkey);
    
    // 정리
    X509_free(x509);
    EVP_PKEY_free(pkey);
    
    return 1;
}

// SSL 컨텍스트 생성 함수
SSL_CTX* create_context() {
    const SSL_METHOD* method = SSLv23_server_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    
    if (!ctx) {
        perror("SSL 컨텍스트 생성 실패");
        ERR_print_errors_fp(stderr);
        return NULL;
    }
    
    // 자체 서명 인증서 생성 (테스트용)
    if (!create_self_signed_cert(ctx)) {
        printf("자체 서명 인증서 생성 실패\n");
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    return ctx;
}

// HTTP 응답 생성 함수
void send_http_response(SSL* ssl, const char* status, const char* content_type, const char* body) {
    char response[BUFFER_SIZE];
    char content_length[32];
    
    snprintf(content_length, sizeof(content_length), "%zu", strlen(body));
    
    snprintf(response, sizeof(response),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %s\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status, content_type, content_length, body);
    
    SSL_write(ssl, response, strlen(response));
}

// HTTP 요청 처리 함수
void handle_http_request(SSL* ssl) {
    char buffer[BUFFER_SIZE];
    int bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("=== 수신된 HTTP 요청 ===\n%s\n", buffer);
        
        // 간단한 응답 전송
        const char* response_body = 
            "<html><head><title>TLS Test Server</title></head>"
            "<body><h1>TLS 연결 성공!</h1>"
            "<p>이 페이지는 OpenSSL TLS 서버에서 제공됩니다.</p>"
            "<p>현재 시간: " __DATE__ " " __TIME__ "</p>"
            "</body></html>";
        
        send_http_response(ssl, "200 OK", "text/html; charset=utf-8", response_body);
        printf("=== HTTP 응답 전송 완료 ===\n");
    }
}

// TLS 서버 실행 함수
int run_tls_server(int port) {
    SSL_CTX* ctx;
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    printf("=== TLS 서버 시작 ===\n");
    printf("포트: %d\n", port);
    printf("서버 주소: https://localhost:%d\n\n", port);
    
    // SSL 컨텍스트 생성
    ctx = create_context();
    if (!ctx) {
        return -1;
    }
    
    // 소켓 생성
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("소켓 생성 실패");
        SSL_CTX_free(ctx);
        return -1;
    }
    
    // 소켓 옵션 설정 (재사용)
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    // 바인딩
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("바인딩 실패");
        close(server_sock);
        SSL_CTX_free(ctx);
        return -1;
    }
    
    // 리스닝
    if (listen(server_sock, 5) < 0) {
        perror("리스닝 실패");
        close(server_sock);
        SSL_CTX_free(ctx);
        return -1;
    }
    
    printf("서버가 연결을 기다리는 중...\n");
    printf("(Ctrl+C로 종료)\n\n");
    
    // 클라이언트 연결 처리
    while ((client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len))) {
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        printf("=== 클라이언트 연결 수락 ===\n");
        printf("클라이언트 IP: %s:%d\n", client_ip, ntohs(client_addr.sin_port));
        
        // SSL 생성
        SSL* ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_sock);
        
        // SSL 핸드셰이크
        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            close(client_sock);
            continue;
        }
        
        printf("=== TLS 연결 정보 ===\n");
        printf("프로토콜: %s\n", SSL_get_version(ssl));
        printf("암호화 스위트: %s\n", SSL_get_cipher(ssl));
        printf("=====================\n");
        
        // HTTP 요청 처리
        handle_http_request(ssl);
        
        // 연결 종료
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(client_sock);
        
        printf("=== 클라이언트 연결 종료 ===\n\n");
    }
    
    // 정리
    close(server_sock);
    SSL_CTX_free(ctx);
    
    return 0;
}

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;
    
    if (argc > 1) {
        port = atoi(argv[1]);
    }
    
    printf("OpenSSL TLS 서버 테스트\n");
    printf("사용법: %s [port]\n", argv[0]);
    printf("기본 포트: %d\n\n", DEFAULT_PORT);
    
    // OpenSSL 초기화
    init_openssl();
    
    // TLS 서버 실행
    if (run_tls_server(port) == 0) {
        printf("✅ TLS 서버가 정상적으로 종료되었습니다.\n");
    } else {
        printf("❌ TLS 서버 실행 중 오류가 발생했습니다.\n");
    }
    
    // OpenSSL 정리
    cleanup_openssl();
    
    return 0;
} 