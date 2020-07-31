/**
	@file		PrinterHelperCtrl.cpp
	@brief		SPrinter 후킹 모듈을 제어하는 루틴이 정의된 파일
	@note		https://www.cutepdf.com/products/sdk/pdfwriter2.asp BypassSaveAs, OutputFile
*/
#include "stdafx.h"
#include <TlHelp32.h>
#include <Winspool.h>
#include "PrinterHelperCtrl.h"
#include "..\Common\Common.h"
#include <vector>
#include <string>
#include <strsafe.h>
#include "CProcessCreateNotification.h"
#include "Log.h"

using std::vector;
using std::wstring;

WCHAR					g_wszCurrentDir[MAX_PATH] = { 0, };	///< 현재 디렉토리
HMODULE					g_hModule = NULL;					///< 현재 모듈 핸들
CProcessCreateNotify	g_classProcessCreateNotify;			///< 프로세스 생성 알림 클래스

vector<wstring>			g_exceptedProcess;					///< 예외 프로세스 저장소	

CLog					*g_pLog = NULL;						///< 파일 로그 기록 인스턴스

SPRINTER_CONFIG			g_sPrinterConfig = { 0, };			///< 프린터 설정
BOOL					g_bInstalling = FALSE;				///< 프린터가 설치되는중일 경우

HANDLE					g_hChangePrinter = NULL;
BOOL					g_bChangePrinter = FALSE;

