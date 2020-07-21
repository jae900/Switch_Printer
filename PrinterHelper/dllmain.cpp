/**
	@file		dllmain.cpp
	@brief		DLL 진입점이 정의된 파일
*/
#include "stdafx.h"
#include "Hook.h"
#include "NewFunction.h"
#include <strsafe.h>
#include "Debug.h"

extern HOOKINFO	g_HookInfo[MAX_FUNCTION];
extern WCHAR	g_wszCfgFilePath[MAX_PATH];

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
			GetModuleFileName(hModule, g_wszCfgFilePath, MAX_PATH);
			for (size_t i = wcslen(g_wszCfgFilePath); i > 0; i--) {
				if (g_wszCfgFilePath[i] == L'\\') {
					g_wszCfgFilePath[i + 1] = L'\0';
					StringCchCat(g_wszCfgFilePath, MAX_PATH, L"PrinterHelper.cfg");
					break;
				}
			}
			
			// 후킹
			if (ERROR_NOT_SUPPORTED == HookFunction(L"Gdi32.dll", "CreateDCW", &g_HookInfo[nhCreateDCW], NewCreateDCW)) {
				DEBUG(L"Failed to hook CreateDCW.");

				HookFunction(L"Winspool.drv", "OpenPrinter2W", &g_HookInfo[nhOpenPrinter2W], NewOpenPrinter2W, TRUE);
			}

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
			UnhookFunction(&g_HookInfo[nhCreateDCW]);
		}
		break;
	}
	return TRUE;
}

