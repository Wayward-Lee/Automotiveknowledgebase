/**
 * verify_firmware.c - 펌웨어 검증 도구
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#pragma pack(push, 1)
typedef struct {
    char magic[8];
    uint32_t version;
    uint32_t algorithm;
    uint32_t payload_size;
    uint32_t sig_length;
    uint8_t reserved[44];
} FirmwareHeader;
#pragma pack(pop)

int main(int argc, char *argv[]) {
    printf("=== 펌웨어 서명 검증 도구 ===\n\n");
    
    const char *fw_file = (argc > 1) ? argv[1] : "signed_firmware.bin";
    const char *key_file = "public_key.pem";
    
    // 공개키 로드
    FILE *kf = fopen(key_file, "r");
    if (!kf) {
        printf("공개키 파일 없음. 먼저 sign_firmware를 실행하세요.\n");
        return 1;
    }
    EVP_PKEY *pkey = PEM_read_PUBKEY(kf, NULL, NULL, NULL);
    fclose(kf);
    
    // 서명된 펌웨어 로드
    FILE *fw = fopen(fw_file, "rb");
    if (!fw) {
        printf("펌웨어 파일 없음: %s\n", fw_file);
        EVP_PKEY_free(pkey);
        return 1;
    }
    
    FirmwareHeader header;
    fread(&header, 1, sizeof(header), fw);
    
    printf("파일: %s\n\n", fw_file);
    printf("=== 헤더 정보 ===\n");
    printf("Magic: %.6s\n", header.magic);
    printf("버전: %u\n", header.version);
    printf("알고리즘: %s\n", header.algorithm == 1 ? "ECDSA-P256-SHA256" : "Unknown");
    printf("페이로드 크기: %u 바이트\n", header.payload_size);
    printf("서명 길이: %u 바이트\n\n", header.sig_length);
    
    // Magic 검사
    if (memcmp(header.magic, "FWSIGN", 6) != 0) {
        printf("✗ 잘못된 매직 넘버\n");
        fclose(fw);
        EVP_PKEY_free(pkey);
        return 1;
    }
    
    // 서명 읽기
    unsigned char *signature = malloc(header.sig_length);
    fread(signature, 1, header.sig_length, fw);
    
    // 페이로드 읽기
    unsigned char *payload = malloc(header.payload_size);
    fread(payload, 1, header.payload_size, fw);
    fclose(fw);
    
    printf("=== 서명 검증 ===\n");
    
    // 검증
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestVerifyInit(ctx, NULL, EVP_sha256(), NULL, pkey);
    EVP_DigestVerifyUpdate(ctx, payload, header.payload_size);
    int result = EVP_DigestVerifyFinal(ctx, signature, header.sig_length);
    EVP_MD_CTX_free(ctx);
    
    if (result == 1) {
        printf("✓ 서명 검증 성공!\n\n");
        printf("펌웨어 내용:\n  \"%.*s\"\n\n", header.payload_size, payload);
        printf("→ 펌웨어가 정품이며 변조되지 않았습니다.\n");
        printf("→ 설치를 진행해도 안전합니다.\n");
    } else {
        printf("✗ 서명 검증 실패!\n\n");
        printf("→ 펌웨어가 변조되었거나 출처가 불분명합니다.\n");
        printf("→ 설치를 중단합니다.\n");
    }
    
    free(signature);
    free(payload);
    EVP_PKEY_free(pkey);
    return (result == 1) ? 0 : 1;
}
