# Example 05: Secure Boot 시뮬레이션

Secure Boot의 신뢰 체인(Chain of Trust)을 시뮬레이션한다.

---

## 학습 목표

1. Secure Boot 체인의 원리를 이해한다
2. 신뢰의 근원(Root of Trust) 개념을 학습한다
3. 다단계 검증 프로세스를 구현한다

---

## 파일 구조

```
05_secure_boot_sim/
├── README.md
├── Makefile
└── src/
    └── secure_boot.c   # Secure Boot 시뮬레이션
```

---

## 시뮬레이션 시나리오

```
[ROM] ─검증→ [1차 부트로더] ─검증→ [2차 부트로더] ─검증→ [OS]
  │            │                  │                  │
  │            │                  │                  │
 신뢰          서명               서명               서명
 근원         필요               필요               필요
```

---

## 빌드 및 실행

```bash
make
./bin/secure_boot
```
