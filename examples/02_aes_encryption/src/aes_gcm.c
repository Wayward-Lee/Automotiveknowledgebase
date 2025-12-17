/**
 * aes_gcm.c - AES-GCM 인증 암호화 데모
 * 
 * AES-256-GCM (AEAD)으로 기밀성과 무결성을 동시에 보장한다.
 * 
 * 빌드: make
 * 실행: ./bin/aes_gcm
 */

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define AES_KEY_SIZE 32   // 256 bits
#define GCM_IV_SIZE 12    // 96 bits (GCM 권장)
#define GCM_TAG_SIZE 16   // 128 bits

/**
 * AES-256-GCM으로 인증 암호화를 수행한다.
 * 
 * @param key 256비트 키
 * @param iv 96비트 IV
 * @param aad 추가 인증 데이터 (암호화되지 않지만 인증됨)
 * @param aad_len AAD 길이
 * @param plaintext 평문
 * @param plaintext_len 평문 길이
 * @param ciphertext 암호문 출력
 * @param tag 인증 태그 출력 (16바이트)
 */
int aes_gcm_encrypt(const unsigned char *key,
                    const unsigned char *iv,
                    const unsigned char *aad, size_t aad_len,
                    const unsigned char *plaintext, size_t plaintext_len,
                    unsigned char *ciphertext,
                    unsigned char *tag) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    int len;
    int ciphertext_len = 0;
    
    // 초기화
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1)
        goto error;
    
    // IV 길이 설정
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, GCM_IV_SIZE, NULL) != 1)
        goto error;
    
    // 키와 IV 설정
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1)
        goto error;
    
    // AAD 처리 (암호화되지 않지만 태그에 포함)
    if (aad && aad_len > 0) {
        if (EVP_EncryptUpdate(ctx, NULL, &len, aad, (int)aad_len) != 1)
            goto error;
    }
    
    // 평문 암호화
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, (int)plaintext_len) != 1)
        goto error;
    ciphertext_len = len;
    
    // 암호화 완료
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1)
        goto error;
    ciphertext_len += len;
    
    // 인증 태그 추출
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_SIZE, tag) != 1)
        goto error;
    
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
    
error:
    EVP_CIPHER_CTX_free(ctx);
    return -1;
}

/**
 * AES-256-GCM으로 인증 복호화를 수행한다.
 */
int aes_gcm_decrypt(const unsigned char *key,
                    const unsigned char *iv,
                    const unsigned char *aad, size_t aad_len,
                    const unsigned char *ciphertext, size_t ciphertext_len,
                    const unsigned char *tag,
                    unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    int len;
    int plaintext_len = 0;
    
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1)
        goto error;
    
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, GCM_IV_SIZE, NULL) != 1)
        goto error;
    
    if (EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv) != 1)
        goto error;
    
    if (aad && aad_len > 0) {
        if (EVP_DecryptUpdate(ctx, NULL, &len, aad, (int)aad_len) != 1)
            goto error;
    }
    
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, (int)ciphertext_len) != 1)
        goto error;
    plaintext_len = len;
    
    // 태그 설정
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, GCM_TAG_SIZE, 
                           (void *)tag) != 1)
        goto error;
    
    // 복호화 완료 및 태그 검증
    int ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
    
    EVP_CIPHER_CTX_free(ctx);
    
    if (ret > 0) {
        return plaintext_len + len;  // 성공
    } else {
        return -2;  // 인증 실패
    }
    
error:
    EVP_CIPHER_CTX_free(ctx);
    return -1;
}

