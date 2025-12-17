/**
 * tls_client.c - TLS 클라이언트 (차량 텔레매틱스 시뮬레이션)
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8443

int main(void) {
    printf("=== TLS 클라이언트 (차량 텔레매틱스 시뮬레이션) ===\n\n");
    
    SSL_library_init();
    SSL_load_error_strings();
    
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    
    // 서버 인증서 검증 비활성화 (데모용)
    // 실제로는 CA 인증서로 검증해야 함
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    
    // 소켓 연결
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT)
    };
    inet_pton(AF_INET, SERVER_ADDR, &addr.sin_addr);
    
    printf("서버 연결 중: %s:%d\n", SERVER_ADDR, SERVER_PORT);
    
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        printf("연결 실패. 서버가 실행 중인지 확인하세요.\n");
        return 1;
    }
    
    // TLS 핸드셰이크
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    
    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        return 1;
    }
    
    printf("TLS 핸드셰이크 완료!\n");
    printf("프로토콜: %s\n", SSL_get_version(ssl));
    printf("암호 스위트: %s\n\n", SSL_get_cipher(ssl));
    
    // 서버 인증서 정보
    X509 *cert = SSL_get_peer_certificate(ssl);
    if (cert) {
        char *subject = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);
        printf("서버 인증서 CN: %s\n\n", subject);
        OPENSSL_free(subject);
        X509_free(cert);
    }
    
    // 요청 전송
    const char *request = "Vehicle XYZ123: Check for updates";
    SSL_write(ssl, request, strlen(request));
    printf("송신: %s\n", request);
    
    // 응답 수신
    char buffer[256];
    int bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("수신: %s\n\n", buffer);
    }
    
    printf("✓ 보안 통신 완료\n");
    printf("→ 모든 데이터가 TLS로 암호화되어 전송됨\n");
    
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(ctx);
    
    return 0;
}