/**
	@brief		프로세스 실행
	@param[in]	wszCommandLine		Command Line
	@date		2020.07.16
	@author		Kevin
*/
DWORD
Run(__in PWCHAR wszCommandLine)
{
	DWORD				dwRet = ERROR_SUCCESS;
	STARTUPINFO			si = { 0, };
	PROCESS_INFORMATION	pi = { 0, };

	do {
		if (FALSE == CreateProcess(NULL, wszCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
			dwRet = GetLastError();
			WRITELOG(L"Failed to create process(%d:%s).", dwRet, wszCommandLine);
			break;
		}

		DWORD dwWait = WaitForSingleObject(pi.hProcess, 30000);

		if (si.hStdError) {
			CloseHandle(si.hStdError);
		}

		if (si.hStdInput) {
			CloseHandle(si.hStdInput);
		}

		if (si.hStdOutput) {
			CloseHandle(si.hStdOutput);
		}

		// 핸들 리소스 제거
		if (pi.hProcess) {
			if (pi.hThread) {
				CloseHandle(pi.hThread);
			}

			CloseHandle(pi.hProcess);
		}
	} while (FALSE);

	return dwRet;
}

/**
	@brief		파일 프린터를 설치하는 스레드
	@date		2020.07.16
	@author		Kevin
*/
DWORD
WINAPI
InstallFilePrinter(__in LPVOID lpParam)
{
	BOOL	*pInstalling = (BOOL*)lpParam;
	DWORD	dwRet = NO_ERROR;
	WCHAR	wszCfgFile[MAX_PATH] = { 0, };
	WCHAR	wszFileName[MAX_PATH];
	WCHAR	wszCommandLine[520] = { L'\0', };
	HKEY	hKey = NULL;

	do {
		StringCchPrintf(wszCfgFile, MAX_PATH, L"%s\\PrinterHelper.cfg", g_wszCurrentDir);

		// GhostScript 설치
		ZeroMemory(wszFileName, MAX_PATH * sizeof(WCHAR));
		dwRet = GetPrivateProfileString(L"Install", L"ps2pdf", L"", wszFileName, MAX_PATH, wszCfgFile);
		if ((dwRet > 0)
			&& (wszFileName[0] != L'\0'))
		{
			StringCchPrintf(wszCommandLine, 520, L"%s\\%s", g_wszCurrentDir, wszFileName);
			Run(wszCommandLine);
		}

		// 파일 프린터 설치
		ZeroMemory(wszFileName, MAX_PATH * sizeof(WCHAR));
		dwRet = GetPrivateProfileString(L"Install", L"Printer", L"", wszFileName, MAX_PATH, wszCfgFile);
		if ((dwRet == 0)
			|| (wszFileName[0] == L'\0'))
		{
			dwRet = GetLastError();
			WRITELOG(L"Failed to get printer install script(%d:%s).", dwRet, wszFileName);
			break;
		}

		StringCchPrintf(wszCommandLine, 520, L"%s\\%s", g_wszCurrentDir, wszFileName);
		Run(wszCommandLine);

		WRITELOG(L"Complete install File Printer.");
		
		// 설치된 7-Zip 제거
		if (hKey == NULL) {
			dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\7-Zip", 0, KEY_READ, &hKey);
			if (dwRet != 0) {
				WRITELOG(L"Failed to query key for uninstall 7-Zip(%d).", dwRet);
				break;
			}

			DWORD		dwType = REG_SZ;
			DWORD		dwLength = MAX_PATH * 2;

			dwRet = RegQueryValueEx(hKey, L"Path", NULL, &dwType, (LPBYTE)wszFileName, &dwLength);
			if (dwRet != 0) {
				WRITELOG(L"Failed to query value(7-Zip Path) for uninstall(%d).", dwRet);
				break;
			}

			StringCchPrintf(wszCommandLine, 520, L"%sUninstall.exe /S", wszFileName);
			Run(wszCommandLine);
		}
	} while (FALSE);

	if (hKey) {
		RegCloseKey(hKey);
	}

	*pInstalling = FALSE;

	return dwRet;
}

/**
	@brief		현재 선택된 파일 프린터 정보를 반환한다.
	@param[out]	wszPrinterName		프린터 이름
	@param[out]	wszPort				포트
	@return		ERROR_SUCCESS		성공
	@return		DWORD				에러코드
	@date		2020.07.16
	@author		Kevin
*/
PRINTERHELPERCTRL_API
DWORD
GetSelectedFilePrinter(__out PWCHAR wszPrinterName, __out PWCHAR wszPort, __in BOOL bInit)
{
	DWORD			dwRet = ERROR_SUCCESS;
	PPRINTER_INFO_2	pPrinterInfo = NULL;

	do {
		if (bInit) {
			ZeroMemory(g_sPrinterConfig.wszPrinterName, 64);
			ZeroMemory(g_sPrinterConfig.wszPortName, 32);
		}
		else if (g_sPrinterConfig.wszPrinterName[0] == L'\0') {
			GetConfig();
		}

		if (g_sPrinterConfig.wszPrinterName[0] != L'\0') {
			break;
		}		

		// 프린터 드라이버가 설치 중일 경우
		if (g_bInstalling) {
			dwRet = ERROR_NOT_READY;
			break;
		}

		DWORD		dwNeeded = 0;
		DWORD		dwReturn;

		if (FALSE == EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, NULL, 0, &dwNeeded, &dwReturn)) {
			dwRet = GetLastError();
			if (dwRet != ERROR_INSUFFICIENT_BUFFER) {
				WRITELOG(L"Failed to get buffer size for enumerating local printers(%d).", dwRet);
				break;
			}			
		}

		dwRet = ERROR_SUCCESS;

		pPrinterInfo = (PPRINTER_INFO_2)HeapAlloc(GetProcessHeap(), 0, dwNeeded);
		if (pPrinterInfo == NULL) {
			dwRet = ERROR_INSUFFICIENT_BUFFER;
			WRITELOG(L"Failed to allocate pool.");
			break;
		}

		if (FALSE == EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, (LPBYTE)pPrinterInfo, dwNeeded, &dwNeeded, &dwReturn)) {
			dwRet = GetLastError();
			WRITELOG(L"Failed to enumerate local printers(%d).", dwRet);
			break;
		}

		DWORD		n3rdPartyPDF = -1;

		for (DWORD i = 0; i < dwReturn; i++) {
			WRITELOG(L"Printer : %s", pPrinterInfo[i].pPrinterName);

			if (wcsstr(pPrinterInfo[i].pPrinterName, L"PDF") != 0) {
				n3rdPartyPDF = i;
			}
			else if (wcscmp(pPrinterInfo[i].pPrinterName, DEFAULT_PRINTER) == 0) {
				StringCchCopy(g_sPrinterConfig.wszPrinterName, 64, pPrinterInfo[i].pPrinterName);
				StringCchCopy(g_sPrinterConfig.wszPortName, 32, pPrinterInfo[i].pPortName);
				break;
			}
		}

		// PDF 로 출력하는 다른 프린터가 존재할 경우 해당 프린터 사용
		if ((g_sPrinterConfig.wszPrinterName[0] == 0)
			&& (n3rdPartyPDF != -1))
		{
			StringCchCopy(g_sPrinterConfig.wszPrinterName, 64, pPrinterInfo[n3rdPartyPDF].pPrinterName);
			StringCchCopy(g_sPrinterConfig.wszPortName, 32, pPrinterInfo[n3rdPartyPDF].pPortName);
		}

		// 복사 후 반환
		if (g_sPrinterConfig.wszPrinterName[0] != 0) {
			break;
		}

		// 설치
		g_bInstalling = TRUE;

		HANDLE hThread = CreateThread(NULL, 0, InstallFilePrinter, &g_bInstalling, 0, NULL);
		if (hThread) {
			CloseHandle(hThread);
		}
		else {
			g_bInstalling = FALSE;
			dwRet = GetLastError();
			WRITELOG(L"Failed to create installing thread for file printer(%d).", dwRet);
		}
	} while (FALSE);

	if (g_sPrinterConfig.wszPrinterName[0] != L'\0') {
		WCHAR		wszCfgFile[MAX_PATH] = { L'\0', };

		StringCchPrintf(wszCfgFile, MAX_PATH, L"%s\\PrinterHelper.cfg", g_wszCurrentDir);
		WritePrivateProfileStruct(L"PrinterHelper", L"Config", &g_sPrinterConfig, sizeof(SPRINTER_CONFIG), wszCfgFile);

		WRITELOG(L"before Printer : %s\r\nPort : %s", g_sPrinterConfig.wszPrinterName, g_sPrinterConfig.wszPortName);

		StringCchCopy(wszPrinterName, 64, g_sPrinterConfig.wszPrinterName);
		StringCchCopy(wszPort, 32, g_sPrinterConfig.wszPortName);

		WRITELOG(L"after Printer : %s\r\nPort : %s", wszPrinterName, wszPort);
	}

	if (pPrinterInfo) {
		HeapFree(GetProcessHeap(), 0, pPrinterInfo);
	}

	return dwRet;
}

