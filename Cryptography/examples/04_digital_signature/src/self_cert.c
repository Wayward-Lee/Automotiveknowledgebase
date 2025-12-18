/**
 * self_cert.c - 자체 서명 X.509 인증서 생성
 */

#include <stdio.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/evp.h>

int main(void) {
    printf("=== 자체 서명 X.509 인증서 생성 ===\n\n");
    
    // 키 생성
    EVP_PKEY_CTX *keygen_ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_keygen_init(keygen_ctx);
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid(keygen_ctx, NID_X9_62_prime256v1);
    EVP_PKEY_keygen(keygen_ctx, &pkey);
    EVP_PKEY_CTX_free(keygen_ctx);
    
    // 인증서 생성
    X509 *x509 = X509_new();
    
    // 시리얼 번호
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    
    // 유효 기간 (1년)
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 365 * 24 * 60 * 60);
    
    // 공개키 설정
    X509_set_pubkey(x509, pkey);
    
    // Subject 설정
    X509_NAME *name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, (unsigned char *)"KR", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, (unsigned char *)"Vehicle OEM", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)"ECU Signing Key", -1, -1, 0);
    
    // 자체 서명 (Issuer = Subject)
    X509_set_issuer_name(x509, name);
    X509_sign(x509, pkey, EVP_sha256());
    
    // 파일로 저장
    FILE *cert_file = fopen("ecu_cert.pem", "w");
    PEM_write_X509(cert_file, x509);
    fclose(cert_file);
    
    FILE *key_file = fopen("ecu_key.pem", "w");
    PEM_write_PrivateKey(key_file, pkey, NULL, NULL, 0, NULL, NULL);
    fclose(key_file);
    
    printf("인증서 생성 완료!\n\n");
    printf("파일:\n");
    printf("  ecu_cert.pem - X.509 인증서\n");
    printf("  ecu_key.pem  - 개인키\n\n");
    
    printf("인증서 정보:\n");
    printf("  Subject: C=KR, O=Vehicle OEM, CN=ECU Signing Key\n");
    printf("  알고리즘: ECDSA P-256\n");
    printf("  유효기간: 1년\n\n");
    
    printf("확인 명령:\n");
    printf("  openssl x509 -in ecu_cert.pem -text -noout\n");
    
    X509_free(x509);
    EVP_PKEY_free(pkey);
    return 0;
}
