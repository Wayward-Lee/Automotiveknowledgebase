/**
 * secoc_sim.c - SecOC PDU 시뮬레이션
 */

#include <stdio.h>
#include <string.h>
#include <openssl/cmac.h>
#include <openssl/evp.h>

typedef struct {
    uint8_t data[8];        // CAN 페이로드
    uint8_t data_len;
    uint32_t freshness;     // Freshness counter
    uint8_t mac[4];         // Truncated MAC
} SecOC_PDU;

static unsigned char secoc_key[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

void calculate_secoc_mac(SecOC_PDU *pdu) {
    // Data || Freshness를 인증
    unsigned char auth_data[16];
    memcpy(auth_data, pdu->data, pdu->data_len);
    memcpy(auth_data + pdu->data_len, &pdu->freshness, 4);
    
    unsigned char full_mac[16];
    size_t mac_len;
    
    CMAC_CTX *ctx = CMAC_CTX_new();
    CMAC_Init(ctx, secoc_key, 16, EVP_aes_128_cbc(), NULL);
    CMAC_Update(ctx, auth_data, pdu->data_len + 4);
    CMAC_Final(ctx, full_mac, &mac_len);
    CMAC_CTX_free(ctx);
    
    memcpy(pdu->mac, full_mac, 4);  // Truncate to 4 bytes
}

int verify_secoc_mac(SecOC_PDU *pdu) {
    unsigned char auth_data[16];
    memcpy(auth_data, pdu->data, pdu->data_len);
    memcpy(auth_data + pdu->data_len, &pdu->freshness, 4);
    
    unsigned char expected_mac[16];
    size_t mac_len;
    
    CMAC_CTX *ctx = CMAC_CTX_new();
    CMAC_Init(ctx, secoc_key, 16, EVP_aes_128_cbc(), NULL);
    CMAC_Update(ctx, auth_data, pdu->data_len + 4);
    CMAC_Final(ctx, expected_mac, &mac_len);
    CMAC_CTX_free(ctx);
    
    return (memcmp(pdu->mac, expected_mac, 4) == 0);
}

void print_pdu(const char *label, SecOC_PDU *pdu) {
    printf("%s:\n", label);
    printf("  Data: ");
    for (int i = 0; i < pdu->data_len; i++) printf("%02X ", pdu->data[i]);
    printf("\n  Freshness: %u\n", pdu->freshness);
    printf("  MAC: %02X%02X%02X%02X\n", pdu->mac[0], pdu->mac[1], pdu->mac[2], pdu->mac[3]);
}

int main(void) {
    printf("=== SecOC PDU 시뮬레이션 ===\n\n");
    
    static uint32_t tx_counter = 1000;  // 송신 카운터
    static uint32_t rx_counter = 1000;  // 수신 카운터
    
    // 송신 ECU: 메시지 생성
    printf("=== 송신 ECU ===\n");
    SecOC_PDU tx_pdu = {
        .data = {0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .data_len = 2,
        .freshness = tx_counter++,
        .mac = {0}
    };
    
    calculate_secoc_mac(&tx_pdu);
    print_pdu("송신 PDU", &tx_pdu);
    printf("\n");
    
    // 수신 ECU: 검증
    printf("=== 수신 ECU (정상) ===\n");
    SecOC_PDU rx_pdu = tx_pdu;  // 정상 수신
    
    if (rx_pdu.freshness == rx_counter && verify_secoc_mac(&rx_pdu)) {
        printf("✓ MAC 검증 성공\n");
        printf("✓ Freshness 검증 성공\n");
        rx_counter++;
        printf("→ 메시지 수락\n\n");
    }
    
    // 재전송 공격 시뮬레이션
    printf("=== 재전송 공격 탐지 ===\n");
    printf("공격자가 이전 메시지를 재전송...\n");
    
    SecOC_PDU replay_pdu = tx_pdu;  // 같은 메시지 재전송
    
    if (replay_pdu.freshness < rx_counter) {
        printf("✗ Freshness 검증 실패 (예상: %u, 수신: %u)\n", 
               rx_counter, replay_pdu.freshness);
        printf("→ 재전송 공격 탐지! 메시지 거부\n\n");
    }
    
    // 메시지 변조 탐지
    printf("=== 메시지 변조 탐지 ===\n");
    SecOC_PDU tampered_pdu = {
        .data = {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 변조
        .data_len = 2,
        .freshness = rx_counter,
        .mac = {tx_pdu.mac[0], tx_pdu.mac[1], tx_pdu.mac[2], tx_pdu.mac[3]}  // 원본 MAC
    };
    
    if (!verify_secoc_mac(&tampered_pdu)) {
        printf("✗ MAC 검증 실패\n");
        printf("→ 변조 탐지! 메시지 거부\n");
    }
    
    return 0;
}
