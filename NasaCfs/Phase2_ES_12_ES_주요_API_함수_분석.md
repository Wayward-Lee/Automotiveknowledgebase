# Phase 2 ES-12: ES 주요 API 함수 분석

## 서론

지금까지 ES 모듈의 다양한 기능을 살펴보았다. 본 문서에서는 ES가 제공하는 주요 API 함수들을 카테고리별로 정리하고, 각 함수의 용도, 파라미터, 반환 값, 사용 예시를 종합적으로 분석한다.

이 문서는 ES API의 레퍼런스 가이드로 활용할 수 있다.

---

## 1. 애플리케이션 생명주기 API

### 1.1 CFE_ES_RegisterApp

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

### 2.1 앱 ID 조회

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

### 3.1 CFE_ES_CreateChildTask

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

### 4.1 풀 생성

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

### 5.1 CFE_ES_RegisterCDS

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

### 6.1 리셋 관련

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

### 7.1 성능 마커

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

### 9.1 CRC 계산

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
