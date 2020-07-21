/**
	@file		NewFunction.cpp
	@brief		Trampoline 후킹을 위한 함수가 정의된 파일
*/
#include "stdafx.h"
#include "NewFunction.h"
#include "Hook.h"
#include "..\Common\Common.h"
#include "Config.h"
#include <Winspool.h>
#include <strsafe.h>
#include "Debug.h"

extern "C" int WINAPI GetEDI();
extern "C" void WINAPI SetEDI(int value);

extern HOOKINFO	g_HookInfo[MAX_FUNCTION];
extern HMODULE	g_hMod;

SPRINTER_CONFIG	g_Config = { 0, };				///< 설정 정보

/**
	@brief		DC를 생성하는 루틴의 Trampoline 함수
	@param[in]	pwszDriver		드라이버 이름
	@param[in]	pwszDevice		장치 이름
	@param[in]	pszPort			포트 이름
	@param[in]	pdm				DEVMODE 포인터
	@return		HDC				생성된 DC 핸들
	@return		NULL			실패
	@date		2020.07.14
	@author		Kevin
*/
HDC
WINAPI
NewCreateDCW(__in LPCWSTR        pwszDriver,
	__in LPCWSTR        pwszDevice,
	__in LPCWSTR        pszPort,
	__in const DEVMODEW *pdm)
{
	LPCWSTR		chDevice = pwszDevice;
	LPCWSTR		chPort = pszPort;

	do {
		// 프린터 DC 확인
		if (pwszDriver == NULL
			|| (_wcsicmp(pwszDriver, L"winspool") != 0))
		{
			break;
		}

		GetConfiguration(&g_Config);

		// 설정 확인
		if (!g_Config.bRun) {
			break;
		}

#ifndef _DEBUG
		if (pwszDevice) {
			WCHAR	wszDeviceName[64] = { L'\0', };

			StringCchCopy(wszDeviceName, 64, pwszDevice);
			_wcslwr(wszDeviceName);

			// PDF, XPS 등의 파일로 출력하는 프린터일 경우 예외 
			if (wcsstr(wszDeviceName, L"pdf") || wcsstr(wszDeviceName, L"xps")) {
				if (pszPort) {
					ZeroMemory(wszDeviceName, 128);
					StringCchCopy(wszDeviceName, 32, pszPort);
					_wcslwr(wszDeviceName);
					if (wcsstr(L"LPT1:LPT2:LPT3:COM1:COM2:COM3:COM4:COM5:", wszDeviceName) == 0) {
						DEBUG(L"This is file printer : %s", pwszDevice);
						break;
					}
				}
				else {
					DEBUG(L"This is file printer : %s", pwszDevice);
					break;
				}
			}
		}
#endif

		DEBUG(L"Driver : %s\nDevice : %s\nPort : %s\nDeviceName : %s", pwszDriver, pwszDevice, pszPort, pdm->dmDeviceName);

		if (pdm) {
			StringCchCopy((LPWSTR)pdm->dmDeviceName, CCHDEVICENAME, g_Config.wszPrinterName);
		}

		if (g_Config.wszOutputDir[0] != L'\0') {
			SetBypassSaveAs(TRUE);
		}

		chDevice = g_Config.wszPrinterName;
		chPort = g_Config.wszPortName;

		DEBUG(L"Change, chDevice : %s\nchPort : %s", chDevice, chPort);
	} while (FALSE);

	DEBUG(L"%s, %s, %s," POINTER, pwszDriver, pwszDevice, pszPort, pdm);

	return ((FN_CREATEDCW)g_HookInfo[nhCreateDCW].pfnOrgFunction)(pwszDriver, chDevice, chPort, pdm);
}

/**
	@brief		OpenPrinter2W의 후킹 함수
	@date		2020.07.21
	@author		Kevin
*/
BOOL
WINAPI
NewOpenPrinter2W(__in  LPWSTR				pPrinterName,
				 __out LPHANDLE				phPrinter,
				 __in  LPPRINTER_DEFAULTSW	pDefault,
				 __in PPRINTER_OPTIONSW		pOptions)
{
	LPWSTR		pNewPrinterName = pPrinterName;

	do {
		if (pPrinterName == NULL) {
			break;
		}

		GetConfiguration(&g_Config);

		// 설정 확인
		if (!g_Config.bRun) {
			break;
		}

		if (g_Config.wszOutputDir[0] != L'\0') {
			SetBypassSaveAs(TRUE);
		}

		pNewPrinterName = g_Config.wszPrinterName;
		DEBUG(L"Change : %s -> %s", pPrinterName, pNewPrinterName);
	} while (FALSE);

	return ((FN_OPENPRINTER2W)g_HookInfo[nhOpenPrinter2W].pfnOrgFunction)(pNewPrinterName, phPrinter, pDefault, pOptions);
}

