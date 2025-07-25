#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include <unistd.h>

// 응답 데이터 구조체
typedef struct {
    char* data;
    long response_code;
    double total_time;
    char* ip_version;
    char* resolved_ip;
    int success;
} ResponseData;

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

// 문자열 복사 함수
char* strdup_safe(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char* new_str = (char*)malloc(len + 1);
    if (new_str) {
        strcpy(new_str, str);
    }
    return new_str;
}

// ResponseData 초기화 함수
ResponseData initResponseData(const char* ip_version) {
    ResponseData result;
    result.data = (char*)malloc(1);
    if (result.data) {
        result.data[0] = 0;
    }
    result.response_code = 0;
    result.total_time = 0.0;
    result.ip_version = strdup_safe(ip_version);
    result.resolved_ip = NULL;
    result.success = 0;
    return result;
}

// ResponseData 정리 함수
void cleanupResponseData(ResponseData* data) {
    if (data) {
        if (data->data) {
            free(data->data);
            data->data = NULL;
        }
        if (data->ip_version) {
            free(data->ip_version);
            data->ip_version = NULL;
        }
        if (data->resolved_ip) {
            free(data->resolved_ip);
            data->resolved_ip = NULL;
        }
    }
}

// HTTP 요청을 수행하는 함수
ResponseData performRequest(const char* url, const char* ip_version, int verbose) {
    ResponseData result = initResponseData(ip_version);
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "libcurl 초기화 실패!\n");
        return result;
    }
    
    // libcurl 옵션 설정
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result.data);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "ipv4-ipv6-test/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    // IP 버전에 따른 설정
    if (strcmp(ip_version, "IPv4") == 0) {
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    } else if (strcmp(ip_version, "IPv6") == 0) {
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6);
    }
    
    // 디버그 모드 설정
    if (verbose) {
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, DebugCallback);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    }
    
    // 요청 시작 시간 기록
    clock_t start_time = clock();
    
    // HTTP 요청 실행
    CURLcode res = curl_easy_perform(curl);
    
    // 요청 종료 시간 기록
    clock_t end_time = clock();
    result.total_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    if (res != CURLE_OK) {
        fprintf(stderr, "%s 요청 실패: %s\n", ip_version, curl_easy_strerror(res));
    } else {
        // HTTP 응답 코드 확인
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.response_code);
        
        // 해결된 IP 주소 가져오기
        char* resolved_ip = nullptr;
        curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &resolved_ip);
        if (resolved_ip) {
            result.resolved_ip = strdup_safe(resolved_ip);
        }
        
        result.success = 1;
    }
    
    // libcurl 정리
    curl_easy_cleanup(curl);
    
    return result;
}

// 기본 IP 해결 동작을 테스트하는 함수
ResponseData performDefaultRequest(const char* url, int verbose) {
    ResponseData result = initResponseData("기본 (WHATEVER)");
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "libcurl 초기화 실패!\n");
        return result;
    }
    
    // libcurl 옵션 설정 (IP 버전 지정하지 않음)
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result.data);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "ipv4-ipv6-test/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    // IP 버전을 명시적으로 설정하지 않음 (기본값: CURL_IPRESOLVE_WHATEVER)
    // curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER); // 이것이 기본값
    
    // 디버그 모드 설정
    if (verbose) {
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, DebugCallback);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    }
    
    // 요청 시작 시간 기록
    clock_t start_time = clock();
    
    // HTTP 요청 실행
    CURLcode res = curl_easy_perform(curl);
    
    // 요청 종료 시간 기록
    clock_t end_time = clock();
    result.total_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    if (res != CURLE_OK) {
        fprintf(stderr, "기본 요청 실패: %s\n", curl_easy_strerror(res));
    } else {
        // HTTP 응답 코드 확인
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.response_code);
        
        // 해결된 IP 주소 가져오기
        char* resolved_ip = nullptr;
        curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &resolved_ip);
        if (resolved_ip) {
            result.resolved_ip = strdup_safe(resolved_ip);
        }
        
        result.success = 1;
    }
    
    // libcurl 정리
    curl_easy_cleanup(curl);
    
    return result;
}

// 결과를 출력하는 함수
void printResult(const ResponseData* result, const char* url) {
    printf("\n=== %s 테스트 결과 ===\n", result->ip_version);
    printf("URL: %s\n", url);
    printf("성공: %s\n", result->success ? "예" : "아니오");
    
    if (result->success) {
        printf("HTTP 응답 코드: %ld\n", result->response_code);
        printf("응답 시간: %.3f초\n", result->total_time);
        printf("해결된 IP: %s\n", result->resolved_ip ? result->resolved_ip : "알 수 없음");
        printf("응답 데이터 길이: %zu 바이트\n", strlen(result->data));
        printf("응답 데이터 (처음 500자):\n%.500s", result->data);
        if (strlen(result->data) > 500) {
            printf("\n... (더 많은 데이터가 있습니다)");
        }
        printf("\n");
    }
    printf("================================\n");
}

