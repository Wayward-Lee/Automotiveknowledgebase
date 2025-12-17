/**
 * hash_demo.c - SHA-256 해시 계산 데모
 * 
 * 문자열에 대한 SHA-256 해시를 계산하고 출력한다.
 * 
 * 빌드: make
 * 실행: ./bin/hash_demo
 */

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>

/**
 * 데이터의 SHA-256 해시를 계산한다.
 * 
 * @param data 해시할 데이터
 * @param len 데이터 길이
 * @param hash 해시 결과를 저장할 버퍼 (최소 32바이트)
 * @return 성공 시 0, 실패 시 -1
 */
int calculate_sha256(const unsigned char *data, size_t len, unsigned char *hash) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        return -1;
    }
    
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    if (EVP_DigestUpdate(ctx, data, len) != 1) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    unsigned int hash_len = 0;
    if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(ctx);
        return -1;
    }
    
    EVP_MD_CTX_free(ctx);
    return 0;
}

/**
 * 해시 값을 16진수 문자열로 출력한다.
 */
void print_hash(const unsigned char *hash, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

int main(void) {
    printf("=== SHA-256 해시 계산 데모 ===\n\n");
    
    // 테스트 문자열들
    const char *test_strings[] = {
        "Hello, World!",
        "hello, world!",  // 대소문자 변경
        "Hello, World",   // 느낌표 제거
        "차량 사이버보안",
        ""                // 빈 문자열
    };
    
    int num_tests = sizeof(test_strings) / sizeof(test_strings[0]);
    unsigned char hash[32];  // SHA-256은 32바이트 출력
    
    for (int i = 0; i < num_tests; i++) {
        printf("입력: \"%s\"\n", test_strings[i]);
        printf("길이: %zu 바이트\n", strlen(test_strings[i]));
        
        if (calculate_sha256((const unsigned char *)test_strings[i], 
                            strlen(test_strings[i]), hash) == 0) {
            printf("SHA-256: ");
            print_hash(hash, 32);
        } else {
            printf("해시 계산 실패\n");
        }
        printf("\n");
    }
    
    // 동일 입력 반복 테스트 (결정론적 특성)
    printf("=== 결정론적 특성 테스트 ===\n");
    const char *same_input = "Deterministic Test";
    
    printf("동일 입력 \"%s\"를 3번 해시:\n", same_input);
    for (int i = 0; i < 3; i++) {
        calculate_sha256((const unsigned char *)same_input, 
                        strlen(same_input), hash);
        printf("  %d회차: ", i + 1);
        print_hash(hash, 32);
    }
    printf("→ 동일 입력에 대해 항상 동일한 해시 값 출력\n");
    
    return 0;
}
