/**
 * file_integrity.c - 파일 무결성 검증 도구
 * 
 * 파일의 SHA-256 해시를 계산하여 무결성을 검증한다.
 * 
 * 빌드: make
 * 실행: ./bin/file_integrity <파일경로>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

#define BUFFER_SIZE 4096

/**
 * 파일의 SHA-256 해시를 계산한다.
 * 
 * @param filepath 파일 경로
 * @param hash 해시 결과를 저장할 버퍼 (최소 32바이트)
 * @return 성공 시 0, 실패 시 -1
 */
int calculate_file_hash(const char *filepath, unsigned char *hash) {
    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        perror("파일 열기 실패");
        return -1;
    }
    
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        fclose(file);
        return -1;
    }
    
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(ctx);
        fclose(file);
        return -1;
    }
    
    unsigned char buffer[BUFFER_SIZE];
    size_t bytes_read;
    
    // 파일을 청크 단위로 읽으며 해시 업데이트
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (EVP_DigestUpdate(ctx, buffer, bytes_read) != 1) {
            EVP_MD_CTX_free(ctx);
            fclose(file);
            return -1;
        }
    }
    
    unsigned int hash_len = 0;
    if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(ctx);
        fclose(file);
        return -1;
    }
    
    EVP_MD_CTX_free(ctx);
    fclose(file);
    return 0;
}

/**
 * 해시를 16진수 문자열로 변환한다.
 */
void hash_to_hex(const unsigned char *hash, size_t hash_len, char *hex_str) {
    for (size_t i = 0; i < hash_len; i++) {
        sprintf(hex_str + (i * 2), "%02x", hash[i]);
    }
    hex_str[hash_len * 2] = '\0';
}

/**
 * 두 해시를 비교한다.
 * 
 * @return 동일하면 0, 다르면 -1
 */
int compare_hashes(const unsigned char *hash1, const unsigned char *hash2, size_t len) {
    // 상수 시간 비교 (타이밍 공격 방지)
    int result = 0;
    for (size_t i = 0; i < len; i++) {
        result |= hash1[i] ^ hash2[i];
    }
    return (result == 0) ? 0 : -1;
}

void print_usage(const char *program_name) {
    printf("사용법:\n");
    printf("  %s <파일>              - 파일의 SHA-256 해시 계산\n", program_name);
    printf("  %s -c <파일> <해시>    - 파일 해시를 기대값과 비교\n", program_name);
    printf("\n예시:\n");
    printf("  %s myfile.bin\n", program_name);
    printf("  %s -c myfile.bin a1b2c3d4...\n", program_name);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    unsigned char hash[32];
    char hex_hash[65];
    
    if (strcmp(argv[1], "-c") == 0) {
        // 검증 모드
        if (argc != 4) {
            print_usage(argv[0]);
            return 1;
        }
        
        const char *filepath = argv[2];
        const char *expected_hash = argv[3];
        
        printf("파일: %s\n", filepath);
        printf("기대 해시: %s\n", expected_hash);
        
        if (calculate_file_hash(filepath, hash) != 0) {
            return 1;
        }
        
        hash_to_hex(hash, 32, hex_hash);
        printf("계산 해시: %s\n", hex_hash);
        
        if (strcasecmp(hex_hash, expected_hash) == 0) {
            printf("\n✓ 무결성 검증 성공: 해시가 일치합니다.\n");
            return 0;
        } else {
            printf("\n✗ 무결성 검증 실패: 해시가 불일치합니다!\n");
            printf("  파일이 변조되었을 수 있습니다.\n");
            return 1;
        }
    } else {
        // 해시 계산 모드
        const char *filepath = argv[1];
        
        printf("파일: %s\n", filepath);
        
        if (calculate_file_hash(filepath, hash) != 0) {
            return 1;
        }
        
        hash_to_hex(hash, 32, hex_hash);
        printf("SHA-256: %s\n", hex_hash);
        
        return 0;
    }
}