/**
	@brief		프린터가 존재하는지 확인한다.
	@param[in]	wszPrinterName		프린터 이름
	@return		TRUE				성공
	@return		FALSE				실패
	@date		2020.07.18
	@author		Kevin
*/
BOOL
IsExistPrinter(__in PWCHAR wszPrinterName)
{
	DWORD			dwRet = ERROR_SUCCESS;
	BOOL			bRet = FALSE;
	PPRINTER_INFO_1	printerInfo = NULL;

	do {
		DWORD		dwNeeded = 0;
		DWORD		dwReturn;

		if (FALSE == EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 1, NULL, 0, &dwNeeded, &dwReturn)) {
			dwRet = GetLastError();
			if (dwRet != ERROR_INSUFFICIENT_BUFFER) {
				WRITELOG(L"Failed to get buffer size for enumerating local printers(%d).", dwRet);
				break;
			}
		}

		dwRet = ERROR_SUCCESS;

		printerInfo = (PPRINTER_INFO_1)HeapAlloc(GetProcessHeap(), 0, dwNeeded);
		if (printerInfo == NULL) {
			dwRet = ERROR_INSUFFICIENT_BUFFER;
			WRITELOG(L"Failed to allocate pool.");
			break;
		}

		if (FALSE == EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 1, (LPBYTE)printerInfo, dwNeeded, &dwNeeded, &dwReturn)) {
			dwRet = GetLastError();
			WRITELOG(L"Failed to enumerate local printers(%d).", dwRet);
			break;
		}

		for (DWORD i = 0; i < dwReturn; i++) {
			if (wcscmp(printerInfo[i].pName, wszPrinterName) == 0) {
				bRet = TRUE;
				break;
			}
		}
	} while (FALSE);

	if (printerInfo) {
		HeapFree(GetProcessHeap(), 0, printerInfo);
	}

	return bRet;
}

