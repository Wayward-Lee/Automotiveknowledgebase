/**
 * hmac_demo.c - HMAC-SHA256 메시지 인증 코드 데모
 * 
 * 비밀 키를 사용한 메시지 인증 코드를 생성하고 검증한다.
 * 
 * 빌드: make
 * 실행: ./bin/hmac_demo
 */

#include <stdio.h>
#include <string.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

/**
 * HMAC-SHA256을 계산한다.
 * 
 * @param key 비밀 키
 * @param key_len 키 길이
 * @param data 메시지 데이터
 * @param data_len 데이터 길이
 * @param mac MAC 결과를 저장할 버퍼 (최소 32바이트)
 * @return 성공 시 0, 실패 시 -1
 */
int calculate_hmac_sha256(const unsigned char *key, size_t key_len,
                          const unsigned char *data, size_t data_len,
                          unsigned char *mac) {
    unsigned int mac_len = 0;
    
    unsigned char *result = HMAC(EVP_sha256(), 
                                  key, (int)key_len,
                                  data, data_len,
                                  mac, &mac_len);
    
    return (result != NULL) ? 0 : -1;
}

/**
 * 두 MAC을 상수 시간으로 비교한다.
 * (타이밍 공격 방지)
 */
int verify_mac(const unsigned char *mac1, 
               const unsigned char *mac2, 
               size_t len) {
    int result = 0;
    for (size_t i = 0; i < len; i++) {
        result |= mac1[i] ^ mac2[i];
    }
    return (result == 0) ? 0 : -1;
}

/**
 * MAC을 16진수로 출력한다.
 */
void print_mac(const unsigned char *mac, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", mac[i]);
    }
    printf("\n");
}

int main(void) {
    printf("=== HMAC-SHA256 메시지 인증 코드 데모 ===\n\n");
    
    // 비밀 키 (실제로는 안전하게 생성/저장해야 함)
    const unsigned char key1[] = "SecretKey123!@#";
    const unsigned char key2[] = "DifferentKey456";
    size_t key1_len = strlen((char *)key1);
    size_t key2_len = strlen((char *)key2);
    
    // 메시지
    const unsigned char message[] = "CAN Message: Engine RPM = 3500";
    size_t msg_len = strlen((char *)message);
    
    unsigned char mac[32];
    
    printf("메시지: \"%s\"\n", message);
    printf("길이: %zu 바이트\n\n", msg_len);
    
    // 키 1로 HMAC 계산
    printf("키 1: \"%s\"\n", key1);
    calculate_hmac_sha256(key1, key1_len, message, msg_len, mac);
    printf("HMAC: ");
    print_mac(mac, 32);
    printf("\n");
    
    // 키 2로 HMAC 계산
    printf("키 2: \"%s\"\n", key2);
    calculate_hmac_sha256(key2, key2_len, message, msg_len, mac);
    printf("HMAC: ");
    print_mac(mac, 32);
    printf("\n");
    
    printf("→ 동일 메시지라도 키가 다르면 다른 MAC 생성\n\n");
    
    // MAC 검증 시나리오
    printf("=== MAC 검증 시나리오 ===\n\n");
    
    // 1. 정상 메시지 검증
    unsigned char original_mac[32];
    calculate_hmac_sha256(key1, key1_len, message, msg_len, original_mac);
    
    printf("1. 정상 메시지 검증\n");
    printf("   저장된 MAC: ");
    print_mac(original_mac, 32);
    
    unsigned char verify_mac_buf[32];
    calculate_hmac_sha256(key1, key1_len, message, msg_len, verify_mac_buf);
    
    if (verify_mac(original_mac, verify_mac_buf, 32) == 0) {
        printf("   ✓ 검증 성공: 메시지 무결성 확인\n\n");
    } else {
        printf("   ✗ 검증 실패\n\n");
    }
    
    // 2. 변조된 메시지 검증
    printf("2. 변조된 메시지 검증\n");
    unsigned char tampered[] = "CAN Message: Engine RPM = 9999";  // 값 변경
    size_t tampered_len = strlen((char *)tampered);
    
    printf("   원본 메시지: \"%s\"\n", message);
    printf("   변조 메시지: \"%s\"\n", tampered);
    
    calculate_hmac_sha256(key1, key1_len, tampered, tampered_len, verify_mac_buf);
    
    if (verify_mac(original_mac, verify_mac_buf, 32) == 0) {
        printf("   ✓ 검증 성공\n\n");
    } else {
        printf("   ✗ 검증 실패: 메시지 변조 탐지!\n\n");
    }
    
    // 3. 잘못된 키로 검증
    printf("3. 잘못된 키로 검증 시도\n");
    printf("   공격자가 다른 키로 MAC 생성 시도\n");
    
    calculate_hmac_sha256(key2, key2_len, message, msg_len, verify_mac_buf);
    
    if (verify_mac(original_mac, verify_mac_buf, 32) == 0) {
        printf("   ✓ 검증 성공\n\n");
    } else {
        printf("   ✗ 검증 실패: 키 불일치 탐지!\n\n");
    }
    
    printf("=== 차량 보안 적용 ===\n\n");
    printf("HMAC은 차량 내부 통신(SecOC)에서 다음과 같이 활용된다:\n");
    printf("  1. 송신 ECU: 메시지 + HMAC(비밀키, 메시지) 전송\n");
    printf("  2. 수신 ECU: 동일 키로 HMAC 계산 후 비교\n");
    printf("  3. 불일치 시 메시지 폐기 (변조 또는 위조 탐지)\n");
    
    return 0;
}
