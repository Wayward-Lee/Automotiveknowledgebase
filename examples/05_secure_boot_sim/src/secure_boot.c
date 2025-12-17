/**
 * secure_boot.c - Secure Boot 시뮬레이션
 * 
 * 신뢰 체인 기반의 Secure Boot 과정을 시뮬레이션한다.
 */

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

typedef struct {
    char name[32];
    unsigned char data[256];
    size_t data_len;
    unsigned char signature[128];
    size_t sig_len;
} BootStage;

// 루트 키 (HSM에 저장됨)
EVP_PKEY *root_key = NULL;

int verify_stage(const char *stage_name, BootStage *stage);

void create_root_key(void) {
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    EVP_PKEY_keygen_init(ctx);
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, NID_X9_62_prime256v1);
    EVP_PKEY_keygen(ctx, &root_key);
    EVP_PKEY_CTX_free(ctx);
}

void sign_stage(BootStage *stage) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestSignInit(ctx, NULL, EVP_sha256(), NULL, root_key);
    EVP_DigestSignUpdate(ctx, stage->data, stage->data_len);
    EVP_DigestSignFinal(ctx, NULL, &stage->sig_len);
    EVP_DigestSignFinal(ctx, stage->signature, &stage->sig_len);
    EVP_MD_CTX_free(ctx);
}

int verify_stage(const char *stage_name, BootStage *stage) {
    printf("  [%s] 서명 검증 중...\n", stage_name);
    
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestVerifyInit(ctx, NULL, EVP_sha256(), NULL, root_key);
    EVP_DigestVerifyUpdate(ctx, stage->data, stage->data_len);
    int result = EVP_DigestVerifyFinal(ctx, stage->signature, stage->sig_len);
    EVP_MD_CTX_free(ctx);
    
    if (result == 1) {
        printf("  [%s] ✓ 검증 성공\n", stage_name);
        return 1;
    } else {
        printf("  [%s] ✗ 검증 실패! 부팅 중단\n", stage_name);
        return 0;
    }
}

int main(void) {
    printf("=== Secure Boot 시뮬레이션 ===\n\n");
    
    // 루트 키 생성 (HSM 시뮬레이션)
    printf("[ROM] Root of Trust 초기화\n");
    create_root_key();
    printf("[ROM] 루트 공개키 로드 완료\n\n");
    
    // 부팅 단계 생성
    BootStage stage1 = {"Stage1_Bootloader", {0}, 0, {0}, 0};
    BootStage stage2 = {"Stage2_Bootloader", {0}, 0, {0}, 0};
    BootStage stage3 = {"OS_Kernel", {0}, 0, {0}, 0};
    
    // 각 단계 데이터 설정 (펌웨어 이미지 시뮬레이션)
    strcpy((char *)stage1.data, "BOOTLOADER_STAGE1_v1.0.0_CODE");
    stage1.data_len = strlen((char *)stage1.data);
    
    strcpy((char *)stage2.data, "BOOTLOADER_STAGE2_v2.0.0_CODE");
    stage2.data_len = strlen((char *)stage2.data);
    
    strcpy((char *)stage3.data, "OS_KERNEL_v3.0.0_IMAGE_CODE_XYZ");
    stage3.data_len = strlen((char *)stage3.data);
    
    // 서명 (제조 시 수행)
    sign_stage(&stage1);
    sign_stage(&stage2);
    sign_stage(&stage3);
    
    printf("=== 정상 부팅 시나리오 ===\n\n");
    
    printf("[STEP 1] 1차 부트로더 검증\n");
    if (!verify_stage("Stage1", &stage1)) goto boot_fail;
    printf("  → 1차 부트로더 실행\n\n");
    
    printf("[STEP 2] 2차 부트로더 검증\n");
    if (!verify_stage("Stage2", &stage2)) goto boot_fail;
    printf("  → 2차 부트로더 실행\n\n");
    
    printf("[STEP 3] OS 커널 검증\n");
    if (!verify_stage("Kernel", &stage3)) goto boot_fail;
    printf("  → OS 커널 실행\n\n");
    
    printf("═══════════════════════════════\n");
    printf("✓ Secure Boot 완료: 시스템 시작\n");
    printf("═══════════════════════════════\n\n");
    
    // 변조 시나리오
    printf("=== 변조 탐지 시나리오 ===\n\n");
    
    stage2.data[0] ^= 0xFF;  // 데이터 변조
    
    printf("[STEP 1] 1차 부트로더 검증\n");
    if (!verify_stage("Stage1", &stage1)) goto boot_fail_demo;
    printf("  → 1차 부트로더 실행\n\n");
    
    printf("[STEP 2] 2차 부트로더 검증 (변조됨)\n");
    if (!verify_stage("Stage2", &stage2)) {
        printf("\n═══════════════════════════════\n");
        printf("✗ 부팅 실패: 악성 코드 실행 방지\n");
        printf("═══════════════════════════════\n");
    }
    
boot_fail_demo:
    EVP_PKEY_free(root_key);
    return 0;
    
boot_fail:
    printf("부팅 중단!\n");
    EVP_PKEY_free(root_key);
    return 1;
}
