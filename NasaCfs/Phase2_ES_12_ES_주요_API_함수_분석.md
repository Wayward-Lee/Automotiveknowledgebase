# Phase 2 ES-12: ES 주요 API 함수 분석

## 서론

지금까지 ES 모듈의 다양한 기능을 살펴보았다. 본 문서에서는 ES가 제공하는 주요 API 함수들을 카테고리별로 정리하고, 각 함수의 용도, 파라미터, 반환 값, 사용 예시를 종합적으로 분석한다.

이 문서는 ES API의 레퍼런스 가이드로 활용할 수 있다.

---

## 1. 애플리케이션 생명주기 API

cFS 애플리케이션 개발에서 가장 기본이 되는 API들이다. 모든 애플리케이션은 시작 시 ES에 자신을 등록하고, 실행 중에는 ES와 협력하여 종료 요청을 확인하며, 종료 시에는 ES에 알리고 리소스를 정리해야 한다. 이 세 함수는 애플리케이션의 기본 구조를 형성한다.

### 1.1 CFE_ES_RegisterApp

모든 cFS 애플리케이션은 Main 함수의 가장 처음에 `CFE_ES_RegisterApp()` 함수를 호출해야 한다. 이 함수는 ES에게 해당 앱이 실행을 시작했음을 알리고, ES가 앱의 상태를 추적할 수 있게 해준다. 등록하지 않으면 ES는 앱이 정상적으로 시작되었는지 알 수 없으며, 일부 ES 기능이 제대로 동작하지 않을 수 있다.

```c
/**
 * @brief 애플리케이션을 ES에 등록
 * 
 * 모든 cFS 앱은 Main 함수 시작 시 이 함수를 호출해야 한다.
 *
 * @return CFE_SUCCESS        등록 성공
 * @return CFE_ES_ERR_APP_REGISTER 등록 실패
 *
 * @note 앱 Main 함수에서 첫 번째로 호출해야 함
 */
CFE_Status_t CFE_ES_RegisterApp(void);

/* 사용 예시 */
void MY_APP_Main(void)
{
    CFE_ES_RegisterApp();  /* 필수 */
    /* 이후 초기화 */
}
```

### 1.2 CFE_ES_RunLoop

```c
/**
 * @brief 앱 실행 루프 제어
 *
 * 앱이 계속 실행해야 하는지 확인한다.
 * ES는 이 함수를 통해 앱에 종료 신호를 전달한다.
 *
 * @param[in,out] RunStatus  앱의 실행 상태
 *
 * @return true   계속 실행
 * @return false  종료 필요
 *
 * @note 메인 루프 조건으로 사용
 */
bool CFE_ES_RunLoop(uint32 *RunStatus);

/* 사용 예시 */
uint32 RunStatus = CFE_ES_RunStatus_APP_RUN;

while (CFE_ES_RunLoop(&RunStatus))
{
    /* 메시지 처리 */
}
```

### 1.3 CFE_ES_ExitApp

```c
/**
 * @brief 앱 종료
 *
 * 앱이 종료될 때 호출한다. 이 함수는 반환하지 않는다.
 *
 * @param[in] ExitStatus  종료 상태 (CFE_ES_RunStatus_*)
 *
 * @note Main 함수 끝에서 호출
 */
void CFE_ES_ExitApp(uint32 ExitStatus);

/* 사용 예시 */
void MY_APP_Main(void)
{
    /* ... */
    CFE_ES_ExitApp(CFE_ES_RunStatus_APP_EXIT);
}
```

### 1.4 CFE_ES_RestartApp / ReloadApp / DeleteApp

```c
/**
 * @brief 앱 재시작
 * @param[in] AppID  대상 앱 ID
 */
CFE_Status_t CFE_ES_RestartApp(CFE_ES_AppId_t AppID);

/**
 * @brief 다른 파일로 앱 재로드
 * @param[in] AppID     대상 앱 ID
 * @param[in] FileName  새 앱 파일 경로
 */
CFE_Status_t CFE_ES_ReloadApp(CFE_ES_AppId_t AppID, const char *FileName);

/**
 * @brief 앱 삭제
 * @param[in] AppID  대상 앱 ID
 */
CFE_Status_t CFE_ES_DeleteApp(CFE_ES_AppId_t AppID);
```

