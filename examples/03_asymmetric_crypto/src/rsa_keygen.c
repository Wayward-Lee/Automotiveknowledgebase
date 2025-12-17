/**
 * rsa_keygen.c - RSA 키 쌍 생성
 */

#include <stdio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

int main(void) {
    printf("=== RSA-2048 키 쌍 생성 ===\n\n");
    
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    EVP_PKEY *pkey = NULL;
    
    EVP_PKEY_keygen_init(ctx);
    EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048);
    EVP_PKEY_keygen(ctx, &pkey);
    
    // 개인키 저장
    FILE *priv_file = fopen("private_key.pem", "w");
    PEM_write_PrivateKey(priv_file, pkey, NULL, NULL, 0, NULL, NULL);
    fclose(priv_file);
    printf("개인키 저장: private_key.pem\n");
    
    // 공개키 저장
    FILE *pub_file = fopen("public_key.pem", "w");
    PEM_write_PUBKEY(pub_file, pkey);
    fclose(pub_file);
    printf("공개키 저장: public_key.pem\n\n");
    
    printf("키 정보:\n");
    printf("  알고리즘: RSA\n");
    printf("  키 길이: 2048 비트\n");
    printf("  공개 지수 e: 65537 (기본값)\n\n");
    
    printf("사용법:\n");
    printf("  공개키: 누구에게나 배포 가능 (암호화/서명 검증용)\n");
    printf("  개인키: 안전하게 보관 (복호화/서명 생성용)\n");
    
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    
    return 0;
}
