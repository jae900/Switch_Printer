/**
	@file		PrinterHelperCtrl.h
	@brief		SPrinter 제어 모듈의 Export된 함수들이 선언된 파일
*/
#ifdef PRINTERHELPERCTRL_EXPORTS
#define PRINTERHELPERCTRL_API extern "C" __declspec(dllexport)
#else
#define PRINTERHELPERCTRL_API extern "C" __declspec(dllimport)
#endif

/**
	@brief		현재 선택된 파일 프린터 정보를 반환한다.
	@param[out]	wszPrinterName		프린터 이름
	@param[out]	wszPort				포트
	@return		ERROR_SUCCESS		성공
	@return		DWORD				에러코드
*/
PRINTERHELPERCTRL_API
DWORD
GetSelectedFilePrinter(__out PWCHAR wszPrinterName, __out PWCHAR wszPort, __in BOOL bInit);

/**
	@brief		출력할 프린터를 지정
	@parma[in]	wszPrinterName		프린터 이름
	@param[in]	wszPort				포트
	@return		ERROR_SUCCESS		성공
	@return		DWORD				에러코드
*/
PRINTERHELPERCTRL_API
VOID
PrinterRedirection(__in PWCHAR wszPrinterName, __in PWCHAR wszPort);

/**
	@brief		CFG 파일을 참고하여 파일 프린터를 설치한다.
*/
PRINTERHELPERCTRL_API
VOID
InstallFilePrinter();

/**
	@brief		SPrinter 모듈의 설정을 수정한다.
	@param[in]	bRun			실행 여부(On/Off)
	@param[in]	lpDestDir		출력 파일의 위치
	@return		ERROR_SUCCESS	성공
	@return		DWORD			에러코드
*/
PRINTERHELPERCTRL_API
DWORD
SetConfig(__in BOOL bRun, __in PWCHAR lpDestDir);

/**
	@brief		SPrinter 모듈의 현재 설정을 가져한다.
	@param[out]	wszOutputDir		출력 파일의 위치
	@param[out]	wszFileNameFormat	파일 이름 포멧
	@return		ERROR_SUCCESS	성공
	@return		DWORD			에러코드
*/
PRINTERHELPERCTRL_API
DWORD
GetConfig(__out PWCHAR wszOutputDir = NULL, __out PWCHAR wszFileNameFormat = NULL);

/**
	@brief		지정된 프로세스에 SPrinter 모듈을 Attach 한다.
	@param[in]	dwProcessId		프로세스 ID
	@return		ERROR_SUCCESS	성공
	@return		DWORD			에러코드
*/
PRINTERHELPERCTRL_API
DWORD
ManualAttach(__in DWORD dwProcessId);

/**
	@brief		자동으로 프로세스에 Attach 하도록 설정한다.
	@param[in]	bAuto			자동 여부(TRUE:자동, FALSE:수동)
	@return		ERROR_SUCCESS	성공
	@return		DWORD			에러코드
*/
PRINTERHELPERCTRL_API
DWORD
AutoAttach(__in BOOL bAuto);

/**
	@brief		자동 Attach 활성화시 Attach 시키지 않을 프로세스를 등록한다.
	@param[in]	lpProcessPath		프로세스 경로
*/
PRINTERHELPERCTRL_API
VOID
InsertExceptedProcess(__in LPWSTR lpProcessPath);

/**
	@brief		자동 Attach 활성화시 Attach 시키지 않을 프로세스 목록에서 프로세스를 제거한다.
	@param[in]	lpProcessPath		프로세스 경로
*/
PRINTERHELPERCTRL_API
VOID
RemoveExceptedProcess(__in LPWSTR lpProcessPath);

/**
	@brief		자동 Attach 활성화시 Attach 시키지 않을 프로세스 목록을 모두 제거한다.
	@param[in]	lpProcessPath		프로세스 경로
*/
PRINTERHELPERCTRL_API
VOID
CleanupExceptedProcesses();

typedef DWORD (*FN_GetSelectedFilePrinter)(__out PWCHAR wszPrinterName, __out PWCHAR wszPort, __in BOOL bInit);
typedef VOID (*FN_PrinterRedirection)(__in PWCHAR wszPrinterName, __in PWCHAR wszPort);
typedef DWORD (*FN_SetConfig)(__in BOOL bRun, __in PWCHAR lpDestDir);
typedef DWORD (*FN_GetConfig)(__out PWCHAR wszOutputDir, __out PWCHAR wszFileNameFormat);
typedef DWORD (*FN_ManualAttach)(__in DWORD dwProcessId);
typedef DWORD (*FN_AutoAttach)(__in BOOL bAuto);
typedef VOID (*FN_InsertExceptedProcess)(__in LPWSTR lpProcessPath);
typedef VOID (*FN_RemoveExceptedProcess)(__in LPWSTR lpProcessPath);
typedef VOID (*FN_CleanupExceptedProcesses)();