/**
	@brief		출력할 프린터를 지정
	@parma[in]	wszPrinterName		프린터 이름
	@param[in]	wszPort				포트
	@return		ERROR_SUCCESS		성공
	@return		DWORD				에러코드
	@date		2020.07.16
	@author		Kevin
*/
PRINTERHELPERCTRL_API
VOID
PrinterRedirection(__in PWCHAR wszPrinterName, __in PWCHAR wszPort)
{
	WCHAR		wszCfgFile[MAX_PATH] = { 0, };

	WRITELOG(L"Entering %S", __FUNCTION__);

	if (wszPrinterName) {
		StringCchCopy(g_sPrinterConfig.wszPrinterName, 64, wszPrinterName);
	}

	if (!IsExistPrinter(g_sPrinterConfig.wszPrinterName)) {
		WRITELOG(L"Not found %s printer, so installing...", g_sPrinterConfig.wszPrinterName);
		InstallFilePrinter();
	}

	if (wszPort) {
		StringCchCopy(g_sPrinterConfig.wszPortName, 32, wszPort);
	}

	StringCchPrintf(wszCfgFile, MAX_PATH, L"%s\\PrinterHelper.cfg", g_wszCurrentDir);

	if (FALSE == WritePrivateProfileStruct(L"PrinterHelper", L"Config", &g_sPrinterConfig, sizeof(SPRINTER_CONFIG), wszCfgFile)) {
		WRITELOG(L"Failed to write printer config to cfg file(%d).", GetLastError());
	}

	WRITELOG(L"Leaving %S", __FUNCTION__);
}

/**
	@brief		CFG에 설정된 파일 프린터를 설치한다.
	@date		2020.07.16
	@author		Kevin
*/
PRINTERHELPERCTRL_API
VOID
InstallFilePrinter()
{
	HANDLE		hThread;

	if (g_bInstalling == FALSE) {
		g_bInstalling = TRUE;

		hThread = CreateThread(NULL, 0, InstallFilePrinter, &g_bInstalling, 0, NULL);
		if (hThread == NULL) {
			g_bInstalling = FALSE;
		}
	}
}

/**
	@brief		파일 명 포멧을 설정한다.
	@param[in]	wszFileNameFormat		파일 이름 포멧
	@date		2020.07.16
	@author		Kevin
*/
PRINTERHELPERCTRL_API
VOID
SetFileNameFormat(__in PWCHAR wszFileNameFormat)
{
	WCHAR	wszCfgFile[MAX_PATH] = { L'\0', };

	do {
		StringCchPrintf(wszCfgFile, MAX_PATH, L"%s\\PrinterHelper.cfg", g_wszCurrentDir);

		if (g_sPrinterConfig.wszPrinterName[0] == L'\0') {
			GetConfig();
		}

		StringCchCopy(g_sPrinterConfig.wszFileNameFormat, 32, wszFileNameFormat);

		WritePrivateProfileStruct(L"PrinterHelper", L"Config", &g_sPrinterConfig, sizeof(SPRINTER_CONFIG), wszCfgFile);
	} while (FALSE);
}

/**
	@brief		기본 프린터를 변경하는 루틴
*/
DWORD
WINAPI
ChangeDefaultPrinter(__in LPVOID lpParam)
{
	DWORD	dwRet = NO_ERROR;
	WCHAR	wszPrinterName[64] = { 0, };
	DWORD	dwLength = 64;

	do {
		if (g_sPrinterConfig.bRun) {	// 기본 프린터를 설정된 프린터로 변경한다.
			GetDefaultPrinter(wszPrinterName, &dwLength);

			if ((g_sPrinterConfig.wszPrinterName[0] != '\0')
				&& (_wcsicmp(wszPrinterName, g_sPrinterConfig.wszPrinterName) != 0))
			{
				WCHAR	wszCfgFile[MAX_PATH] = { 0, };

				SetDefaultPrinter(g_sPrinterConfig.wszPrinterName);

				StringCchPrintf(wszCfgFile, MAX_PATH, L"%s\\PrinterHelper.cfg", g_wszCurrentDir);
				WritePrivateProfileString(L"Default", L"Printer", wszPrinterName, wszCfgFile);
			}
		}
		else {							// 기존의 기본 프린터로 변경한다.
			break;
		}

		// 3초에 한번씩
		Sleep(3000);
	} while (g_bChangePrinter);

	return dwRet;
}

