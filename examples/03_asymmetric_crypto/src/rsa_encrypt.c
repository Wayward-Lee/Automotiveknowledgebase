/**
 * rsa_encrypt.c - RSA 암호화/복호화 및 하이브리드 암호화
 */

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

int main(void) {
    printf("=== RSA 암호화/복호화 데모 ===\n\n");
    
    // 키 생성
    EVP_PKEY_CTX *keygen_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_keygen_init(keygen_ctx);
    EVP_PKEY_CTX_set_rsa_keygen_bits(keygen_ctx, 2048);
    EVP_PKEY_keygen(keygen_ctx, &pkey);
    EVP_PKEY_CTX_free(keygen_ctx);
    
    printf("RSA-2048 키 쌍 생성 완료\n\n");
    
    // 평문 (RSA로 직접 암호화할 수 있는 크기 제한 있음)
    const char *plaintext = "Secret AES Key!";  // 16바이트
    size_t pt_len = strlen(plaintext);
    
    printf("평문: \"%s\" (%zu 바이트)\n\n", plaintext, pt_len);
    
    // RSA 암호화
    EVP_PKEY_CTX *enc_ctx = EVP_PKEY_CTX_new(pkey, NULL);
    EVP_PKEY_encrypt_init(enc_ctx);
    EVP_PKEY_CTX_set_rsa_padding(enc_ctx, RSA_PKCS1_OAEP_PADDING);
    
    size_t ct_len;
    EVP_PKEY_encrypt(enc_ctx, NULL, &ct_len, 
                     (unsigned char *)plaintext, pt_len);
    
    unsigned char ciphertext[256];
    EVP_PKEY_encrypt(enc_ctx, ciphertext, &ct_len,
                     (unsigned char *)plaintext, pt_len);
    EVP_PKEY_CTX_free(enc_ctx);
    
    printf("암호문 (%zu 바이트): ", ct_len);
    for (size_t i = 0; i < 32; i++) printf("%02x", ciphertext[i]);
    printf("...\n\n");
    
    // RSA 복호화
    EVP_PKEY_CTX *dec_ctx = EVP_PKEY_CTX_new(pkey, NULL);
    EVP_PKEY_decrypt_init(dec_ctx);
    EVP_PKEY_CTX_set_rsa_padding(dec_ctx, RSA_PKCS1_OAEP_PADDING);
    
    unsigned char decrypted[256];
    size_t dec_len = sizeof(decrypted);
    EVP_PKEY_decrypt(dec_ctx, decrypted, &dec_len, ciphertext, ct_len);
    EVP_PKEY_CTX_free(dec_ctx);
    
    decrypted[dec_len] = '\0';
    printf("복호문: \"%s\"\n", decrypted);
    
    if (memcmp(plaintext, decrypted, pt_len) == 0) {
        printf("✓ 복호화 성공\n\n");
    }
    
    printf("=== 하이브리드 암호화 원리 ===\n");
    printf("1. 랜덤 AES 키 생성\n");
    printf("2. RSA로 AES 키 암호화 (위 데모)\n");
    printf("3. AES로 실제 데이터 암호화\n");
    printf("→ RSA의 키 교환 + AES의 속도 결합\n");
    
    EVP_PKEY_free(pkey);
    return 0;
}
