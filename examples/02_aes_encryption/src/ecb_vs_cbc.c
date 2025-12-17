/**
 * ecb_vs_cbc.c - ECB vs CBC 모드 비교
 * 
 * ECB 모드의 패턴 노출 취약점을 시각적으로 보여준다.
 * 
 * 빌드: make
 * 실행: ./bin/ecb_vs_cbc
 */

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define AES_BLOCK_SIZE 16
#define AES_KEY_SIZE 16  // AES-128 사용

/**
 * AES-ECB 암호화
 */
int aes_ecb_encrypt(const unsigned char *key,
                    const unsigned char *plaintext, size_t len,
                    unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL);
    EVP_CIPHER_CTX_set_padding(ctx, 0);  // 패딩 비활성화
    
    int outlen;
    EVP_EncryptUpdate(ctx, ciphertext, &outlen, plaintext, (int)len);
    EVP_EncryptFinal_ex(ctx, ciphertext + outlen, &outlen);
    
    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

/**
 * AES-CBC 암호화
 */
int aes_cbc_encrypt(const unsigned char *key,
                    const unsigned char *iv,
                    const unsigned char *plaintext, size_t len,
                    unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv);
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    
    int outlen;
    EVP_EncryptUpdate(ctx, ciphertext, &outlen, plaintext, (int)len);
    EVP_EncryptFinal_ex(ctx, ciphertext + outlen, &outlen);
    
    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

void print_blocks(const char *label, const unsigned char *data, 
                  size_t len, size_t block_size) {
    printf("%s:\n", label);
    for (size_t i = 0; i < len; i += block_size) {
        printf("  블록 %zu: ", i / block_size + 1);
        for (size_t j = 0; j < block_size && (i + j) < len; j++) {
            printf("%02x", data[i + j]);
        }
        printf("\n");
    }
}

int main(void) {
    printf("=== ECB vs CBC 모드 비교 ===\n\n");
    printf("ECB(Electronic Codebook)의 패턴 노출 문제를 시연한다.\n\n");
    
    // 키 생성
    unsigned char key[AES_KEY_SIZE];
    unsigned char iv[AES_BLOCK_SIZE];
    RAND_bytes(key, AES_KEY_SIZE);
    RAND_bytes(iv, AES_BLOCK_SIZE);
    
    // 반복 패턴이 있는 평문 (4개의 동일 블록)
    unsigned char plaintext[64];
    memset(plaintext, 0, sizeof(plaintext));
    
    // 블록 1, 2, 3, 4에 동일한 패턴
    const char *pattern = "REPEAT_PATTERN!!";  // 정확히 16바이트
    memcpy(plaintext + 0,  pattern, 16);
    memcpy(plaintext + 16, pattern, 16);  // 블록 2 = 블록 1
    memcpy(plaintext + 32, "DIFFERENT_DATA!!", 16);
    memcpy(plaintext + 48, pattern, 16);  // 블록 4 = 블록 1
    
    printf("=== 평문 ===\n");
    print_blocks("평문", plaintext, 64, AES_BLOCK_SIZE);
    printf("\n주목: 블록 1, 2, 4가 동일함 (\"REPEAT_PATTERN!!\")\n\n");
    
    // ECB 암호화
    unsigned char ciphertext_ecb[64];
    aes_ecb_encrypt(key, plaintext, 64, ciphertext_ecb);
    
    printf("=== ECB 모드 암호화 결과 ===\n");
    print_blocks("ECB 암호문", ciphertext_ecb, 64, AES_BLOCK_SIZE);
    
    // ECB에서 동일 블록 확인
    printf("\nECB 분석:\n");
    if (memcmp(ciphertext_ecb + 0, ciphertext_ecb + 16, 16) == 0) {
        printf("  ⚠️  블록 1 == 블록 2 (패턴 노출!)\n");
    }
    if (memcmp(ciphertext_ecb + 0, ciphertext_ecb + 48, 16) == 0) {
        printf("  ⚠️  블록 1 == 블록 4 (패턴 노출!)\n");
    }
    printf("\n→ ECB는 동일 평문 블록을 동일 암호문 블록으로 변환!\n");
    printf("→ 공격자가 패턴을 분석할 수 있음\n\n");
    
    // CBC 암호화
    unsigned char ciphertext_cbc[64];
    aes_cbc_encrypt(key, iv, plaintext, 64, ciphertext_cbc);
    
    printf("=== CBC 모드 암호화 결과 ===\n");
    print_blocks("CBC 암호문", ciphertext_cbc, 64, AES_BLOCK_SIZE);
    
    // CBC에서 동일 블록 확인
    printf("\nCBC 분석:\n");
    if (memcmp(ciphertext_cbc + 0, ciphertext_cbc + 16, 16) != 0) {
        printf("  ✓ 블록 1 ≠ 블록 2 (패턴 숨겨짐)\n");
    }
    if (memcmp(ciphertext_cbc + 0, ciphertext_cbc + 48, 16) != 0) {
        printf("  ✓ 블록 1 ≠ 블록 4 (패턴 숨겨짐)\n");
    }
    printf("\n→ CBC는 체이닝으로 동일 평문도 다른 암호문 생성\n");
    printf("→ 패턴이 숨겨져 분석이 어려움\n\n");
    
    // 시각적 비교
    printf("=== 시각적 비교 (블록 해시 패턴) ===\n\n");
    printf("평문 블록:  [AAAA] [AAAA] [BBBB] [AAAA]\n");
    printf("ECB 결과:   [XXXX] [XXXX] [YYYY] [XXXX]  ← 패턴 보존!\n");
    printf("CBC 결과:   [XXXX] [YYYY] [ZZZZ] [WWWW]  ← 패턴 숨김\n\n");
    
    printf("=== 보안 권장사항 ===\n");
    printf("❌ ECB 모드: 절대 사용 금지\n");
    printf("⚠️  CBC 모드: 가능하지만 무결성 보장 안됨\n");
    printf("✓  GCM 모드: 권장 (기밀성 + 무결성)\n");
    
    return 0;
}
