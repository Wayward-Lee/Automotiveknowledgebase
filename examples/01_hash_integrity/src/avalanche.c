/**
 * avalanche.c - 눈사태 효과(Avalanche Effect) 시연
 * 
 * 입력의 1비트 변화가 해시 출력에 미치는 영향을 분석한다.
 * 
 * 빌드: make
 * 실행: ./bin/avalanche
 */

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>

/**
 * SHA-256 해시를 계산한다.
 */
int sha256(const unsigned char *data, size_t len, unsigned char *hash) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == NULL) return -1;
    
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, data, len);
    
    unsigned int hash_len;
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);
    
    return 0;
}

/**
 * 두 해시의 비트 차이 수를 계산한다.
 */
int count_bit_differences(const unsigned char *hash1, 
                          const unsigned char *hash2, 
                          size_t len) {
    int diff_count = 0;
    
    for (size_t i = 0; i < len; i++) {
        unsigned char xor_val = hash1[i] ^ hash2[i];
        // 해밍 가중치 계산 (1인 비트 수)
        while (xor_val) {
            diff_count += xor_val & 1;
            xor_val >>= 1;
        }
    }
    
    return diff_count;
}

/**
 * 해시를 16진수로 출력한다.
 */
void print_hash(const unsigned char *hash, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", hash[i]);
    }
}

/**
 * 바이트 배열을 비트로 출력한다 (처음 n바이트만).
 */
void print_bits(const unsigned char *data, size_t bytes) {
    for (size_t i = 0; i < bytes; i++) {
        for (int j = 7; j >= 0; j--) {
            printf("%d", (data[i] >> j) & 1);
        }
        printf(" ");
    }
}

int main(void) {
    printf("=== SHA-256 눈사태 효과(Avalanche Effect) 시연 ===\n\n");
    
    // 원본 데이터
    unsigned char original[] = "Avalanche Effect Demo";
    size_t data_len = strlen((char *)original);
    
    unsigned char hash_original[32];
    unsigned char hash_modified[32];
    
    // 원본 해시 계산
    sha256(original, data_len, hash_original);
    
    printf("원본 데이터: \"%s\"\n", original);
    printf("원본 해시:   ");
    print_hash(hash_original, 32);
    printf("\n\n");
    
    printf("=== 1비트씩 변경하며 해시 비교 ===\n\n");
    
    // 첫 번째 바이트의 각 비트를 변경
    for (int bit = 0; bit < 8; bit++) {
        // 데이터 복사 및 1비트 변경
        unsigned char modified[64];
        memcpy(modified, original, data_len);
        modified[0] ^= (1 << bit);  // bit 위치의 비트 반전
        
        // 변경된 데이터 해시 계산
        sha256(modified, data_len, hash_modified);
        
        // 비트 차이 계산
        int bit_diff = count_bit_differences(hash_original, hash_modified, 32);
        double percent = (bit_diff / 256.0) * 100.0;
        
        printf("비트 %d 반전 ('%c'→'%c'):\n", 
               bit, original[0], modified[0]);
        printf("  변경 해시: ");
        print_hash(hash_modified, 32);
        printf("\n");
        printf("  비트 차이: %d / 256 (%.1f%%)\n\n", bit_diff, percent);
    }
    
    // 통계 요약
    printf("=== 통계 요약 ===\n\n");
    
    int total_tests = 100;
    int total_diff = 0;
    
    for (int test = 0; test < total_tests; test++) {
        unsigned char test_data[32];
        unsigned char test_modified[32];
        
        // 랜덤 데이터 시뮬레이션 (간단히 인덱스 기반)
        for (int i = 0; i < 32; i++) {
            test_data[i] = (unsigned char)(test * 17 + i * 31);
        }
        
        sha256(test_data, 32, hash_original);
        
        // 1비트 변경
        memcpy(test_modified, test_data, 32);
        test_modified[0] ^= 1;
        
        sha256(test_modified, 32, hash_modified);
        
        total_diff += count_bit_differences(hash_original, hash_modified, 32);
    }
    
    double avg_diff = (double)total_diff / total_tests;
    double avg_percent = (avg_diff / 256.0) * 100.0;
    
    printf("%d개 테스트 결과:\n", total_tests);
    printf("  평균 비트 차이: %.1f / 256 (%.1f%%)\n", avg_diff, avg_percent);
    printf("  이상적 값:      128 / 256 (50%%)\n\n");
    
    printf("→ 1비트 입력 변화가 출력의 약 50%%를 변화시킴\n");
    printf("→ 이것이 '눈사태 효과'이며, 좋은 해시 함수의 특성이다.\n");
    
    return 0;
}
