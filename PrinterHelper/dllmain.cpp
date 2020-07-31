/**
	@file		dllmain.cpp
	@brief		DLL 진입점이 정의된 파일
*/
#include "stdafx.h"
#include "Hook.h"
#include "NewFunction.h"
#include "..\Common\Common.h"
#include "Config.h"
#include <strsafe.h>
#include "Debug.h"

extern SPRINTER_CONFIG	g_Config;
extern HOOKINFO	g_HookInfo[MAX_FUNCTION];
extern WCHAR	g_wszCfgFilePath[MAX_PATH];

CRITICAL_SECTION	cs;

/**
	@brief		DLL 엔트리
*/
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			InitializeCriticalSection(&cs);

			GetModuleFileName(hModule, g_wszCfgFilePath, MAX_PATH);
			for (size_t i = wcslen(g_wszCfgFilePath); i > 0; i--) {
				if (g_wszCfgFilePath[i] == L'\\') {
					g_wszCfgFilePath[i + 1] = L'\0';
					StringCchCat(g_wszCfgFilePath, MAX_PATH, L"PrinterHelper.cfg");
					break;
				}
			}
			
			GetConfiguration(&g_Config);

			// FarPoint를 사용하는 출력 프로그램의 division by zero 오류등을 해결하기 위해
			// 기본 프린터를 설정된 프린터로 설정한다.
			WCHAR	wszPrinterName[64];
			DWORD	dwLength = 64;

			// 1. 동작 중인지 확인
			if (g_Config.bRun) {
				// 2. 현재의 프린터를 가져온다.
				if (GetDefaultPrinter(wszPrinterName, &dwLength)
					&& (_wcsicmp(wszPrinterName, g_Config.wszPrinterName) != 0))
				{
					SetDefaultPrinter(g_Config.wszPrinterName);
					WritePrivateProfileString(L"Default", L"Printer", wszPrinterName, g_wszCfgFilePath);
				}
			}

			HookFunction(L"Winspool.drv", "OpenPrinter2W", &g_HookInfo[nhOpenPrinter2W], NewOpenPrinter2W, TRUE);
			HookFunction(L"XpsPrint.dll", "StartXpsPrintJob", &g_HookInfo[nhStartXpsPrintJob], NewStartXpsPrintJob);
			HookFunction(L"kernel32.dll", "LoadLibraryExW", &g_HookInfo[nhLoadLibraryExW], NewLoadLibraryExW, TRUE);
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			// 언후킹
			UnhookFunction(&g_HookInfo[nhLoadLibraryExW]);
			UnhookFunction(&g_HookInfo[nhStartXpsPrintJob]);
			UnhookFunction(&g_HookInfo[nhOpenPrinter2W]);

			DeleteCriticalSection(&cs);
		}
		break;
	}
	return TRUE;
}