---

## 2. 앱/태스크 정보 조회 API

애플리케이션은 종종 자신이나 다른 앱/태스크에 대한 정보를 조회해야 한다. ES는 이를 위한 다양한 조회 함수를 제공한다. 앱 ID를 컴으으로써 앱의 상태, 실행 카운터, 우선순위 등의 상세 정보를 얻을 수 있다.

### 2.1 앱 ID 조회

앱 ID는 ES가 각 애플리케이션을 식별하는 고유한 식별자이다. `CFE_ES_GetAppID()` 함수는 현재 실행 중인 앱의 ID를 반환하고, `CFE_ES_GetAppIDByName()` 함수는 앱 이름으로 ID를 조회한다. 앱 ID는 다른 ES API 호출에 필요한 파라미터로 자주 사용된다.

```c
/**
 * @brief 현재 앱 ID 조회
 * @param[out] AppIdPtr  앱 ID 반환
 */
CFE_Status_t CFE_ES_GetAppID(CFE_ES_AppId_t *AppIdPtr);

/**
 * @brief 이름으로 앱 ID 조회
 * @param[out] AppIdPtr  앱 ID 반환
 * @param[in]  AppName   앱 이름
 */
CFE_Status_t CFE_ES_GetAppIDByName(CFE_ES_AppId_t *AppIdPtr, 
                                    const char *AppName);

/* 사용 예시 */
CFE_ES_AppId_t AppId;
CFE_ES_GetAppID(&AppId);  /* 현재 앱 */

CFE_ES_AppId_t OtherAppId;
CFE_ES_GetAppIDByName(&OtherAppId, "OTHER_APP");  /* 다른 앱 */
```

### 2.2 앱 정보 조회

```c
/**
 * @brief 앱 상세 정보 조회
 * @param[out] AppInfo  정보 구조체
 * @param[in]  AppId    앱 ID
 */
CFE_Status_t CFE_ES_GetAppInfo(CFE_ES_AppInfo_t *AppInfo, 
                                CFE_ES_AppId_t AppId);

/**
 * @brief 앱 이름 조회
 * @param[out] AppName  이름 버퍼
 * @param[in]  AppId    앱 ID
 * @param[in]  BufferLength 버퍼 크기
 */
CFE_Status_t CFE_ES_GetAppName(char *AppName, 
                                CFE_ES_AppId_t AppId, 
                                size_t BufferLength);
```

### 2.3 태스크 정보 조회

```c
/**
 * @brief 현재 태스크 ID 조회
 */
CFE_Status_t CFE_ES_GetTaskID(CFE_ES_TaskId_t *TaskIdPtr);

/**
 * @brief 태스크 상세 정보 조회
 */
CFE_Status_t CFE_ES_GetTaskInfo(CFE_ES_TaskInfo_t *TaskInfo, 
                                 CFE_ES_TaskId_t TaskId);
```

---

## 3. 자식 태스크 API

자식 태스크는 애플리케이션 내에서 병렬 처리가 필요할 때 사용된다. 메인 태스크와 독립적으로 실행되며, 바로킹 I/O 처리, 높은 주기 데이터 수집, CPU 집약적 백그라운드 작업 등에 적합하다.

### 3.1 CFE_ES_CreateChildTask

자식 태스크를 생성하는 했심 함수이다. 태스크 이름, 진입점 함수, 스택 크기, 우선순위 등을 지정하여 새로운 태스크를 생성한다. 생성된 자식 태스크는 부모 앱과 동일한 주소 공간을 공유하므로 데이터 공유가 쉽지만, 동기화에 주의해야 한다.

