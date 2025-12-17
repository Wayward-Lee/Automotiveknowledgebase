/**
 * rsa_sign.c - RSA-SHA256 디지털 서명
 */

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

int main(void) {
    printf("=== RSA-SHA256 디지털 서명 데모 ===\n\n");
    
    // 키 생성
    EVP_PKEY_CTX *keygen_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_keygen_init(keygen_ctx);
    EVP_PKEY_CTX_set_rsa_keygen_bits(keygen_ctx, 2048);
    EVP_PKEY_keygen(keygen_ctx, &pkey);
    EVP_PKEY_CTX_free(keygen_ctx);
    
    const char *document = "OTA Update v2.5.1 - ECU Firmware";
    size_t doc_len = strlen(document);
    
    printf("문서: \"%s\"\n\n", document);
    
    // 서명 생성
    EVP_MD_CTX *sign_ctx = EVP_MD_CTX_new();
    EVP_DigestSignInit(sign_ctx, NULL, EVP_sha256(), NULL, pkey);
    EVP_DigestSignUpdate(sign_ctx, document, doc_len);
    
    size_t sig_len;
    EVP_DigestSignFinal(sign_ctx, NULL, &sig_len);
    unsigned char signature[512];
    EVP_DigestSignFinal(sign_ctx, signature, &sig_len);
    EVP_MD_CTX_free(sign_ctx);
    
    printf("서명 (%zu 바이트): ", sig_len);
    for (size_t i = 0; i < 32; i++) printf("%02x", signature[i]);
    printf("...\n\n");
    
    // 서명 검증
    EVP_MD_CTX *verify_ctx = EVP_MD_CTX_new();
    EVP_DigestVerifyInit(verify_ctx, NULL, EVP_sha256(), NULL, pkey);
    EVP_DigestVerifyUpdate(verify_ctx, document, doc_len);
    
    int result = EVP_DigestVerifyFinal(verify_ctx, signature, sig_len);
    EVP_MD_CTX_free(verify_ctx);
    
    if (result == 1) {
        printf("✓ 서명 검증 성공: 문서가 변조되지 않았음\n\n");
    } else {
        printf("✗ 서명 검증 실패\n\n");
    }
    
    // 문서 변조 테스트
    printf("=== 문서 변조 테스트 ===\n");
    const char *tampered = "OTA Update v9.9.9 - ECU Firmware";
    
    EVP_MD_CTX *verify_ctx2 = EVP_MD_CTX_new();
    EVP_DigestVerifyInit(verify_ctx2, NULL, EVP_sha256(), NULL, pkey);
    EVP_DigestVerifyUpdate(verify_ctx2, tampered, strlen(tampered));
    result = EVP_DigestVerifyFinal(verify_ctx2, signature, sig_len);
    EVP_MD_CTX_free(verify_ctx2);
    
    printf("변조 문서: \"%s\"\n", tampered);
    if (result != 1) {
        printf("✗ 검증 실패: 변조 탐지!\n");
    }
    
    EVP_PKEY_free(pkey);
    return 0;
}
