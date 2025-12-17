/**
 * tls_server.c - TLS 서버 (OTA 백엔드 시뮬레이션)
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

#define PORT 8443

void generate_self_signed_cert(void) {
    printf("=== 자체 서명 인증서 생성 ===\n");
    
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_keygen_init(pctx);
    EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 2048);
    EVP_PKEY_keygen(pctx, &pkey);
    EVP_PKEY_CTX_free(pctx);
    
    X509 *x509 = X509_new();
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 365*24*60*60);
    X509_set_pubkey(x509, pkey);
    
    X509_NAME *name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, 
                                (unsigned char *)"OTA Update Server", -1, -1, 0);
    X509_set_issuer_name(x509, name);
    X509_sign(x509, pkey, EVP_sha256());
    
    FILE *kf = fopen("server_key.pem", "w");
    PEM_write_PrivateKey(kf, pkey, NULL, NULL, 0, NULL, NULL);
    fclose(kf);
    
    FILE *cf = fopen("server_cert.pem", "w");
    PEM_write_X509(cf, x509);
    fclose(cf);
    
    printf("생성 완료: server_key.pem, server_cert.pem\n\n");
    
    X509_free(x509);
    EVP_PKEY_free(pkey);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "--gen-cert") == 0) {
        generate_self_signed_cert();
        return 0;
    }
    
    printf("=== TLS 서버 (OTA 백엔드 시뮬레이션) ===\n\n");
    
    SSL_library_init();
    SSL_load_error_strings();
    
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    
    // 인증서 로드
    if (SSL_CTX_use_certificate_file(ctx, "server_cert.pem", SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, "server_key.pem", SSL_FILETYPE_PEM) <= 0) {
        printf("인증서 로드 실패. --gen-cert로 먼저 생성하세요.\n");
        return 1;
    }
    
    // 소켓 생성
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY
    };
    
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 1);
    
    printf("포트 %d에서 TLS 연결 대기 중...\n", PORT);
    printf("클라이언트에서 ./bin/tls_client 실행\n\n");
    
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    
    printf("클라이언트 연결됨: %s\n", inet_ntoa(client_addr.sin_addr));
    
    // TLS 핸드셰이크
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_fd);
    
    if (SSL_accept(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
    } else {
        printf("TLS 핸드셰이크 완료\n");
        printf("암호 스위트: %s\n\n", SSL_get_cipher(ssl));
        
        // 데이터 수신
        char buffer[256];
        int bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            printf("수신: %s\n", buffer);
            
            // 응답 전송
            const char *response = "OTA: Firmware v2.5.1 available";
            SSL_write(ssl, response, strlen(response));
            printf("송신: %s\n", response);
        }
    }
    
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(client_fd);
    close(server_fd);
    SSL_CTX_free(ctx);
    
    return 0;
}
