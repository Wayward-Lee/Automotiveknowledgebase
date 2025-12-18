# Example 07: SecOC 메시지 인증

AUTOSAR SecOC 표준에 따른 CAN 메시지 인증을 시뮬레이션한다.

---

## 학습 목표

1. SecOC 표준의 원리를 이해한다
2. AES-CMAC 메시지 인증을 구현한다
3. Freshness Value(재전송 방지)를 학습한다

---

## 파일 구조

```
07_secoc_mac/
├── README.md
├── Makefile
└── src/
    ├── cmac_demo.c     # AES-CMAC 구현
    └── secoc_sim.c     # SecOC PDU 시뮬레이션
```

---

## SecOC PDU 구조

```
CAN 메시지 (최대 64바이트 in CAN-FD)
┌────────────────┬─────────────┬─────────────────┐
│  페이로드       │ Freshness   │  Truncated MAC  │
│  (가변)        │ (1-8 바이트) │  (2-8 바이트)   │
└────────────────┴─────────────┴─────────────────┘
```

---

## 빌드 및 실행

```bash
make
./bin/cmac_demo
./bin/secoc_sim
```
