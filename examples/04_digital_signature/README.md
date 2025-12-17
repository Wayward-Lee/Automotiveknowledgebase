# Example 04: 디지털 서명 및 검증

RSA-SHA256과 ECDSA를 사용한 디지털 서명을 학습한다.

---

## 학습 목표

1. 디지털 서명의 원리를 이해한다
2. RSA-SHA256, ECDSA 서명을 구현한다
3. 자체 서명 인증서를 생성한다

---

## 파일 구조

```
04_digital_signature/
├── README.md
├── Makefile
└── src/
    ├── rsa_sign.c       # RSA-SHA256 서명/검증
    ├── ecdsa_sign.c     # ECDSA 서명/검증
    └── self_cert.c      # 자체 서명 인증서 생성
```

---

## 핵심 개념

```
[서명 과정]
  문서 → SHA-256 해시 → 개인키로 암호화 → 서명

[검증 과정]
  서명 → 공개키로 복호화 → 해시값 추출
  문서 → SHA-256 해시 → 비교
  일치하면 ✓ 유효, 불일치면 ✗ 위조
```

---

## 차량 보안 적용

- **Secure Boot**: 부트로더/OS 이미지 서명 검증
- **OTA 업데이트**: 펌웨어 패키지 서명
- **진단 인증**: 진단 세션 인증서 검증

---

## 빌드 및 실행

```bash
make
./bin/rsa_sign
./bin/ecdsa_sign
./bin/self_cert
```