// 결과를 비교하는 함수
void compareResults(const ResponseData* ipv4_result, const ResponseData* ipv6_result) {
    printf("\n=== IPv4 vs IPv6 비교 결과 ===\n");
    
    if (!ipv4_result->success && !ipv6_result->success) {
        printf("두 버전 모두 실패했습니다.\n");
        return;
    }
    
    if (ipv4_result->success && ipv6_result->success) {
        printf("두 버전 모두 성공했습니다.\n");
        printf("응답 시간 비교:\n");
        printf("  IPv4: %.3f초\n", ipv4_result->total_time);
        printf("  IPv6: %.3f초\n", ipv6_result->total_time);
        
        double time_diff = ipv4_result->total_time - ipv6_result->total_time;
        if (time_diff > 0) {
            printf("  IPv6이 %.3f초 더 빠릅니다.\n", time_diff);
        } else if (time_diff < 0) {
            printf("  IPv4가 %.3f초 더 빠릅니다.\n", -time_diff);
        } else {
            printf("  응답 시간이 동일합니다.\n");
        }
        
        printf("응답 코드 비교:\n");
        printf("  IPv4: %ld\n", ipv4_result->response_code);
        printf("  IPv6: %ld\n", ipv6_result->response_code);
        
        if (ipv4_result->response_code == ipv6_result->response_code) {
            printf("  응답 코드가 동일합니다.\n");
        } else {
            printf("  응답 코드가 다릅니다.\n");
        }
        
        printf("데이터 길이 비교:\n");
        printf("  IPv4: %zu 바이트\n", strlen(ipv4_result->data));
        printf("  IPv6: %zu 바이트\n", strlen(ipv6_result->data));
        
        if (strlen(ipv4_result->data) == strlen(ipv6_result->data)) {
            printf("  데이터 길이가 동일합니다.\n");
        } else {
            printf("  데이터 길이가 다릅니다.\n");
        }
        
        printf("해결된 IP 비교:\n");
        printf("  IPv4: %s\n", ipv4_result->resolved_ip ? ipv4_result->resolved_ip : "알 수 없음");
        printf("  IPv6: %s\n", ipv6_result->resolved_ip ? ipv6_result->resolved_ip : "알 수 없음");
        
    } else if (ipv4_result->success) {
        printf("IPv4만 성공했습니다.\n");
        printf("IPv6 연결에 실패했습니다.\n");
    } else {
        printf("IPv6만 성공했습니다.\n");
        printf("IPv4 연결에 실패했습니다.\n");
    }
    
    printf("=================================\n");
}

int main() {
    printf("=== IPv4 vs IPv6 vs 기본 동작 테스트 시작 ===\n");
    
    // libcurl 초기화
    curl_global_init(CURL_GLOBAL_ALL);
    
    // 테스트할 URL들
    const char* test_urls[] = {
        "https://httpbin.org/ip",
        "https://api.ipify.org?format=json",
        "https://jsonplaceholder.typicode.com/posts/1",
        "https://httpbin.org/get"
    };
    int num_urls = 4;
    
    for (int i = 0; i < num_urls; i++) {
        const char* url = test_urls[i];
        printf("\n\n=== 테스트 URL: %s ===\n", url);
        
        // 기본 동작 테스트 (IP 버전 지정하지 않음)
        printf("\n--- 기본 동작 요청 시작 (IP 버전 미지정) ---\n");
        ResponseData default_result = performDefaultRequest(url, 0);
        printResult(&default_result, url);
        
        // IPv4 요청
        printf("\n--- IPv4 요청 시작 ---\n");
        ResponseData ipv4_result = performRequest(url, "IPv4", 0);
        printResult(&ipv4_result, url);
        
        // IPv6 요청
        printf("\n--- IPv6 요청 시작 ---\n");
        ResponseData ipv6_result = performRequest(url, "IPv6", 0);
        printResult(&ipv6_result, url);
        
        // 결과 비교 (기본 동작 포함)
        printf("\n=== 기본 동작 vs IPv4 vs IPv6 비교 결과 ===\n");
        
        if (default_result.success) {
            printf("기본 동작 결과:\n");
            printf("  응답 시간: %.3f초\n", default_result.total_time);
            printf("  해결된 IP: %s\n", default_result.resolved_ip ? default_result.resolved_ip : "알 수 없음");
            printf("  응답 코드: %ld\n", default_result.response_code);
            
            // 어떤 IP 버전이 선택되었는지 추측
            if (default_result.resolved_ip && strchr(default_result.resolved_ip, ':')) {
                printf("  추측: IPv6 주소가 선택됨\n");
            } else if (default_result.resolved_ip) {
                printf("  추측: IPv4 주소가 선택됨\n");
            }
        }
        
        if (ipv4_result.success && ipv6_result.success) {
            printf("\nIPv4 vs IPv6 성능 비교:\n");
            double time_diff = ipv4_result.total_time - ipv6_result.total_time;
            if (time_diff > 0) {
                printf("  IPv6이 %.3f초 더 빠릅니다.\n", time_diff);
            } else if (time_diff < 0) {
                printf("  IPv4가 %.3f초 더 빠릅니다.\n", -time_diff);
            } else {
                printf("  응답 시간이 동일합니다.\n");
            }
        }
        
        printf("=========================================\n");
        
        // 메모리 정리
        cleanupResponseData(&default_result);
        cleanupResponseData(&ipv4_result);
        cleanupResponseData(&ipv6_result);
        
        // 잠시 대기 (서버 부하 방지)
        printf("\n3초 대기 중...\n");
        sleep(3);
    }
    
    // libcurl 정리
    curl_global_cleanup();
    
    printf("\n=== IPv4 vs IPv6 vs 기본 동작 테스트 완료 ===\n");
    
    return 0;
} 