/**
	@brief		SPrinter 모듈의 설정을 수정한다.
	@param[in]	bRun			실행 여부(On/Off)
	@param[in]	lpDestDir		출력 파일의 위치
	@return		ERROR_SUCCESS	성공
	@return		DWORD			에러코드
	@date		2020.07.09
	@author		Kevin
*/
PRINTERHELPERCTRL_API
DWORD
SetConfig(__in BOOL bRun, __in PWCHAR lpDestDir)
{
	DWORD				dwRet = ERROR_SUCCESS;	
	WCHAR				wszCfgFile[MAX_PATH] = { L'\0', };

	StringCchCopy(wszCfgFile, MAX_PATH, g_wszCurrentDir);
	StringCchCat(wszCfgFile, MAX_PATH, L"\\PrinterHelper.cfg");

	if (g_sPrinterConfig.bRun != bRun) {
		// 파일로 저장 설정이 On일 경우 스레드를 이용하여 주기적으로 Default Printer를 변경한다.
		if (bRun) {
			if (g_hChangePrinter == NULL) {
				g_bChangePrinter = TRUE;
				g_hChangePrinter = CreateThread(NULL, 0, ChangeDefaultPrinter, NULL, 0, NULL);
			}
		}
		else {
			g_bChangePrinter = FALSE;
			if (g_hChangePrinter) {
				CloseHandle(g_hChangePrinter);
				g_hChangePrinter = NULL;
			}
			
			WCHAR	wszPrinterName[64] = { 0, };

			GetPrivateProfileString(L"Default", L"Printer", L"", wszPrinterName, 64, wszCfgFile);
			if (wszPrinterName[0] != L'\0') {
				SetDefaultPrinter(wszPrinterName);
			}
			// 삭제
			WritePrivateProfileString(L"Default", L"Printer", L"", wszCfgFile);
		}
	}

	g_sPrinterConfig.bRun = bRun;
	if (lpDestDir && lpDestDir[1] == L':') {
		StringCchCopy(g_sPrinterConfig.wszOutputDir, MAX_PATH, lpDestDir);

		WRITELOG(L"Change OutputDir : %s", g_sPrinterConfig.wszOutputDir);
	}
	else if(lpDestDir == NULL) {
		ZeroMemory(g_sPrinterConfig.wszOutputDir, MAX_PATH * sizeof(WCHAR));
	}

	if (FALSE == WritePrivateProfileStruct(L"PrinterHelper", L"Config", &g_sPrinterConfig, sizeof(SPRINTER_CONFIG), wszCfgFile)) {
		dwRet = GetLastError();
		WRITELOG(L"Failed to write printer config(%d).", dwRet);
	}

	return dwRet;
}

/**
	@brief		SPrinter 모듈의 현재 설정을 가져한다.
	@param[out]	wszOutputDir		출력 파일의 위치
	@param[out]	wszFileNameFormat	파일 이름 포멧
	@return		ERROR_SUCCESS	성공
	@return		DWORD			에러코드
	@date		2020.07.09
	@author		Kevin
*/
PRINTERHELPERCTRL_API
DWORD
GetConfig(__out PWCHAR wszOutputDir, __out PWCHAR wszFileNameFormat)
{
	DWORD				dwRet = ERROR_SUCCESS;	
	WCHAR				wszCfgFile[MAX_PATH] = { L'\0', };

	StringCchCopy(wszCfgFile, MAX_PATH, g_wszCurrentDir);
	StringCchCat(wszCfgFile, MAX_PATH, L"\\PrinterHelper.cfg");

	if (!GetPrivateProfileStruct(L"PrinterHelper", L"Config", &g_sPrinterConfig, sizeof(SPRINTER_CONFIG), wszCfgFile)) {
		dwRet = GetLastError();
		WRITELOG(L"Failed to get printer config(%d).", dwRet);
		SetLastError(dwRet);
	}
	else {
		WRITELOG(L"OutDir : %s", g_sPrinterConfig.wszOutputDir);

		if (wszOutputDir) {
			StringCchCopy(wszOutputDir, MAX_PATH, g_sPrinterConfig.wszOutputDir);
		}

		WRITELOG(L"FileNameFormat : %s", g_sPrinterConfig.wszFileNameFormat);

		if (wszFileNameFormat) {
			StringCchCopy(wszFileNameFormat, MAX_PATH, g_sPrinterConfig.wszFileNameFormat);
		}
	}

	return static_cast<DWORD>(g_sPrinterConfig.bRun);
}

