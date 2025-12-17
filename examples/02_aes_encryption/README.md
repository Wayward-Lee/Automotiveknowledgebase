# Example 02: 대칭키 암호화 (AES)

AES 알고리즘을 사용하여 데이터를 암호화하고 복호화하는 방법을 학습한다.

---

## 학습 목표

1. AES-128/256 암호화 원리를 이해한다
2. 블록 암호 운용 모드(CBC, GCM)를 구현한다
3. IV(초기화 벡터)의 역할을 이해한다
4. 인증 암호화(AEAD)의 개념을 학습한다

---

## 이론 배경

### AES란?

AES(Advanced Encryption Standard)는 128비트 블록 크기와 128/192/256비트 키 길이를 지원하는 대칭키 블록 암호이다. 동일한 키로 암호화와 복호화를 수행한다.

### 운용 모드

```
┌─────────────────────────────────────────────────────────────┐
│  ECB (Electronic Codebook)                                  │
│  - 각 블록을 독립적으로 암호화                               │
│  - ❌ 패턴이 노출됨, 사용 금지                               │
├─────────────────────────────────────────────────────────────┤
│  CBC (Cipher Block Chaining)                                │
│  - 이전 암호문 블록을 XOR                                   │
│  - ⚠️ 무결성 보장 안됨                                       │
├─────────────────────────────────────────────────────────────┤
│  GCM (Galois/Counter Mode)                                  │
│  - CTR 모드 + GHASH 인증                                    │
│  - ✓ 기밀성 + 무결성 동시 제공 (AEAD)                        │
│  - ✓ 차량 보안에 권장                                        │
└─────────────────────────────────────────────────────────────┘
```

### 법규 연관성

UN R155/R156은 통신 및 저장 데이터의 암호화를 요구한다. AES-GCM은 TLS 1.3, OTA 업데이트 패키지 암호화 등에 사용된다.

---

## 파일 구조

```
02_aes_encryption/
├── README.md           # 현재 문서
├── Makefile           # 빌드 스크립트
└── src/
    ├── aes_cbc.c          # AES-CBC 암호화/복호화
    ├── aes_gcm.c          # AES-GCM 인증 암호화
    ├── ecb_vs_cbc.c       # ECB vs CBC 비교
    └── key_derivation.c   # 키 파생 함수
```

---

## 빌드 및 실행

```bash
# 빌드
make

# AES-CBC 데모
./bin/aes_cbc

# AES-GCM 데모
./bin/aes_gcm

# ECB vs CBC 비교
./bin/ecb_vs_cbc

# 키 파생 데모
./bin/key_derivation
```

---

## 실습 과제

### 과제 1: AES-CBC 암호화
`aes_cbc.c`를 실행하고 동일 평문에 다른 IV를 사용했을 때 결과를 비교하라.

### 과제 2: AES-GCM 인증 암호화
`aes_gcm.c`에서 암호문을 1바이트 변조한 후 복호화 시 어떻게 되는지 확인하라.

### 과제 3: ECB 취약점 이해
`ecb_vs_cbc.c`를 실행하여 ECB 모드의 패턴 노출 문제를 시각적으로 확인하라.

### 과제 4: 비밀번호 기반 키 파생
`key_derivation.c`에서 PBKDF2를 사용하여 비밀번호로부터 안전한 암호화 키를 생성하라.

---

## 핵심 API (OpenSSL)

```c
// AES-GCM 암호화
EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv);
EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len);
EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);
```

---

## 참고 자료

- [Part 6: 암호화 기술 스택 - 기초 암호학](../../06_cryptography_fundamentals.md)
- OpenSSL EVP 암호화 가이드
