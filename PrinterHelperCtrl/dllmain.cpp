/**
	@file		PrinterHelperCtrl\dllmain.cpp
	@brief		PrinterHelperCtrl DLL 엔트리가 존재하는 파일
*/
#include "stdafx.h"
#include <vector>
#include <string>
#include "CProcessCreateNotification.h"
#include "PrinterHelperCtrl.h"
#include "Log.h"
#include <strsafe.h>

using std::vector;
using std::wstring;

extern vector<wstring>		g_exceptedProcess;

extern HMODULE				g_hModule;
extern CProcessCreateNotify	g_classProcessCreateNotify;
extern CLog					*g_pLog;
extern WCHAR				g_wszCurrentDir[MAX_PATH];
/**
	@brief		DLL 엔트리
*/
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			WCHAR	wszLogFilePath[MAX_PATH] = { L'\0', };

			g_hModule = hModule;

			GetModuleFileName(g_hModule, wszLogFilePath, MAX_PATH);
			StringCchCat(wszLogFilePath, MAX_PATH, L".log");

			GetModuleFileName(g_hModule, g_wszCurrentDir, MAX_PATH);

			for (size_t i = wcslen(g_wszCurrentDir); i > 0; i--) {
				if (g_wszCurrentDir[i] == L'\\') {
					g_wszCurrentDir[i] = L'\0';
					break;
				}
			}

			g_pLog = CLog::GetInstance();
			g_pLog->InitLog(wszLogFilePath);
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		{
			CleanupExceptedProcesses();
		}
		break;
	}
	return TRUE;
}

