/**
	@file		Common.h
	@brief		���� ���
*/
#pragma once

#define DEFAULT_PRINTER				L"CutePDF Writer"
#define DEFAULT_PRINTER_PORT		L"CPW4:"

#define INJECTOR_NAME				L"Injector\0"			///< ������ ��ų ���μ����� �̸�
#define SPRINTER_NAME				L"PrinterHelper\0"		///< �����ǵ� DLL�� �̸�

#pragma pack(push, 1)
/**
	@struct _SPRINTER_CONFIG
	@brief	SPRINTER ����� ���� ����ü
*/
typedef struct _SPRINTER_CONFIG {
	BOOL		bRun;					///< ���� ����(TRUE:����, FALSE:����)
	ULONG		ulReserved1;			///< �߰� ������ ���� �ӽ� �����
	WCHAR		wszPrinterName[64];		///< ����� ������ ��ġ �̸�
	WCHAR		wszPortName[32];		///< ��Ʈ �̸�
	WCHAR		wszOutputDir[MAX_PATH];	///< ��� ���丮
	WCHAR		wszFileNameFormat[32];	///< ���� ����
} SPRINTER_CONFIG, *PSPRINTER_CONFIG;

#pragma pack(pop)