```c
/**
 * @brief 자식 태스크 생성
 *
 * @param[out] TaskIdPtr     생성된 태스크 ID
 * @param[in]  TaskName      태스크 이름
 * @param[in]  FunctionPtr   진입점 함수
 * @param[in]  StackPtr      스택 (NULL = 자동)
 * @param[in]  StackSize     스택 크기
 * @param[in]  Priority      우선순위
 * @param[in]  Flags         플래그
 *
 * @return CFE_SUCCESS       성공
 * @return CFE_ES_ERR_CHILD_TASK_CREATE 실패
 */
CFE_Status_t CFE_ES_CreateChildTask(
    CFE_ES_TaskId_t *TaskIdPtr,
    const char *TaskName,
    CFE_ES_ChildTaskMainFuncPtr_t FunctionPtr,
    CFE_ES_StackPointer_t StackPtr,
    size_t StackSize,
    CFE_ES_TaskPriority_Atom_t Priority,
    uint32 Flags);

/* 사용 예시 */
CFE_ES_TaskId_t ChildTaskId;
CFE_ES_CreateChildTask(&ChildTaskId, "MY_CHILD",
                       MyChildFunction, NULL, 4096, 100, 0);
```

### 3.2 CFE_ES_DeleteChildTask

```c
/**
 * @brief 자식 태스크 삭제
 * @param[in] TaskId  삭제할 태스크 ID
 */
CFE_Status_t CFE_ES_DeleteChildTask(CFE_ES_TaskId_t TaskId);
```

### 3.3 CFE_ES_ExitChildTask

```c
/**
 * @brief 자식 태스크 종료 (자체 호출)
 * @note 자식 태스크 함수 끝에서 호출, 반환하지 않음
 */
void CFE_ES_ExitChildTask(void);
```

---

## 4. 메모리 풀 API

동적 메모리 할당은 비행 소프트웨어에서 신중하게 다루어야 하는 영역이다. ES의 메모리 풀 시스템은 고정 크기 버킷을 사용하여 단편화를 방지하고, 결정론적 할당 시간을 보장한다.

### 4.1 풀 생성

`CFE_ES_PoolCreate()` 함수는 지정된 메모리 영역에 매모리 풀을 생성한다. 풀 생성 시 기본 버킷 크기가 사용되며, `CFE_ES_PoolCreateEx()` 함수를 사용하면 커스텀 버킷 크기와 덼텍스 옵션을 지정할 수 있다.

```c
/**
 * @brief 기본 메모리 풀 생성
 */
CFE_Status_t CFE_ES_PoolCreate(CFE_ES_MemHandle_t *PoolID,
                                void *MemPtr,
                                size_t Size);

/**
 * @brief 확장 메모리 풀 생성 (커스텀 버킷)
 */
CFE_Status_t CFE_ES_PoolCreateEx(CFE_ES_MemHandle_t *PoolID,
                                  void *MemPtr,
                                  size_t Size,
                                  uint16 NumBlockSizes,
                                  const size_t *BlockSizes,
                                  CFE_ES_MemPoolMutex_t UseMutex);

/* 사용 예시 */
uint8 PoolMemory[16384];
CFE_ES_MemHandle_t PoolId;
CFE_ES_PoolCreate(&PoolId, PoolMemory, sizeof(PoolMemory));
```

### 4.2 버퍼 할당/해제

```c
/**
 * @brief 풀에서 버퍼 할당
 * @return 할당된 블록 크기 (양수) 또는 오류 코드 (음수)
 */
int32 CFE_ES_GetPoolBuf(CFE_ES_MemPoolBuf_t *BufPtr,
                        CFE_ES_MemHandle_t PoolID,
                        size_t Size);

/**
 * @brief 버퍼를 풀에 반환
 * @return 반환된 블록 크기 (양수) 또는 오류 코드 (음수)
 */
int32 CFE_ES_PutPoolBuf(CFE_ES_MemHandle_t PoolID,
                        CFE_ES_MemPoolBuf_t BufPtr);

/* 사용 예시 */
CFE_ES_MemPoolBuf_t Buffer;
int32 AllocSize = CFE_ES_GetPoolBuf(&Buffer, PoolId, 100);
if (AllocSize > 0)
{
    /* 버퍼 사용 */
    CFE_ES_PutPoolBuf(PoolId, Buffer);
}
```

