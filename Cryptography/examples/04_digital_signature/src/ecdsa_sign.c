/**
 * ecdsa_sign.c - ECDSA P-256 디지털 서명
 */

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/ec.h>

int main(void) {
    printf("=== ECDSA P-256 디지털 서명 데모 ===\n\n");
    
    // EC 키 생성
    EVP_PKEY_CTX *keygen_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_keygen_init(keygen_ctx);
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid(keygen_ctx, NID_X9_62_prime256v1);
    EVP_PKEY_keygen(keygen_ctx, &pkey);
    EVP_PKEY_CTX_free(keygen_ctx);
    
    printf("곡선: P-256 (secp256r1)\n");
    printf("키 길이: 256 비트 (RSA 2048과 동등한 보안)\n\n");
    
    const char *firmware = "ECU_FIRMWARE_IMAGE_HASH_1234567890ABCDEF";
    size_t fw_len = strlen(firmware);
    
    printf("서명 대상: \"%s\"\n\n", firmware);
    
    // ECDSA 서명
    EVP_MD_CTX *sign_ctx = EVP_MD_CTX_new();
    EVP_DigestSignInit(sign_ctx, NULL, EVP_sha256(), NULL, pkey);
    EVP_DigestSignUpdate(sign_ctx, firmware, fw_len);
    
    size_t sig_len;
    EVP_DigestSignFinal(sign_ctx, NULL, &sig_len);
    unsigned char signature[128];
    EVP_DigestSignFinal(sign_ctx, signature, &sig_len);
    EVP_MD_CTX_free(sign_ctx);
    
    printf("ECDSA 서명 (%zu 바이트): ", sig_len);
    for (size_t i = 0; i < sig_len && i < 32; i++) printf("%02x", signature[i]);
    printf("...\n");
    printf("→ RSA 서명(256바이트)보다 훨씬 짧음\n\n");
    
    // 검증
    EVP_MD_CTX *verify_ctx = EVP_MD_CTX_new();
    EVP_DigestVerifyInit(verify_ctx, NULL, EVP_sha256(), NULL, pkey);
    EVP_DigestVerifyUpdate(verify_ctx, firmware, fw_len);
    
    if (EVP_DigestVerifyFinal(verify_ctx, signature, sig_len) == 1) {
        printf("✓ ECDSA 서명 검증 성공\n\n");
    }
    EVP_MD_CTX_free(verify_ctx);
    
    printf("=== RSA vs ECDSA 비교 ===\n");
    printf("          RSA-2048    ECDSA P-256\n");
    printf("보안강도   112비트     128비트\n");
    printf("공개키    256바이트    64바이트\n");
    printf("서명크기  256바이트    ~64바이트\n");
    printf("속도      느림         빠름\n");
    printf("\n→ 차량 ECU에서는 ECDSA 권장\n");
    
    EVP_PKEY_free(pkey);
    return 0;
}
