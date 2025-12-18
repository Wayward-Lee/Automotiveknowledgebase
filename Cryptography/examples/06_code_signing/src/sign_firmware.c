/**
 * sign_firmware.c - 펌웨어 서명 도구
 */

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

#pragma pack(push, 1)
typedef struct {
    char magic[8];          // "FWSIGN\0\0"
    uint32_t version;       // 헤더 버전
    uint32_t algorithm;     // 1 = ECDSA-P256-SHA256
    uint32_t payload_size;  // 페이로드 크기
    uint32_t sig_length;    // 서명 길이
    uint8_t reserved[44];   // 예약 (총 64바이트)
} FirmwareHeader;
#pragma pack(pop)

int main(void) {
    printf("=== 펌웨어 코드 서명 도구 ===\n\n");
    
    // 서명 키 생성
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_keygen_init(ctx);
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, NID_X9_62_prime256v1);
    EVP_PKEY_keygen(ctx, &pkey);
    EVP_PKEY_CTX_free(ctx);
    
    // 키 저장
    FILE *kf = fopen("signing_key.pem", "w");
    PEM_write_PrivateKey(kf, pkey, NULL, NULL, 0, NULL, NULL);
    fclose(kf);
    
    FILE *pf = fopen("public_key.pem", "w");
    PEM_write_PUBKEY(pf, pkey);
    fclose(pf);
    
    printf("서명 키 생성: signing_key.pem, public_key.pem\n\n");
    
    // 샘플 펌웨어 생성
    const char *fw_data = "ECU_FIRMWARE_v2.5.1_BUILD_20231217_RELEASE";
    size_t fw_len = strlen(fw_data);
    
    printf("펌웨어 데이터: \"%s\"\n", fw_data);
    printf("크기: %zu 바이트\n\n", fw_len);
    
    // 서명 생성
    EVP_MD_CTX *sign_ctx = EVP_MD_CTX_new();
    EVP_DigestSignInit(sign_ctx, NULL, EVP_sha256(), NULL, pkey);
    EVP_DigestSignUpdate(sign_ctx, fw_data, fw_len);
    
    size_t sig_len;
    EVP_DigestSignFinal(sign_ctx, NULL, &sig_len);
    unsigned char signature[128];
    EVP_DigestSignFinal(sign_ctx, signature, &sig_len);
    EVP_MD_CTX_free(sign_ctx);
    
    printf("서명 길이: %zu 바이트\n", sig_len);
    printf("서명: ");
    for (size_t i = 0; i < 32; i++) printf("%02x", signature[i]);
    printf("...\n\n");
    
    // 서명된 패키지 생성
    FirmwareHeader header = {0};
    memcpy(header.magic, "FWSIGN", 6);
    header.version = 1;
    header.algorithm = 1;  // ECDSA-P256-SHA256
    header.payload_size = (uint32_t)fw_len;
    header.sig_length = (uint32_t)sig_len;
    
    FILE *out = fopen("signed_firmware.bin", "wb");
    fwrite(&header, 1, sizeof(header), out);
    fwrite(signature, 1, sig_len, out);
    fwrite(fw_data, 1, fw_len, out);
    fclose(out);
    
    printf("서명된 펌웨어 저장: signed_firmware.bin\n");
    printf("  헤더: %zu 바이트\n", sizeof(header));
    printf("  서명: %zu 바이트\n", sig_len);
    printf("  페이로드: %zu 바이트\n", fw_len);
    printf("  총합: %zu 바이트\n\n", sizeof(header) + sig_len + fw_len);
    
    printf("검증 명령: ./bin/verify_firmware signed_firmware.bin\n");
    
    EVP_PKEY_free(pkey);
    return 0;
}
