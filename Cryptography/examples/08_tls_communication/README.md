# Example 08: TLS 클라이언트/서버 통신

OpenSSL을 사용한 TLS 보안 통신을 구현한다.

---

## 학습 목표

1. TLS 핸드셰이크 과정을 이해한다
2. 인증서 기반 서버 인증을 구현한다
3. 차량-백엔드 통신 시나리오를 시뮬레이션한다

---

## 파일 구조

```
08_tls_communication/
├── README.md
├── Makefile
└── src/
    ├── tls_server.c    # TLS 서버
    └── tls_client.c    # TLS 클라이언트
```

---

## 실행 방법

```bash
make

# 인증서 생성
./bin/tls_server --gen-cert

# 터미널 1: 서버 시작
./bin/tls_server

# 터미널 2: 클라이언트 연결
./bin/tls_client
```

---

## TLS 핸드셰이크

```
클라이언트                   서버
    │                         │
    ├── ClientHello ─────────→│
    │                         │
    │←───── ServerHello ──────┤
    │←───── Certificate ──────┤
    │←───── ServerHelloDone ──┤
    │                         │
    ├── ClientKeyExchange ───→│
    ├── ChangeCipherSpec ────→│
    ├── Finished ────────────→│
    │                         │
    │←── ChangeCipherSpec ────┤
    │←────── Finished ────────┤
    │                         │
    │← 암호화된 통신 시작 →│
```
