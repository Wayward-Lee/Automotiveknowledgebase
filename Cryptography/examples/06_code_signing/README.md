# Example 06: 코드 서명 및 검증 시스템

펌웨어 이미지에 서명하고 검증하는 워크플로우를 구현한다.

---

## 학습 목표

1. 코드 서명 워크플로우를 이해한다
2. 서명 헤더 구조를 설계한다
3. OTA 업데이트 시나리오를 시뮬레이션한다

---

## 파일 구조

```
06_code_signing/
├── README.md
├── Makefile
└── src/
    ├── sign_firmware.c    # 펌웨어 서명 도구
    └── verify_firmware.c  # 펌웨어 검증 도구
```

---

## 서명 패키지 구조

```
┌─────────────────────────────────────┐
│ 헤더 (64 바이트)                    │
│   - Magic: "FWSIGN"                 │
│   - Version: 1                      │
│   - Algorithm: ECDSA-P256-SHA256    │
│   - Payload Size                    │
│   - Signature Length                │
├─────────────────────────────────────┤
│ 서명 (~64 바이트)                   │
├─────────────────────────────────────┤
│ 펌웨어 페이로드                     │
└─────────────────────────────────────┘
```

---

## 빌드 및 실행

```bash
make

# 펌웨어 서명
./bin/sign_firmware sample.bin signed.bin

# 서명 검증
./bin/verify_firmware signed.bin
```
