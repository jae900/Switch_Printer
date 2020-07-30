/**
	@file		dllmain.cpp
	@brief		DLL �������� ���ǵ� ����
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

/**
	@brief		DLL ��Ʈ��
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
			
			GetConfiguration(&g_Config);

			// FarPoint�� ����ϴ� ��� ���α׷��� division by zero �������� �ذ��ϱ� ����
			// �⺻ �����͸� ������ �����ͷ� �����Ѵ�.
			WCHAR	wszPrinterName[64];
			DWORD	dwLength = 64;

			// 1. ���� ������ Ȯ��
			if (g_Config.bRun) {
				// 2. ������ �����͸� �����´�.
				if (GetDefaultPrinter(wszPrinterName, &dwLength)
					&& (wcsicmp(wszPrinterName, g_Config.wszPrinterName) != 0))
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
			// ����ŷ
			UnhookFunction(&g_HookInfo[nhLoadLibraryExW]);
			UnhookFunction(&g_HookInfo[nhStartXpsPrintJob]);
			UnhookFunction(&g_HookInfo[nhCreateDCW]);
		}
		break;
	}
	return TRUE;
}

