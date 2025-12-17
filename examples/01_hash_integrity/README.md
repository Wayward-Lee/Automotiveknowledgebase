# Example 01: 해시 함수 및 무결성 검증

SHA-256 해시 알고리즘을 사용하여 데이터 무결성을 검증하는 방법을 학습한다.

---

## 학습 목표

1. SHA-256 해시 알고리즘의 원리를 이해한다
2. 파일 무결성 검증 도구를 구현한다
3. 눈사태 효과(Avalanche Effect)를 체험한다
4. HMAC-SHA256 메시지 인증 코드를 구현한다

---

## 이론 배경

### 해시 함수란?

암호학적 해시 함수는 임의 길이의 입력을 받아 고정 길이의 출력(해시 값)을 생성하는 단방향 함수이다. SHA-256은 256비트(32바이트) 해시 값을 출력하며, 현재 가장 널리 사용되는 해시 알고리즘 중 하나이다.

### 해시 함수의 특성

```
┌─────────────────────────────────────────────────────────────┐
│  입력: "Hello"                                               │
│  SHA-256 출력: 185f8db32271fe25f561a6fc938b2e264306ec304eda  │
│                88ea2618af7200a4e9e2a6...                     │
├─────────────────────────────────────────────────────────────┤
│  입력: "hello" (대소문자 변경)                               │
│  SHA-256 출력: 2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa  │
│                7425e73043362938b9824...                     │
└─────────────────────────────────────────────────────────────┘
  1글자만 바뀌어도 완전히 다른 해시 값 → 눈사태 효과
```

### 법규 연관성

UN R155 Annex 5는 소프트웨어 무결성 검증을 요구한다. 해시 함수는 소프트웨어 이미지가 변조되지 않았음을 확인하는 핵심 도구이다.

---

## 파일 구조

```
01_hash_integrity/
├── README.md           # 현재 문서
├── Makefile           # 빌드 스크립트
└── src/
    ├── hash_demo.c        # 기본 해시 계산
    ├── file_integrity.c   # 파일 무결성 검증
    ├── avalanche.c        # 눈사태 효과 시연
    └── hmac_demo.c        # HMAC 구현
```

---

## 빌드 및 실행

### 사전 요구사항

```bash
sudo apt update
sudo apt install -y build-essential libssl-dev
```

### 빌드

```bash
make
```

### 실행

```bash
# 기본 해시 계산
./bin/hash_demo

# 파일 무결성 검증
./bin/file_integrity <파일경로>

# 눈사태 효과 시연
./bin/avalanche

# HMAC 데모
./bin/hmac_demo
```

---

## 실습 과제

### 과제 1: 문자열 해시 계산
`hash_demo.c`를 실행하고 다양한 문자열의 해시 값을 확인하라.

### 과제 2: 파일 무결성 검증
1. 임의의 텍스트 파일을 생성한다
2. `file_integrity`로 해시를 계산한다
3. 파일을 1바이트 수정한다
4. 다시 해시를 계산하여 변화를 확인한다

### 과제 3: 눈사태 효과 분석
`avalanche.c`를 실행하여 1비트 변화가 해시 값에 미치는 영향을 분석하라.

### 과제 4: HMAC 키 의존성
`hmac_demo.c`에서 동일 메시지에 다른 키를 적용했을 때 결과가 어떻게 달라지는지 확인하라.

---

## 핵심 API (OpenSSL)

```c
// SHA-256 컨텍스트
EVP_MD_CTX *ctx = EVP_MD_CTX_new();

// 해시 초기화
EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);

// 데이터 추가
EVP_DigestUpdate(ctx, data, len);

// 해시 완료
EVP_DigestFinal_ex(ctx, hash, &hash_len);

// 정리
EVP_MD_CTX_free(ctx);
```

---

## 참고 자료

- [Part 6: 암호화 기술 스택 - 기초 암호학](../../06_cryptography_fundamentals.md)
- OpenSSL EVP 문서: https://www.openssl.org/docs/man3.0/man7/evp.html
