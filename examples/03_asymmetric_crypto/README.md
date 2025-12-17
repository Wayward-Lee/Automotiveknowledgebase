# Example 03: 비대칭키 암호화 (RSA/ECC)

RSA와 타원 곡선 암호(ECC)를 사용한 공개키 암호화를 학습한다.

---

## 학습 목표

1. RSA 키 생성 및 암호화 원리를 이해한다
2. ECC(타원 곡선 암호)의 기초를 학습한다
3. 하이브리드 암호화 시스템을 구현한다
4. ECDH 키 교환을 이해한다

---

## 파일 구조

```
03_asymmetric_crypto/
├── README.md
├── Makefile
└── src/
    ├── rsa_keygen.c      # RSA 키 생성
    ├── rsa_encrypt.c     # RSA 암호화/복호화
    └── ecdh_exchange.c   # ECDH 키 교환
```

---

## 핵심 개념

### 하이브리드 암호화

```
[송신자]                              [수신자]
   │                                      │
   │  1. 랜덤 대칭키(AES 키) 생성          │
   │  2. 수신자 공개키로 대칭키 암호화      ←── 수신자 공개키
   │  3. 대칭키로 데이터 암호화             │
   │                                      │
   ├──→ 암호화된 대칭키 + 암호화된 데이터 ──→│
   │                                      │
   │                4. 개인키로 대칭키 복호화 ──→ 수신자 개인키
   │                5. 대칭키로 데이터 복호화
```

→ 대칭키의 속도 + 비대칭키의 키 교환 장점 결합

---

## 빌드 및 실행

```bash
make

# RSA 키 생성
./bin/rsa_keygen

# RSA 암호화 테스트
./bin/rsa_encrypt

# ECDH 키 교환
./bin/ecdh_exchange
```

---

## 참고 자료

- [Part 6: 암호화 기술 스택 - 기초 암호학](../../06_cryptography_fundamentals.md)
