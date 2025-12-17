/**
 * key_derivation.c - 키 파생 함수 데모
 * 
 * PBKDF2를 사용하여 비밀번호로부터 암호화 키를 파생한다.
 * 
 * 빌드: make
 * 실행: ./bin/key_derivation
 */

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define SALT_SIZE 16
#define KEY_SIZE 32      // 256 bits
#define ITERATIONS 100000 // PBKDF2 반복 횟수

/**
 * PBKDF2-HMAC-SHA256으로 키를 파생한다.
 */
int derive_key_pbkdf2(const char *password,
                      const unsigned char *salt, size_t salt_len,
                      int iterations,
                      unsigned char *key, size_t key_len) {
    return PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                             salt, (int)salt_len,
                             iterations,
                             EVP_sha256(),
                             (int)key_len, key);
}

void print_hex(const char *label, const unsigned char *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main(void) {
    printf("=== 키 파생 함수 (PBKDF2) 데모 ===\n\n");
    
    printf("PBKDF2 (Password-Based Key Derivation Function 2)\n");
    printf("비밀번호로부터 안전한 암호화 키를 파생한다.\n\n");
    
    const char *password = "MySecretPassword123!";
    
    // 솔트 생성 (각 사용자/세션마다 고유해야 함)
    unsigned char salt[SALT_SIZE];
    RAND_bytes(salt, SALT_SIZE);
    
    printf("비밀번호: \"%s\"\n", password);
    print_hex("솔트 (랜덤)", salt, SALT_SIZE);
    printf("반복 횟수: %d\n", ITERATIONS);
    printf("출력 키 길이: %d 비트\n\n", KEY_SIZE * 8);
    
    // 키 파생
    unsigned char derived_key[KEY_SIZE];
    
    if (derive_key_pbkdf2(password, salt, SALT_SIZE, 
                          ITERATIONS, derived_key, KEY_SIZE) != 1) {
        printf("키 파생 실패\n");
        return 1;
    }
    
    print_hex("파생된 키", derived_key, KEY_SIZE);
    printf("\n");
    
    // 동일 비밀번호, 동일 솔트 → 동일 키
    printf("=== 결정론적 특성 ===\n");
    unsigned char key2[KEY_SIZE];
    derive_key_pbkdf2(password, salt, SALT_SIZE, ITERATIONS, key2, KEY_SIZE);
    
    printf("동일 비밀번호 + 동일 솔트:\n");
    print_hex("  키 1", derived_key, KEY_SIZE);
    print_hex("  키 2", key2, KEY_SIZE);
    
    if (memcmp(derived_key, key2, KEY_SIZE) == 0) {
        printf("  ✓ 동일한 키 생성됨\n\n");
    }
    
    // 다른 솔트 → 다른 키
    printf("=== 솔트의 역할 ===\n");
    unsigned char salt2[SALT_SIZE];
    RAND_bytes(salt2, SALT_SIZE);
    
    unsigned char key3[KEY_SIZE];
    derive_key_pbkdf2(password, salt2, SALT_SIZE, ITERATIONS, key3, KEY_SIZE);
    
    printf("동일 비밀번호 + 다른 솔트:\n");
    print_hex("  솔트 1", salt, SALT_SIZE);
    print_hex("  솔트 2", salt2, SALT_SIZE);
    print_hex("  키 1", derived_key, KEY_SIZE);
    print_hex("  키 3", key3, KEY_SIZE);
    
    if (memcmp(derived_key, key3, KEY_SIZE) != 0) {
        printf("  → 다른 키 생성됨\n");
        printf("  → 솔트는 레인보우 테이블 공격 방지\n\n");
    }
    
    // 다른 비밀번호 → 다른 키
    printf("=== 비밀번호 민감성 ===\n");
    const char *password2 = "MySecretPassword123?";  // '!' → '?'
    
    unsigned char key4[KEY_SIZE];
    derive_key_pbkdf2(password2, salt, SALT_SIZE, ITERATIONS, key4, KEY_SIZE);
    
    printf("원본: \"%s\"\n", password);
    printf("변경: \"%s\" (마지막 문자 다름)\n", password2);
    print_hex("원본 키", derived_key, KEY_SIZE);
    print_hex("변경 키", key4, KEY_SIZE);
    printf("→ 1문자 차이로 완전히 다른 키\n\n");
    
    // 반복 횟수의 의미
    printf("=== 반복 횟수 (Iterations) ===\n");
    printf("반복 횟수가 높을수록:\n");
    printf("  - 브루트포스 공격 비용 증가\n");
    printf("  - 키 생성 시간도 증가\n");
    printf("\n권장 값:\n");
    printf("  - PBKDF2-SHA256: 최소 100,000회\n");
    printf("  - 매년 하드웨어 성능에 따라 상향 조정\n\n");
    
    printf("=== 차량 보안 적용 ===\n");
    printf("PBKDF2 활용 예:\n");
    printf("  1. 진단 도구 인증 비밀번호 → 세션 키\n");
    printf("  2. OTA 업데이트 암호화 키 파생\n");
    printf("  3. 보안 저장소 마스터 키 파생\n");
    
    return 0;
}