### 4.3 풀 통계

```c
/**
 * @brief 풀 통계 조회
 */
CFE_Status_t CFE_ES_GetPoolStats(CFE_ES_MemPoolStats_t *Stats,
                                  CFE_ES_MemHandle_t PoolID);
```

---

## 5. Critical Data Store API

CDS는 Processor Reset을 통해서도 데이터를 보존할 수 있는 특별한 메모리 영역이다. 중요한 상태 정보, 통계 카운터, 캘리브레이션 데이터 등을 리셋 후에도 복원할 수 있다.

### 5.1 CFE_ES_RegisterCDS

CDS 영역을 등록하는 함수이다. 앱 초기화 시 호출하며, 반환 값에 따라 새로 생성된 것인지(CFE_SUCCESS) 또는 기존 영역을 재사용하는 것인지(CFE_ES_CDS_ALREADY_EXISTS) 확인할 수 있다. 기존 영역이 있으면 이전 세션의 데이터를 복원할 수 있다.

```c
/**
 * @brief CDS 영역 등록
 *
 * @param[out] CDSHandlePtr  CDS 핸들 반환
 * @param[in]  BlockSize     필요한 크기
 * @param[in]  Name          블록 이름
 *
 * @return CFE_SUCCESS              새로 등록됨
 * @return CFE_ES_CDS_ALREADY_EXISTS 기존 영역 재사용
 */
CFE_Status_t CFE_ES_RegisterCDS(CFE_ES_CDSHandle_t *CDSHandlePtr,
                                 size_t BlockSize,
                                 const char *Name);
```

### 5.2 CDS 데이터 저장/복원

```c
/**
 * @brief 데이터를 CDS에 저장
 */
CFE_Status_t CFE_ES_CopyToCDS(CFE_ES_CDSHandle_t CDSHandle,
                               const void *DataToCopy);

/**
 * @brief CDS에서 데이터 복원
 */
CFE_Status_t CFE_ES_RestoreFromCDS(void *RestoreToMemory,
                                    CFE_ES_CDSHandle_t CDSHandle);

/* 사용 예시 */
CFE_ES_CDSHandle_t CDSHandle;
MyData_t MyData;

/* 등록 */
CFE_ES_RegisterCDS(&CDSHandle, sizeof(MyData_t), "MyData");

/* 저장 */
CFE_ES_CopyToCDS(CDSHandle, &MyData);

/* 복원 */
CFE_ES_RestoreFromCDS(&MyData, CDSHandle);
```

---

## 6. 리셋 및 시스템 API

시스템 레벨의 제어와 모니터링을 위한 API들이다. 리셋 유형 확인, 시스템 리셋 요청, 저수준 시스템 로그 기록 등의 기능을 제공한다.

### 6.1 리셋 관련

리셋 관련 함수들은 애플리케이션이 현재 리셋 유형을 확인하고 적절한 초기화 전략을 선택할 수 있게 해준다. `CFE_ES_GetResetType()` 함수는 Power-On Reset인지 Processor Reset인지를 반환하며, 이에 따라 CDS 복원 여부를 결정할 수 있다. `CFE_ES_ResetCFE()` 함수는 시스템 리셋을 트리거하며, 이 함수는 반환하지 않는다.

```c
/**
 * @brief 리셋 유형 조회
 * @param[out] ResetSubtypePtr  리셋 부유형 반환 (NULL 가능)
 * @return 리셋 유형 (1=POR, 2=PR)
 */
CFE_Status_t CFE_ES_GetResetType(uint32 *ResetSubtypePtr);

/**
 * @brief cFE 리셋 트리거
 * @param[in] ResetType  리셋 유형
 * @return 반환하지 않음
 */
uint32 CFE_ES_ResetCFE(uint32 ResetType);
```

### 6.2 시스템 로그

