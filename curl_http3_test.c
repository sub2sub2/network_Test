#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// 디버그 콜백 함수 (SSL, ALPN, protocol 관련 로그만 출력)
int debug_callback(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr) {
    (void)handle; (void)userptr;
    if (type == CURLINFO_TEXT || type == CURLINFO_SSL_DATA_IN || type == CURLINFO_SSL_DATA_OUT) {
        // SSL 핸드셰이크, ALPN, protocol 협상 관련 로그만 필터링
        if (strstr(data, "SSL") || strstr(data, "ALPN") || strstr(data, "protocol") || strstr(data, "QUIC")) {
            fprintf(stderr, "[CURL-DEBUG] %.*s", (int)size, data);
        }
    }
    return 0;
}

// 응답 데이터를 출력하는 콜백 함수
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    // 본문은 무시
    return size * nmemb;
}

int main(void) {
    CURL *curl;
    CURLcode res;
    long response_code = 0;
    char errbuf[CURL_ERROR_SIZE] = {0};
    long http_version = 0;

    // libcurl 전역 초기화
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
        fprintf(stderr, "curl_global_init() 실패\n");
        return 1;
    }

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "curl_easy_init() 실패\n");
        curl_global_cleanup();
        return 1;
    }

    // 요청할 URL (HTTP/3 지원 사이트)
    const char *url = "https://cloudflare.com";
    curl_easy_setopt(curl, CURLOPT_URL, url);

    // HTTP/3 사용 옵션
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_3);

    // 리다이렉트 자동 추적
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // 에러 메시지 버퍼 설정
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

    // 응답 데이터 콜백 설정 (본문 무시)
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    // 디버그 로그 출력 옵션 (필터링)
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_callback);

    printf("🚀 libcurl로 HTTP/3 요청: %s\n", url);
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "\n❌ 요청 실패: %s\n", errbuf[0] ? errbuf : curl_easy_strerror(res));
    } else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        curl_easy_getinfo(curl, CURLINFO_HTTP_VERSION, &http_version);
        printf("\n✅ HTTP 응답 코드: %ld\n", response_code);
        // 연결된 실제 프로토콜 출력
        switch (http_version) {
            case CURL_HTTP_VERSION_1_0: printf("실제 연결 프로토콜: HTTP/1.0\n"); break;
            case CURL_HTTP_VERSION_1_1: printf("실제 연결 프로토콜: HTTP/1.1\n"); break;
            case CURL_HTTP_VERSION_2:   printf("실제 연결 프로토콜: HTTP/2\n"); break;
            case CURL_HTTP_VERSION_3:   printf("실제 연결 프로토콜: HTTP/3\n"); break;
            default:                    printf("실제 연결 프로토콜: 알 수 없음\n"); break;
        }
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return 0;
} 