/**
	@brief		프로세스에 로딩된 DLL을 확인하여 Attach를 요청하는 루틴
	@param[in]	dwProcessId		프로세스 ID
	@param[in]	b32bitProcess	32비트 프로세스인지 여부(TRUE 32비트 프로세스)
	@return		ERROR_SUCCESS	성공
	@return		DWORD			에러코드
	@date		2020.07.09
	@author		Kevin
*/
DWORD
CheckAndAttachModule(__in DWORD dwProcessId, __in BOOL b32bitProcess)
{
	DWORD			dwRet = ERROR_SUCCESS;
	HANDLE			hSnapshot;
	DWORD			dwFlags = TH32CS_SNAPMODULE;
	MODULEENTRY32	me = { 0, };
	WCHAR			wszCommandLine[MAX_PATH * 2 + 10] = { 0, };

	STARTUPINFO			si = { 0, };
	PROCESS_INFORMATION	pi = { 0, };
	BOOL			bAttach = FALSE;

	do {
#ifdef _AMD64_
		if (b32bitProcess) {
			dwFlags |= TH32CS_SNAPMODULE32;
		}
#endif

		hSnapshot = CreateToolhelp32Snapshot(dwFlags, dwProcessId);
		if (hSnapshot == INVALID_HANDLE_VALUE) {
			dwRet = GetLastError();
			WRITELOG(L"Failed to create module snapshot of \'%d\' process(%d).", dwProcessId, dwRet);
			break;
		}

		me.dwSize = sizeof(MODULEENTRY32);

		Module32First(hSnapshot, &me);
							//	 	  123 45678901 23456789
		if (_wcsnicmp(me.szExePath, L"C:\\Windows\\System32", 19) == 0) {
			if ((_wcsicmp(me.szModule, L"xpsrchvw.exe") != 0)
				&& (_wcsicmp(me.szModule, L"notepad.exe") != 0))
			{
				WRITELOG(L"Internal excepted process : %s", me.szExePath);
				break;
			}
		}								// 123 45678901 23456789012
		else if (_wcsnicmp(me.szExePath, L"C:\\Windows\\SystemApps\\", 22) == 0) {
			WRITELOG(L"Internal excepted process : %s", me.szExePath);
			break;
		}

		// 예외 프로세스가 있을 경우, 첫번째 모듈 정보를 이용하여 예외 처리(Injection 하지 않음)한다.
		if (g_exceptedProcess.size() > 0) {
			BOOL		bExceptedProcess = FALSE;

			for (vector<wstring>::iterator iter = g_exceptedProcess.begin(); iter != g_exceptedProcess.end(); iter++) {
				if ((*iter).size() > 4) {
					if (((*iter).c_str()[1] == L':')
						&& (_wcsicmp((*iter).c_str(), me.szExePath) == 0))
					{
						bExceptedProcess = TRUE;
						break;
					}
					else if (_wcsicmp((*iter).c_str(), me.szModule) == 0) {
						bExceptedProcess = TRUE;
						break;
					}
				}
			}

			if (bExceptedProcess) {
				WRITELOG(L"User definition excepted process : %s", me.szExePath);
				break;
			}
		}

		if (!b32bitProcess)
			StringCchPrintf(wszCommandLine, MAX_PATH * 2 + 10, L"\"%s\\%s.exe\" -i %d \"%s\\%s.dll\"", g_wszCurrentDir, INJECTOR_NAME, dwProcessId, g_wszCurrentDir, SPRINTER_NAME);
		else
			StringCchPrintf(wszCommandLine, MAX_PATH * 2 + 10, L"\"%s\\%s32.exe\" -i %d \"%s\\%s32.dll\"", g_wszCurrentDir, INJECTOR_NAME, dwProcessId, g_wszCurrentDir, SPRINTER_NAME);

		do {
			if (_wcsicmp(me.szModule, L"gdi32.dll") == 0) {
				// Attach
				bAttach = TRUE;
			}
			// 이미 Attach 되어 있다면, 하지 않는다.
			else if (_wcsnicmp(me.szModule, INJECTOR_NAME, wcslen(INJECTOR_NAME)) == 0) {
				bAttach = FALSE;
				break;
			}
		} while (Module32Next(hSnapshot, &me));

		if (bAttach) {
			OutputDebugString(wszCommandLine);
			OutputDebugString(L"\n");

			// 실행
			if (FALSE == CreateProcess(NULL, wszCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
				dwRet = GetLastError();
				WRITELOG(L"Failed to injector process(%d).", dwRet);
				break;
			}

			DWORD dwWait = WaitForSingleObject(pi.hProcess, 2000);

			if (si.hStdError) {
				CloseHandle(si.hStdError);
			}

			if (si.hStdInput) {
				CloseHandle(si.hStdInput);
			}

			if (si.hStdOutput) {
				CloseHandle(si.hStdOutput);
			}

			// 핸들 리소스 제거
			if (pi.hProcess) {
				if (pi.hThread) {
					CloseHandle(pi.hThread);
				}

				CloseHandle(pi.hProcess);
			}
		}
	} while (FALSE);

	if (hSnapshot) {
		CloseHandle(hSnapshot);
	}

	return dwRet;
}

/**
	@brief		지정된 프로세스에 SPrinter 모듈을 Attach 한다.
	@param[in]	dwProcessId		프로세스 ID
	@return		ERROR_SUCCESS	성공
	@return		DWORD			에러코드
	@date		2020.07.09
	@author		Kevin
*/
PRINTERHELPERCTRL_API
DWORD
ManualAttach(__in DWORD dwProcessId)
{
	DWORD	dwRet = ERROR_SUCCESS;
	BOOL	bWow64 = TRUE;
	
	do {
#ifdef _AMD64_
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
		if (hProcess == NULL) {
			dwRet = GetLastError();
			WRITELOG(L"Failed to open process(%d).", dwRet);
			break;
		}
		if (FALSE == IsWow64Process(hProcess, &bWow64)) {
			dwRet = GetLastError();
			WRITELOG(L"Failed to open process(%d).", dwRet);
			bWow64 = FALSE;
		}

		CloseHandle(hProcess);
#endif

		dwRet = CheckAndAttachModule(dwProcessId, bWow64);
	} while (FALSE);

	return dwRet;
}

/**
	@brief		프로세스 생성 알림 콜백
	@param[in]	dwProcessId		프로세스ID
	@param[in]	lpProcessPath	프로세스 경로
	@date		2020.07.10
	@author		Kevin
*/
VOID 
ProcessCreateNotify(__in DWORD dwProcessId, __in LPWSTR lpProcessPath)
{
	BOOL	bExceptedProcess = FALSE;
	DWORD	dwRet;

	do {
		// 예외 프로세스 확인
		if (g_exceptedProcess.size()) {
			for (vector<wstring>::iterator iter = g_exceptedProcess.begin();
				iter != g_exceptedProcess.end();
				iter++)
			{
				if (_wcsicmp((*iter).c_str(), lpProcessPath) == 0) {
					bExceptedProcess = TRUE;
					break;
				}
			}

			if (bExceptedProcess) {
				break;
			}
		}
		
		dwRet = ManualAttach(dwProcessId);
		if (dwRet != ERROR_SUCCESS) {
			// 실패
		}
	} while (FALSE);
}

/**
	@brief		현재 실행중인 프로세스들에 Attach 하는 루틴
	@return		ERROR_SUCCESS		성공
	@return		DWORD				에러코드
	@date		2020.07.10
	@author		Kevin
*/
DWORD
AttachToExistProcesses()
{
	DWORD			dwRet = ERROR_SUCCESS;
	HANDLE			hSnapshot;
	PROCESSENTRY32	pe = { 0, };
	BOOL			bWin32Process;

	WRITELOG(L"Entering Attach to exist processes.");

	do {
		pe.dwSize = sizeof(PROCESSENTRY32);

		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) {
			dwRet = GetLastError();
			WRITELOG(L"Failed to create tool help 32 for processes.", dwRet);
			break;
		}

		Process32First(hSnapshot, &pe);

		do {
			if ((pe.th32ProcessID > 4)
				&& (pe.th32ProcessID != GetCurrentProcessId()))
			{
				// 기본 예외
				if (_wcsicmp(pe.szExeFile, L"winlogon.exe") == 0) {
					continue;
				}

				bWin32Process = TRUE;
#ifdef _AMD64_
				HANDLE	hProcess;

				// WOW64 프로세스 확인
				hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
				if (hProcess == NULL) {
					bWin32Process = FALSE;
				}
				else {
					if (FALSE != IsWow64Process(hProcess, &bWin32Process)) {
						// 실패
						bWin32Process = FALSE;
					}
					CloseHandle(hProcess);
				}
#endif


				WCHAR		message[MAX_PATH + 10];

				StringCchPrintf(message, MAX_PATH+10, L"%d : %s\n", pe.th32ProcessID, pe.szExeFile);
				OutputDebugString(message);
				dwRet = CheckAndAttachModule(pe.th32ProcessID, bWin32Process);
				if (dwRet != ERROR_SUCCESS) {
					//
				}
			}
		} while (Process32Next(hSnapshot, &pe));
	} while (FALSE);

	if (hSnapshot) {
		CloseHandle(hSnapshot);
	}

	WRITELOG(L"Leaving Attach to exist processes.");

	return dwRet;
}

