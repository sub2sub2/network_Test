#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// libcurl 콜백 함수 - 응답 데이터를 받아서 저장
size_t WriteCallback(void* contents, size_t size, size_t nmemb, char** userp) {
    size_t realsize = size * nmemb;
    char* ptr = (char*)realloc(*userp, realsize + 1);
    
    if (ptr == NULL) {
        return 0;
    }
    
    *userp = ptr;
    memcpy(*userp + strlen(*userp), contents, realsize);
    (*userp)[strlen(*userp) + realsize] = 0;
    
    return realsize;
}

// libcurl 디버그 콜백 함수
int DebugCallback(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr) {
    const char* type_str = "";
    
    switch (type) {
        case CURLINFO_TEXT:
            type_str = "TEXT";
            break;
        case CURLINFO_HEADER_IN:
            type_str = "HEADER_IN";
            break;
        case CURLINFO_HEADER_OUT:
            type_str = "HEADER_OUT";
            break;
        case CURLINFO_DATA_IN:
            type_str = "DATA_IN";
            break;
        case CURLINFO_DATA_OUT:
            type_str = "DATA_OUT";
            break;
        case CURLINFO_SSL_DATA_IN:
            type_str = "SSL_DATA_IN";
            break;
        case CURLINFO_SSL_DATA_OUT:
            type_str = "SSL_DATA_OUT";
            break;
        default:
            type_str = "UNKNOWN";
            break;
    }
    
    // 데이터를 문자열로 변환 (null 종료)
    char* str_data = (char*)malloc(size + 1);
    memcpy(str_data, data, size);
    str_data[size] = '\0';
    
    // 개행 문자 제거
    if (str_data[size-1] == '\n') {
        str_data[size-1] = '\0';
    }
    
    printf("[DEBUG][%s] %s\n", type_str, str_data);
    
    free(str_data);
    return 0;
}

int main() {
    printf("=== libcurl 디버그 테스트 시작 ===\n\n");
    
    // libcurl 초기화
    CURL* curl = curl_easy_init();
    
    if (!curl) {
        fprintf(stderr, "libcurl 초기화 실패!\n");
        return 1;
    }
    
    // 테스트할 URL (예: JSONPlaceholder API)
    const char* url = "https://jsonplaceholder.typicode.com/posts/1";
    printf("요청 URL: %s\n\n", url);
    
    // 응답 데이터를 저장할 문자열
    char* response = (char*)malloc(1);
    response[0] = 0;
    
    // libcurl 옵션 설정
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-test/1.0");
    
    // 커스텀 디버그 콜백 설정
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, DebugCallback);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    
    // 추가 디버그 옵션들
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    printf("=== libcurl 디버그 로그 시작 ===\n");
    printf("(아래에 libcurl의 상세한 디버그 정보가 출력됩니다)\n");
    printf("=====================================\n\n");
    
    // HTTP 요청 실행
    CURLcode res = curl_easy_perform(curl);
    
    printf("\n=== libcurl 디버그 로그 종료 ===\n\n");
    
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() 실패: %s\n", curl_easy_strerror(res));
    } else {
        // HTTP 응답 코드 확인
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        printf("=== 최종 결과 ===\n");
        printf("HTTP 응답 코드: %ld\n", response_code);
        printf("응답 데이터:\n%s\n", response);
        printf("================\n");
    }
    
    // 메모리 정리
    free(response);
    
    // libcurl 정리
    curl_easy_cleanup(curl);
    
    return 0;
} 