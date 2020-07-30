/**
	@file		Hook.h
	@brief		후킹과 관련된 상수, 구조체, 함수들이 선언된 파일
*/
#pragma once

#ifdef _AMD64_
#define DECODETYPE		Decode64Bits
#define JUMP_CODE		12
#define POINTER			L"0x%016I64x"
#else
#define DECODETYPE		Decode32Bits
#define JUMP_CODE		5
#define POINTER			L"0x%08x"
#endif

#define MAX_FUNCTION	5

/**
	@brief		_HOOKINFO의 인덱스
	@date		2020.07.13
	@author		Kevin
*/
typedef enum _HOOKINDEX {
	nhLoadLibraryExW,		///< LoadLibrary, 동적 DLL의 Export 함수 StartXpsPrintJob을 로딩 시점에 후킹하기 위한 LoadLibrary 인덱스
	nhCreateDCW,			///< GDI 프린터 후킹을 위한 CreateDC API 후킹
	nhStartXpsPrintJob,		///< XPSPRINT.DLL을 이용하여 출력 시 해당 API 후킹
	nhOpenPrinterW,
	nhOpenPrinter2W,		///< Windows 7 x64에서 CreateDC의 후킹이 불가하여 OpenPrinter2W 사용
} HOOKINDEX;

/**
	@brief		후킹 정보가 저장된 구조체
	@date		2020.07.13
	@author		Kevin
*/
typedef struct _HOOKINFO {
	PUCHAR		pfnOrgFunction;		///< 복사된 명령어 코드
	PVOID		pfnHookedFunction;	///< 후킹된 함수 주소
} HOOKINFO, *PHOOKINFO;

DWORD
HookFunction(__in HMODULE hMod, __in PVOID pfnOrgFunction, __in PHOOKINFO pHookInfo, __in PVOID pfnNewFunction, __in BOOL bDeepFindJmp = FALSE);

DWORD
HookFunction(__in LPCWSTR lpModuleName, __in LPCSTR lpFunctionName, __in PHOOKINFO pHookInfo, __in PVOID pNewFunction, __in BOOL bDeepFindJmp = FALSE);

DWORD
UnhookFunction(__in PHOOKINFO	pHookInfo);