/**
	@brief		DLL을 로드하는 LoadLibaryExW의 새로운 함수 정의
	@date		2020.07.14
	@author		Kevin
	@see		LoadLibraryExW 참조
*/
HMODULE
WINAPI
NewLoadLibraryExW(__in       LPCWSTR lpFileName,
				  __reserved HANDLE  hFile,
				  __in       DWORD   dwFlags)
{
	HMODULE		hMod = NULL;

	hMod = ((FN_LOADLIBRARYEXW)g_HookInfo[nhLoadLibraryExW].pfnOrgFunction)(lpFileName, hFile, dwFlags);

	//DEBUG(L"LoadLibrary : %s", lpFileName);

	size_t nLength = wcslen(lpFileName);

										//		  123456789012
	if ((nLength >= 12)
		&& _wcsnicmp(&lpFileName[nLength - 12], L"xpsprint.dll", 12) == 0)
	{
		PVOID fnStartXpsPrintJob = GetProcAddress(hMod, "StartXpsPrintJob");

		DEBUG(L"Hook : StartXpsPrintJob : " POINTER, fnStartXpsPrintJob);

		if (fnStartXpsPrintJob) {
			HookFunction(hMod, fnStartXpsPrintJob, &g_HookInfo[nhStartXpsPrintJob], NewStartXpsPrintJob);
		}
		else {
			DWORD dwRet = GetLastError();
			DEBUG(L"Failed to get procedure(%d).", dwRet);
		}
	}

	return hMod;
}

/**
	@brief		새로 정의된 XPS 프린터 시작 함수
	@date		2020.07.14
	@author		Kevin
	@see		StartXpsPrintJob 참조
*/
HRESULT
WINAPI
NewStartXpsPrintJob(__in  const LPCWSTR printerName,
					__in  const LPCWSTR jobName,
					__in  const LPCWSTR outputFileName,
					__in HANDLE progressEvent,
					__in HANDLE completionEvent,
					__in UINT8 *printablePagesOn,
					__in UINT32 printablePagesOnCount,
					__out PVOID *xpsPrintJob,
					__out PVOID *documentStream,
					__out PVOID *printTicketStream)
{
	LPCWSTR		chPrinterName = printerName;
	LPCWSTR		chOutputFileName = outputFileName;

	do {
		GetConfiguration(&g_Config);

		if (!g_Config.bRun) {
			break;
		}

#ifndef _DEBUG
		if (printerName) {
			WCHAR	wszDeviceName[64] = { L'\0', };

			StringCchCopy(wszDeviceName, 64, printerName);
			_wcslwr(wszDeviceName);

			// PDF, XPS 등의 파일로 출력하는 프린터일 경우 예외 
			if (wcsstr(wszDeviceName, L"pdf") || wcsstr(wszDeviceName, L"xps")) {
				if (outputFileName) {
					ZeroMemory(wszDeviceName, 128);
					StringCchCopy(wszDeviceName, 32, outputFileName);
					_wcslwr(wszDeviceName);
					if (wcsstr(L"LPT1:LPT2:LPT3:COM1:COM2:COM3:COM4:COM5:", wszDeviceName) == 0) {
						DEBUG(L"This is file printer : %s", outputFileName);
						break;
					}
				}
				else {
					DEBUG(L"This is file printer : %s", outputFileName);
					break;
				}
			}
		}
#endif

		DEBUG(L"printName : %s\njobName : %s\noutputFileName : %s", printerName, jobName, outputFileName);
		
		chPrinterName = g_Config.wszPrinterName;
		chOutputFileName = g_Config.wszPortName;

		if (g_Config.wszOutputDir[0] != L'\0') {
			SetBypassSaveAs(TRUE);
		}

		DEBUG(L"printName : %s\noutputFileName : %s", chPrinterName, chOutputFileName);

	} while (FALSE);

	return ((FN_STARTXPSPRINTJOB)g_HookInfo[nhStartXpsPrintJob].pfnOrgFunction)(chPrinterName, jobName, chOutputFileName, progressEvent, completionEvent, printablePagesOn, printablePagesOnCount, xpsPrintJob, documentStream, printTicketStream);
}