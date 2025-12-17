/**
 * aes_cbc.c - AES-CBC 암호화/복호화 데모
 * 
 * AES-256-CBC 모드로 데이터를 암호화하고 복호화한다.
 * 
 * 빌드: make
 * 실행: ./bin/aes_cbc
 */

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define AES_BLOCK_SIZE 16
#define AES_KEY_SIZE 32   // 256 bits
#define AES_IV_SIZE 16

/**
 * PKCS#7 패딩을 적용한다.
 */
size_t add_pkcs7_padding(unsigned char *data, size_t len, size_t block_size) {
    size_t padding = block_size - (len % block_size);
    for (size_t i = 0; i < padding; i++) {
        data[len + i] = (unsigned char)padding;
    }
    return len + padding;
}

/**
 * PKCS#7 패딩을 제거한다.
 */
size_t remove_pkcs7_padding(unsigned char *data, size_t len) {
    if (len == 0) return 0;
    unsigned char padding = data[len - 1];
    if (padding > AES_BLOCK_SIZE || padding == 0) return len;
    return len - padding;
}

/**
 * AES-256-CBC로 암호화한다.
 */
int aes_cbc_encrypt(const unsigned char *key,
                    const unsigned char *iv,
                    const unsigned char *plaintext, size_t plaintext_len,
                    unsigned char *ciphertext, size_t *ciphertext_len) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    
    int len;
    int total_len = 0;
    
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, (int)plaintext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    total_len = len;
    
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    total_len += len;
    
    *ciphertext_len = (size_t)total_len;
    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

/**
 * AES-256-CBC로 복호화한다.
 */
int aes_cbc_decrypt(const unsigned char *key,
                    const unsigned char *iv,
                    const unsigned char *ciphertext, size_t ciphertext_len,
                    unsigned char *plaintext, size_t *plaintext_len) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    
    int len;
    int total_len = 0;
    
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, (int)ciphertext_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    total_len = len;
    
    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    total_len += len;
    
    *plaintext_len = (size_t)total_len;
    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

void print_hex(const char *label, const unsigned char *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main(void) {
    printf("=== AES-256-CBC 암호화/복호화 데모 ===\n\n");
    
    // 키와 IV 생성 (실제로는 안전하게 생성/저장해야 함)
    unsigned char key[AES_KEY_SIZE];
    unsigned char iv[AES_IV_SIZE];
    
    // 랜덤 키 생성
    RAND_bytes(key, AES_KEY_SIZE);
    RAND_bytes(iv, AES_IV_SIZE);
    
    printf("키 길이: %d 비트 (AES-256)\n", AES_KEY_SIZE * 8);
    print_hex("키", key, AES_KEY_SIZE);
    print_hex("IV", iv, AES_IV_SIZE);
    printf("\n");
    
    // 평문
    const char *message = "차량 사이버보안 - AES 암호화 테스트 메시지입니다.";
    size_t message_len = strlen(message);
    
    printf("평문: \"%s\"\n", message);
    printf("길이: %zu 바이트\n\n", message_len);
    
    // 암호화
    unsigned char ciphertext[256];
    size_t ciphertext_len;
    
    if (aes_cbc_encrypt(key, iv, (unsigned char *)message, message_len,
                        ciphertext, &ciphertext_len) != 0) {
        printf("암호화 실패\n");
        return 1;
    }
    
    print_hex("암호문", ciphertext, ciphertext_len);
    printf("암호문 길이: %zu 바이트\n\n", ciphertext_len);
    
    // 복호화
    unsigned char decrypted[256];
    size_t decrypted_len;
    
    if (aes_cbc_decrypt(key, iv, ciphertext, ciphertext_len,
                        decrypted, &decrypted_len) != 0) {
        printf("복호화 실패\n");
        return 1;
    }
    
    decrypted[decrypted_len] = '\0';
    printf("복호문: \"%s\"\n", decrypted);
    printf("길이: %zu 바이트\n\n", decrypted_len);
    
    // 원본과 비교
    if (memcmp(message, decrypted, message_len) == 0) {
        printf("✓ 복호화 성공: 원본과 일치\n\n");
    } else {
        printf("✗ 복호화 실패: 원본과 불일치\n\n");
    }
    
    // IV 변경 테스트
    printf("=== IV 변경 테스트 ===\n\n");
    
    unsigned char iv2[AES_IV_SIZE];
    RAND_bytes(iv2, AES_IV_SIZE);
    
    print_hex("원본 IV", iv, AES_IV_SIZE);
    print_hex("새 IV", iv2, AES_IV_SIZE);
    
    unsigned char ciphertext2[256];
    size_t ciphertext2_len;
    
    aes_cbc_encrypt(key, iv2, (unsigned char *)message, message_len,
                    ciphertext2, &ciphertext2_len);
    
    printf("\n동일 평문, 다른 IV:\n");
    print_hex("암호문1 (IV1)", ciphertext, ciphertext_len);
    print_hex("암호문2 (IV2)", ciphertext2, ciphertext2_len);
    printf("\n→ 동일 평문이라도 다른 IV 사용 시 다른 암호문 생성\n");
    printf("→ 이것이 IV의 역할: 암호문 패턴 숨김\n");
    
    return 0;
}
