#!/bin/bash

# libcurl C++ 프로젝트 빌드 스크립트
# 사용법: ./build.sh [옵션]

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

# 함수: 에러 출력
print_error() {
    print_color $RED "❌ $1"
}

# 함수: 성공 출력
print_success() {
    print_color $GREEN "✅ $1"
}

# 함수: 정보 출력
print_info() {
    print_color $BLUE "ℹ️  $1"
}

# 함수: 경고 출력
print_warning() {
    print_color $YELLOW "⚠️  $1"
}

# 함수: 도움말 출력
show_help() {
    echo "libcurl C++ 프로젝트 빌드 스크립트"
    echo ""
    echo "사용법: $0 [옵션]"
    echo ""
    echo "옵션:"
    echo "  -h, --help     이 도움말을 표시"
    echo "  -c, --clean    이전 빌드 파일들을 정리"
    echo "  -s, --simple   기본 테스트만 빌드"
    echo "  -a, --advanced 고급 테스트만 빌드"
    echo "  -i, --ipv6     IPv4/IPv6 테스트만 빌드"
    echo "  -r, --run      빌드 후 실행"
    echo "  -d, --debug    디버그 모드로 빌드"
    echo ""
    echo "예시:"
    echo "  $0              # 모든 테스트 빌드"
    echo "  $0 -s -r        # 기본 테스트 빌드 후 실행"
    echo "  $0 -a -r        # 고급 테스트 빌드 후 실행"
    echo "  $0 -i -r        # IPv4/IPv6 테스트 빌드 후 실행"
    echo "  $0 -c           # 정리만 수행"
}

# 변수 초기화
CLEAN=false
BUILD_SIMPLE=false
BUILD_ADVANCED=false
BUILD_IPV6=false
RUN_AFTER_BUILD=false
DEBUG_MODE=false
COMPILER="g++" # 기본값

# 명령행 인수 파싱
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -s|--simple)
            BUILD_SIMPLE=true
            shift
            ;;
        -a|--advanced)
            BUILD_ADVANCED=true
            shift
            ;;
        -i|--ipv6)
            BUILD_IPV6=true
            shift
            ;;
        -r|--run)
            RUN_AFTER_BUILD=true
            shift
            ;;
        -d|--debug)
            DEBUG_MODE=true
            shift
            ;;
        *)
            print_error "알 수 없는 옵션: $1"
            show_help
            exit 1
            ;;
    esac
done

# 기본값 설정 (옵션이 없으면 모든 테스트 빌드)
if [[ "$BUILD_SIMPLE" == false && "$BUILD_ADVANCED" == false && "$BUILD_IPV6" == false ]]; then
    BUILD_SIMPLE=true
    BUILD_ADVANCED=true
    BUILD_IPV6=true
fi

# 컴파일러 확인
if ! command -v g++ &> /dev/null; then
    if ! command -v clang++ &> /dev/null; then
        print_error "g++ 또는 clang++ 컴파일러를 찾을 수 없습니다."
        exit 1
    else
        COMPILER="clang++"
    fi
else
    COMPILER="g++"
fi

print_info "사용할 컴파일러: $COMPILER"

# libcurl 확인
if ! pkg-config --exists libcurl; then
    print_warning "libcurl이 pkg-config로 감지되지 않습니다. 시스템 libcurl을 사용합니다."
fi

# 정리 모드
if [[ "$CLEAN" == true ]]; then
    print_info "이전 빌드 파일들을 정리합니다..."
    rm -f curl_cpp_simple advanced_curl_cpp ipv4_ipv6_test
    print_success "정리 완료"
    exit 0
fi

# 컴파일 옵션 설정
COMPILE_FLAGS="-std=c++17 -Wall -Wextra"
if [[ "$DEBUG_MODE" == true ]]; then
    COMPILE_FLAGS="$COMPILE_FLAGS -g -O0"
    print_info "디버그 모드로 빌드합니다..."
else
    COMPILE_FLAGS="$COMPILE_FLAGS -O2"
fi

# 기본 테스트 빌드
if [[ "$BUILD_SIMPLE" == true ]]; then
    print_info "기본 테스트를 빌드합니다..."
    if $COMPILER $COMPILE_FLAGS -o curl_cpp_simple curl_cpp_simple.cpp -lcurl; then
        print_success "기본 테스트 빌드 완료: curl_cpp_simple"
        
        if [[ "$RUN_AFTER_BUILD" == true ]]; then
            print_info "기본 테스트를 실행합니다..."
            echo "----------------------------------------"
            ./curl_cpp_simple
            echo "----------------------------------------"
        fi
    else
        print_error "기본 테스트 빌드 실패"
        exit 1
    fi
fi

# 고급 테스트 빌드
if [[ "$BUILD_ADVANCED" == true ]]; then
    print_info "고급 테스트를 빌드합니다..."
    if $COMPILER $COMPILE_FLAGS -o advanced_curl_cpp advanced_curl_cpp.cpp -lcurl; then
        print_success "고급 테스트 빌드 완료: advanced_curl_cpp"
        
        if [[ "$RUN_AFTER_BUILD" == true ]]; then
            print_info "고급 테스트를 실행합니다..."
            echo "----------------------------------------"
            ./advanced_curl_cpp
            echo "----------------------------------------"
        fi
    else
        print_error "고급 테스트 빌드 실패"
        exit 1
    fi
fi

# IPv4/IPv6 테스트 빌드
if [[ "$BUILD_IPV6" == true ]]; then
    print_info "IPv4/IPv6 테스트를 빌드합니다..."
    if $COMPILER $COMPILE_FLAGS -o ipv4_ipv6_test ipv4_ipv6_test.cpp -lcurl; then
        print_success "IPv4/IPv6 테스트 빌드 완료: ipv4_ipv6_test"
        
        if [[ "$RUN_AFTER_BUILD" == true ]]; then
            print_info "IPv4/IPv6 테스트를 실행합니다..."
            echo "----------------------------------------"
            ./ipv4_ipv6_test
            echo "----------------------------------------"
        fi
    else
        print_error "IPv4/IPv6 테스트 빌드 실패"
        exit 1
    fi
fi

print_success "빌드 완료!"
echo ""
print_info "사용 가능한 실행 파일:"
if [[ -f "curl_cpp_simple" ]]; then
    echo "  ./curl_cpp_simple     - 기본 GET 요청 테스트"
fi
if [[ -f "advanced_curl_cpp" ]]; then
    echo "  ./advanced_curl_cpp   - 고급 HTTP 메서드 테스트"
fi
if [[ -f "ipv4_ipv6_test" ]]; then
    echo "  ./ipv4_ipv6_test     - IPv4 vs IPv6 vs 기본 동작 테스트"
fi
echo ""
print_info "빌드 스크립트 사용법: ./build.sh --help" 