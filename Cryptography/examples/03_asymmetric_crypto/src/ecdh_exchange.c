/**
 * ecdh_exchange.c - ECDH(Elliptic Curve Diffie-Hellman) 키 교환
 */

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/ec.h>

void print_hex(const char *label, const unsigned char *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len && i < 32; i++) printf("%02x", data[i]);
    if (len > 32) printf("...");
    printf("\n");
}

int main(void) {
    printf("=== ECDH 키 교환 데모 ===\n\n");
    printf("두 당사자가 공개 채널에서 비밀 키를 합의한다.\n\n");
    
    // Alice 키 쌍 생성
    EVP_PKEY_CTX *alice_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    EVP_PKEY_keygen_init(alice_ctx);
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid(alice_ctx, NID_X9_62_prime256v1);
    EVP_PKEY *alice_key = NULL;
    EVP_PKEY_keygen(alice_ctx, &alice_key);
    EVP_PKEY_CTX_free(alice_ctx);
    
    // Bob 키 쌍 생성
    EVP_PKEY_CTX *bob_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    EVP_PKEY_keygen_init(bob_ctx);
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid(bob_ctx, NID_X9_62_prime256v1);
    EVP_PKEY *bob_key = NULL;
    EVP_PKEY_keygen(bob_ctx, &bob_key);
    EVP_PKEY_CTX_free(bob_ctx);
    
    printf("곡선: P-256 (secp256r1)\n");
    printf("Alice: 키 쌍 생성 완료\n");
    printf("Bob:   키 쌍 생성 완료\n\n");
    
    // Alice가 Bob의 공개키로 공유 비밀 계산
    EVP_PKEY_CTX *derive_ctx = EVP_PKEY_CTX_new(alice_key, NULL);
    EVP_PKEY_derive_init(derive_ctx);
    EVP_PKEY_derive_set_peer(derive_ctx, bob_key);
    
    size_t secret_len;
    EVP_PKEY_derive(derive_ctx, NULL, &secret_len);
    unsigned char alice_secret[64];
    EVP_PKEY_derive(derive_ctx, alice_secret, &secret_len);
    EVP_PKEY_CTX_free(derive_ctx);
    
    // Bob이 Alice의 공개키로 공유 비밀 계산
    derive_ctx = EVP_PKEY_CTX_new(bob_key, NULL);
    EVP_PKEY_derive_init(derive_ctx);
    EVP_PKEY_derive_set_peer(derive_ctx, alice_key);
    
    unsigned char bob_secret[64];
    EVP_PKEY_derive(derive_ctx, bob_secret, &secret_len);
    EVP_PKEY_CTX_free(derive_ctx);
    
    print_hex("Alice의 공유 비밀", alice_secret, secret_len);
    print_hex("Bob의 공유 비밀  ", bob_secret, secret_len);
    
    if (memcmp(alice_secret, bob_secret, secret_len) == 0) {
        printf("\n✓ 동일한 공유 비밀 생성!\n");
        printf("  → 이 비밀로 AES 키를 파생하여 통신 암호화\n");
    }
    
    printf("\n=== ECDH의 특성 ===\n");
    printf("• 공개 채널에서도 안전하게 키 합의\n");
    printf("• 도청자는 공유 비밀을 계산 불가\n");
    printf("• TLS 1.3에서 ECDHE로 사용\n");
    
    EVP_PKEY_free(alice_key);
    EVP_PKEY_free(bob_key);
    return 0;
}
