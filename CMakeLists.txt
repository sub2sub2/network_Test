cmake_minimum_required(VERSION 3.10)
project(libcurl_test)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 실행 파일 생성
add_executable(curl_test main.cpp)

# libcurl 찾기 및 링크
find_package(CURL REQUIRED)
target_link_libraries(curl_test CURL::libcurl) 