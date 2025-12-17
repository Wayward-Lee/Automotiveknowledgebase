/**
 * cmac_demo.c - AES-CMAC 메시지 인증 코드
 */

#include <stdio.h>
#include <string.h>
#include <openssl/cmac.h>
#include <openssl/evp.h>

void print_hex(const char *label, const unsigned char *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) printf("%02x", data[i]);
    printf("\n");
}

int calculate_cmac(const unsigned char *key, size_t key_len,
                   const unsigned char *data, size_t data_len,
                   unsigned char *mac, size_t *mac_len) {
    CMAC_CTX *ctx = CMAC_CTX_new();
    
    const EVP_CIPHER *cipher = (key_len == 16) ? EVP_aes_128_cbc() : EVP_aes_256_cbc();
    CMAC_Init(ctx, key, key_len, cipher, NULL);
    CMAC_Update(ctx, data, data_len);
    CMAC_Final(ctx, mac, mac_len);
    CMAC_CTX_free(ctx);
    
    return 0;
}

int main(void) {
    printf("=== AES-CMAC 메시지 인증 코드 데모 ===\n\n");
    
    // 128비트 키
    unsigned char key[16] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };
    
    // CAN 메시지 시뮬레이션
    unsigned char can_msg[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    
    print_hex("SecOC 키", key, 16);
    print_hex("CAN 메시지", can_msg, 8);
    printf("\n");
    
    // CMAC 계산
    unsigned char mac[16];
    size_t mac_len = 0;
    calculate_cmac(key, 16, can_msg, 8, mac, &mac_len);
    
    print_hex("CMAC (128비트)", mac, mac_len);
    printf("\n");
    
    // Truncated MAC (SecOC에서 사용)
    printf("SecOC 적용:\n");
    printf("  전체 CMAC: %zu 비트\n", mac_len * 8);
    printf("  Truncated (4바이트): ");
    for (int i = 0; i < 4; i++) printf("%02x", mac[i]);
    printf("\n");
    printf("  → CAN 대역폭 절약을 위해 MAC 일부만 전송\n\n");
    
    // 검증
    printf("=== MAC 검증 ===\n");
    unsigned char verify_mac[16];
    size_t verify_len;
    calculate_cmac(key, 16, can_msg, 8, verify_mac, &verify_len);
    
    if (memcmp(mac, verify_mac, 4) == 0) {
        printf("✓ MAC 일치: 메시지 인증 성공\n\n");
    }
    
    // 메시지 변조 탐지
    printf("=== 변조 탐지 ===\n");
    can_msg[0] ^= 0x01;  // 1비트 변조
    calculate_cmac(key, 16, can_msg, 8, verify_mac, &verify_len);
    
    if (memcmp(mac, verify_mac, 4) != 0) {
        printf("✗ MAC 불일치: 메시지 변조 탐지!\n");
    }
    
    return 0;
}