/**
	@ingroup	group1
	@brief		자동으로 프로세스에 Attach 하도록 설정한다.
	@param[in]	bAuto			자동 여부(TRUE:자동, FALSE:수동)
	@return		ERROR_SUCCESS	성공
	@return		DWORD			에러코드
	@date		2020.07.09
	@author		Kevin
*/
PRINTERHELPERCTRL_API
DWORD
AutoAttach(__in BOOL bAuto)
{
	DWORD	dwRet = ERROR_SUCCESS;

	WRITELOG(L"Set AutoAttach : %s", bAuto?L"Auto":L"Manual");

	if (bAuto) {
		do {
			// 현재 실행중인 프로세스에 모두 Attach 함
			AttachToExistProcesses();

			// 새로 생성될 프로세스에 Attach 하기 위한 준비
			for (int i = 0; i < 3; i++) {
				dwRet = g_classProcessCreateNotify.Start(ProcessCreateNotify);
				if (dwRet == 0x800703e5) {
					Sleep(1000);
				}
				else {
					dwRet = HRESULT_CODE(dwRet);
					break;
				}
			}		
		} while (FALSE);
	}
	else {	
		dwRet = HRESULT_CODE(g_classProcessCreateNotify.Stop());
	}

	return dwRet;
}

/**
	@brief		자동 Attach 활성화시 Attach 시키지 않을 프로세스를 등록한다.
	@param[in]	lpProcessPath		프로세스 경로
	@date		2020.07.09
	@author		Kevin
*/
PRINTERHELPERCTRL_API
VOID
InsertExceptedProcess(__in LPWSTR lpProcessPath)
{
	WRITELOG(L"Insert a excepted process : %s", lpProcessPath);

	g_exceptedProcess.insert(g_exceptedProcess.begin(), lpProcessPath);
}

