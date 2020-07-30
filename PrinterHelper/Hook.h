/**
	@file		Hook.h
	@brief		��ŷ�� ���õ� ���, ����ü, �Լ����� ����� ����
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
	@brief		_HOOKINFO�� �ε���
	@date		2020.07.13
	@author		Kevin
*/
typedef enum _HOOKINDEX {
	nhLoadLibraryExW,		///< LoadLibrary, ���� DLL�� Export �Լ� StartXpsPrintJob�� �ε� ������ ��ŷ�ϱ� ���� LoadLibrary �ε���
	nhCreateDCW,			///< GDI ������ ��ŷ�� ���� CreateDC API ��ŷ
	nhStartXpsPrintJob,		///< XPSPRINT.DLL�� �̿��Ͽ� ��� �� �ش� API ��ŷ
	nhOpenPrinterW,
	nhOpenPrinter2W,		///< Windows 7 x64���� CreateDC�� ��ŷ�� �Ұ��Ͽ� OpenPrinter2W ���
} HOOKINDEX;

/**
	@brief		��ŷ ������ ����� ����ü
	@date		2020.07.13
	@author		Kevin
*/
typedef struct _HOOKINFO {
	PUCHAR		pfnOrgFunction;		///< ����� ��ɾ� �ڵ�
	PVOID		pfnHookedFunction;	///< ��ŷ�� �Լ� �ּ�
} HOOKINFO, *PHOOKINFO;

DWORD
HookFunction(__in HMODULE hMod, __in PVOID pfnOrgFunction, __in PHOOKINFO pHookInfo, __in PVOID pfnNewFunction, __in BOOL bDeepFindJmp = FALSE);

DWORD
HookFunction(__in LPCWSTR lpModuleName, __in LPCSTR lpFunctionName, __in PHOOKINFO pHookInfo, __in PVOID pNewFunction, __in BOOL bDeepFindJmp = FALSE);

DWORD
UnhookFunction(__in PHOOKINFO	pHookInfo);