void print_hex(const char *label, const unsigned char *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main(void) {
    printf("=== AES-256-GCM 인증 암호화(AEAD) 데모 ===\n\n");
    
    // 키와 IV 생성
    unsigned char key[AES_KEY_SIZE];
    unsigned char iv[GCM_IV_SIZE];
    
    RAND_bytes(key, AES_KEY_SIZE);
    RAND_bytes(iv, GCM_IV_SIZE);
    
    printf("키 길이: %d 비트\n", AES_KEY_SIZE * 8);
    printf("IV 길이: %d 비트 (GCM 권장)\n", GCM_IV_SIZE * 8);
    print_hex("키", key, AES_KEY_SIZE);
    print_hex("IV", iv, GCM_IV_SIZE);
    printf("\n");
    
    // 평문과 AAD
    const char *plaintext = "OTA Update Package: Version 2.5.1";
    size_t plaintext_len = strlen(plaintext);
    
    const char *aad = "VehicleID:ABC123|Timestamp:1702800000";  // 헤더 정보
    size_t aad_len = strlen(aad);
    
    printf("평문: \"%s\" (%zu 바이트)\n", plaintext, plaintext_len);
    printf("AAD:  \"%s\" (%zu 바이트)\n\n", aad, aad_len);
    printf("※ AAD는 암호화되지 않지만, 인증 태그에 포함됨\n\n");
    
    // 암호화
    unsigned char ciphertext[256];
    unsigned char tag[GCM_TAG_SIZE];
    
    int ct_len = aes_gcm_encrypt(key, iv, 
                                 (unsigned char *)aad, aad_len,
                                 (unsigned char *)plaintext, plaintext_len,
                                 ciphertext, tag);
    
    if (ct_len < 0) {
        printf("암호화 실패\n");
        return 1;
    }
    
    print_hex("암호문", ciphertext, (size_t)ct_len);
    print_hex("인증 태그", tag, GCM_TAG_SIZE);
    printf("\n");
    
    // 정상 복호화
    printf("=== 정상 복호화 ===\n");
    unsigned char decrypted[256];
    
    int pt_len = aes_gcm_decrypt(key, iv,
                                 (unsigned char *)aad, aad_len,
                                 ciphertext, (size_t)ct_len,
                                 tag, decrypted);
    
    if (pt_len >= 0) {
        decrypted[pt_len] = '\0';
        printf("복호문: \"%s\"\n", decrypted);
        printf("✓ 인증 성공: 데이터 무결성 확인\n\n");
    } else {
        printf("✗ 복호화/인증 실패\n\n");
    }
    
    // 암호문 변조 테스트
    printf("=== 암호문 변조 테스트 ===\n");
    unsigned char tampered_ct[256];
    memcpy(tampered_ct, ciphertext, (size_t)ct_len);
    tampered_ct[0] ^= 0x01;  // 1비트 변조
    
    printf("암호문 첫 바이트를 1비트 변조 후 복호화 시도...\n");
    
    pt_len = aes_gcm_decrypt(key, iv,
                             (unsigned char *)aad, aad_len,
                             tampered_ct, (size_t)ct_len,
                             tag, decrypted);
    
    if (pt_len == -2) {
        printf("✗ 인증 실패: 데이터 변조 탐지!\n");
        printf("  → GCM 모드는 암호문 변조를 감지함\n\n");
    } else if (pt_len >= 0) {
        printf("✓ 복호화 성공 (예상치 못함)\n\n");
    }
    
    // AAD 변조 테스트
    printf("=== AAD 변조 테스트 ===\n");
    const char *tampered_aad = "VehicleID:XYZ999|Timestamp:1702800000";
    
    printf("원본 AAD: \"%s\"\n", aad);
    printf("변조 AAD: \"%s\"\n", tampered_aad);
    printf("변조된 AAD로 복호화 시도...\n");
    
    pt_len = aes_gcm_decrypt(key, iv,
                             (unsigned char *)tampered_aad, strlen(tampered_aad),
                             ciphertext, (size_t)ct_len,
                             tag, decrypted);
    
    if (pt_len == -2) {
        printf("✗ 인증 실패: AAD 변조 탐지!\n");
        printf("  → 헤더 정보가 변조되어도 감지됨\n\n");
    }
    
    printf("=== 요약 ===\n");
    printf("AES-GCM (AEAD)의 장점:\n");
    printf("  1. 기밀성: 암호화로 데이터 보호\n");
    printf("  2. 무결성: 태그로 변조 탐지\n");
    printf("  3. 진정성: AAD로 메타데이터 인증\n");
    printf("  → OTA 업데이트, TLS 1.3에서 사용\n");
    
    return 0;
}
