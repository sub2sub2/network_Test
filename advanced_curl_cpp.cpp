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

// HTTP 요청을 보내는 함수
int SendRequest(const char* url, const char* method, const char* data, const char* content_type) {
    CURL* curl;
    CURLcode res;
    char* response = (char*)malloc(1);
    response[0] = 0;
    
    // libcurl 초기화
    curl = curl_easy_init();
    
    if (!curl) {
        fprintf(stderr, "libcurl 초기화 실패!\n");
        return 1;
    }
    
    // 기본 옵션 설정
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-test/1.0");
    
    // HTTP 메서드 설정
    if (strcmp(method, "POST") == 0) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (data) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        }
    } else if (strcmp(method, "PUT") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        if (data) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        }
    } else if (strcmp(method, "DELETE") == 0) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    
    // Content-Type 헤더 설정
    if (content_type) {
        struct curl_slist* headers = NULL;
        char content_type_header[256];
        snprintf(content_type_header, sizeof(content_type_header), "Content-Type: %s", content_type);
        headers = curl_slist_append(headers, content_type_header);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
    
    // HTTP 요청 실행
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() 실패: %s\n", curl_easy_strerror(res));
    } else {
        // HTTP 응답 코드 확인
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        printf("=== %s %s ===\n", method, url);
        printf("HTTP 응답 코드: %ld\n", response_code);
        printf("응답 데이터:\n%s\n\n", response);
    }
    
    // 메모리 정리
    free(response);
    
    // libcurl 정리
    curl_easy_cleanup(curl);
    
    return 0;
}

int main() {
    printf("libcurl 고급 C++ 테스트 시작\n");
    printf("============================\n\n");
    
    // GET 요청 테스트
    SendRequest("https://jsonplaceholder.typicode.com/posts/1", "GET", NULL, NULL);
    
    // POST 요청 테스트
    const char* post_data = "{\"title\":\"libcurl C++ test\",\"body\":\"This is a test post from C++\",\"userId\":1}";
    SendRequest("https://jsonplaceholder.typicode.com/posts", "POST", post_data, "application/json");
    
    // PUT 요청 테스트
    const char* put_data = "{\"id\":1,\"title\":\"Updated title from C++\",\"body\":\"Updated body from C++\",\"userId\":1}";
    SendRequest("https://jsonplaceholder.typicode.com/posts/1", "PUT", put_data, "application/json");
    
    // DELETE 요청 테스트
    SendRequest("https://jsonplaceholder.typicode.com/posts/1", "DELETE", NULL, NULL);
    
    printf("C++ 테스트 완료!\n");
    
    return 0;
} 