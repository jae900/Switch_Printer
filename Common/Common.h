/**
	@file		Common.h
	@brief		공통 상수
*/
#pragma once

#define DEFAULT_PRINTER				L"CutePDF Writer"
#define DEFAULT_PRINTER_PORT		L"CPW4:"

#define INJECTOR_NAME				L"Injector\0"			///< 인젝션 시킬 프로세스의 이름
#define SPRINTER_NAME				L"PrinterHelper\0"		///< 인젝션될 DLL의 이름

#pragma pack(push, 1)
/**
	@struct _SPRINTER_CONFIG
	@brief	SPRINTER 모듈의 설정 구조체
*/
typedef struct _SPRINTER_CONFIG {
	BOOL		bRun;					///< 실행 여부(TRUE:실행, FALSE:중지)
	ULONG		ulReserved1;			///< 추가 설정을 위한 임시 저장소
	WCHAR		wszPrinterName[64];		///< 출력할 프린터 장치 이름
	WCHAR		wszPortName[32];		///< 포트 이름
	WCHAR		wszOutputDir[MAX_PATH];	///< 출력 디렉토리
	WCHAR		wszFileNameFormat[32];	///< 파일 포멧
} SPRINTER_CONFIG, *PSPRINTER_CONFIG;

#pragma pack(pop)