```c
/**
 * @brief 시스템 로그에 메시지 기록
 * @param[in] SpecStringPtr  포맷 문자열
 * @return 기록된 문자 수
 */
int32 CFE_ES_WriteToSysLog(const char *SpecStringPtr, ...);

/* 사용 예시 */
CFE_ES_WriteToSysLog("MY_APP: Initialized, version %d.%d\n",
                     MAJOR_VERSION, MINOR_VERSION);
```

---

## 7. 성능 모니터링 API

성능 모니터링 API는 코드 실행 시간을 측정하는 데 사용된다. 측정하고자 하는 코드 구간의 시작에 Entry 마커를, 끝에 Exit 마커를 삽입한다. 수집된 데이터는 지상 도구를 통해 분석되어 태스크 스케줄링, 실행 시간 통계, 타이밍 다이어그램 등을 생성한다.

### 7.1 성능 마커

`CFE_ES_PerfLogEntry()`와 `CFE_ES_PerfLogExit()` 함수는 코드 구간의 시작과 끝을 표시한다. 각 마커는 고유한 ID를 가지며, ES는 이 ID와 함께 타임스탬프를 기록한다. 성능 로깅이 활성화되어 있을 때만 데이터가 기록되므로, 정상 운영 시에는 추가 오버헤드가 최소화된다.

```c
/**
 * @brief 성능 엔트리 마커
 * @param[in] Marker  마커 ID
 */
void CFE_ES_PerfLogEntry(uint32 Marker);

/**
 * @brief 성능 엑시트 마커
 * @param[in] Marker  마커 ID
 */
void CFE_ES_PerfLogExit(uint32 Marker);

/* 사용 예시 */
#define MY_APP_PERF_ID  42

CFE_ES_PerfLogEntry(MY_APP_PERF_ID);
/* 측정할 코드 */
CFE_ES_PerfLogExit(MY_APP_PERF_ID);
```

---

## 8. Generic Counter API

Generic Counter는 애플리케이션이 통계를 수집하고 모니터링할 수 있는 범용 카운터 메커니즘이다. 메시지 수신 횟수, 오류 발생 횟수, 처리 완료 횟수 등 다양한 통계를 추적하는 데 사용할 수 있다. 카운터는 ES에 등록되고 이름으로 조회할 수 있어, 다른 앱이나 지상에서도 접근 가능하다.

```c
/**
 * @brief 카운터 등록
 */
CFE_Status_t CFE_ES_RegisterGenCounter(CFE_ES_CounterId_t *CounterIdPtr,
                                        const char *CounterName);

/**
 * @brief 카운터 증가
 */
CFE_Status_t CFE_ES_IncrementGenCounter(CFE_ES_CounterId_t CounterId);

/**
 * @brief 카운터 값 설정
 */
CFE_Status_t CFE_ES_SetGenCount(CFE_ES_CounterId_t CounterId, uint32 Count);

/**
 * @brief 카운터 값 조회
 */
CFE_Status_t CFE_ES_GetGenCount(CFE_ES_CounterId_t CounterId, uint32 *Count);

/**
 * @brief 이름으로 카운터 ID 조회
 */
CFE_Status_t CFE_ES_GetGenCounterIDByName(CFE_ES_CounterId_t *CounterIdPtr,
                                           const char *CounterName);
```

---

## 9. 기타 유틸리티 API

ES는 앞서 설명한 주요 API 외에도 몇 가지 유틸리티 함수를 제공한다. 데이터 무결성 검사를 위한 CRC 계산, 앱 간 시작 동기화 등의 기능이 포함된다.

### 9.1 CRC 계산

`CFE_ES_CalculateCRC()` 함수는 데이터 블록의 CRC를 계산한다. 이 함수는 테이블 로드, CDS 무결성 검증, 파일 다운로드 확인 등 다양한 곳에서 데이터 손상을 탐지하는 데 사용된다.