/**
	@brief		자동 Attach 활성화시 Attach 시키지 않을 프로세스 목록에서 프로세스를 제거한다.
	@param[in]	lpProcessPath		프로세스 경로
	@date		2020.07.09
	@author		Kevin
*/
PRINTERHELPERCTRL_API
VOID
RemoveExceptedProcess(__in LPWSTR lpProcessPath)
{
	WRITELOG(L"Remove a excepted process : %s", lpProcessPath);

	for (vector<wstring>::iterator iter = g_exceptedProcess.begin(); iter != g_exceptedProcess.end(); iter++) {
		if (wcscmp((*iter).c_str(), lpProcessPath) == 0) {
			(*iter).clear();
			g_exceptedProcess.erase(iter);
			break;
		}
	}
}

/**
	@brief		자동 Attach 활성화시 Attach 시키지 않을 프로세스 목록을 모두 제거한다.
	@param[in]	lpProcessPath		프로세스 경로
	@date		2020.07.09
	@author		Kevin
*/
PRINTERHELPERCTRL_API
VOID
CleanupExceptedProcesses()
{
	WRITELOG(L"Cleanup excepted process list.");

	for (vector<wstring>::iterator iter = g_exceptedProcess.begin(); iter != g_exceptedProcess.end(); iter++) {
		(*iter).clear();
	}
	g_exceptedProcess.clear();
}