```c
/**
 * @brief CRC 계산
 */
uint32 CFE_ES_CalculateCRC(const void *DataPtr,
                           size_t DataLength,
                           uint32 InputCRC,
                           uint32 TypeCRC);
```

### 9.2 라이브러리 초기화 확인

```c
/**
 * @brief 라이브러리 초기화 대기
 * @param[out] LibIdPtr  라이브러리 ID
 * @param[in]  LibName   라이브러리 이름
 * @param[in]  Timeout   타임아웃 (ms)
 */
CFE_Status_t CFE_ES_WaitForStartupSync(uint32 Timeout);
```

---

## 10. API 반환 값 요약

| 반환 값 | 의미 |
|:---|:---|
| `CFE_SUCCESS` | 성공 |
| `CFE_ES_ERR_APP_REGISTER` | 앱 등록 실패 |
| `CFE_ES_ERR_APP_NOT_RUNNING` | 앱 실행 중 아님 |
| `CFE_ES_ERR_RESOURCEID_NOT_VALID` | 잘못된 리소스 ID |
| `CFE_ES_ERR_NAME_NOT_FOUND` | 이름 찾을 수 없음 |
| `CFE_ES_ERR_CHILD_TASK_CREATE` | 자식 태스크 생성 실패 |
| `CFE_ES_ERR_MEM_POOL_CREATE` | 메모리 풀 생성 실패 |
| `CFE_ES_ERR_MEM_BLOCK_SIZE` | 블록 크기 오류 |
| `CFE_ES_CDS_ALREADY_EXISTS` | CDS 이미 존재 (성공) |
| `CFE_ES_ERR_CDS_NOT_AVAILABLE` | CDS 사용 불가 |
| `CFE_ES_CDS_BLOCK_CRC_ERR` | CDS CRC 오류 |

---

## 11. 정리: ES API 요약표

| 카테고리 | 함수 | 용도 |
|:---|:---|:---|
| **앱 생명주기** | `RegisterApp()` | 앱 등록 |
| | `RunLoop()` | 실행 루프 제어 |
| | `ExitApp()` | 앱 종료 |
| | `RestartApp()` | 앱 재시작 |
| **정보 조회** | `GetAppID()` | 앱 ID 조회 |
| | `GetAppInfo()` | 앱 정보 조회 |
| | `GetTaskID()` | 태스크 ID 조회 |
| **자식 태스크** | `CreateChildTask()` | 자식 생성 |
| | `DeleteChildTask()` | 자식 삭제 |
| | `ExitChildTask()` | 자식 종료 |
| **메모리 풀** | `PoolCreate()` | 풀 생성 |
| | `GetPoolBuf()` | 버퍼 할당 |
| | `PutPoolBuf()` | 버퍼 해제 |
| **CDS** | `RegisterCDS()` | CDS 등록 |
| | `CopyToCDS()` | CDS 저장 |
| | `RestoreFromCDS()` | CDS 복원 |
| **시스템** | `GetResetType()` | 리셋 유형 |
| | `WriteToSysLog()` | Syslog 기록 |
| | `PerfLogEntry/Exit()` | 성능 마커 |

---

## 결론

ES API는 cFE 애플리케이션 개발에 필수적인 다양한 기능을 제공한다. 앱 생명주기 관리, 메모리 관리, 데이터 보존, 성능 분석 등 비행 소프트웨어에 필요한 기본 서비스들이 잘 정의된 API를 통해 제공된다.

이것으로 ES 모듈 시리즈를 마친다. 다음 Phase에서는 Software Bus(SB) 모듈을 상세히 살펴볼 것이다.

---

## 참고 문헌

1. NASA, "cFE Application Developer's Guide"
2. NASA cFE GitHub, cfe/modules/es/fsw/inc/cfe_es.h
3. NASA, "ES API Reference"

---

[이전 문서: Phase 2 ES-11: ES 명령 및 텔레메트리](./Phase2_ES_11_ES_명령_및_텔레메트리.md)

[다음 문서: Phase 2 SB-01: Software Bus 모듈 개요](./Phase2_SB_01_SB_모듈_개요.md)
