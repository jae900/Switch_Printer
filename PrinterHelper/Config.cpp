/**
	@file		Config.cpp
	@brief		파일맵 설정을 읽어오는 함수가 정의된 파일
*/
#include "stdafx.h"
#include "..\Common\Common.h"
#include "Config.h"
#include <strsafe.h>
#include "Debug.h"

extern SPRINTER_CONFIG	g_Config;

WCHAR		g_wszCfgFilePath[MAX_PATH] = { L'\0', };		///< Cfg 파일 경로

/**
	@brief		설정 정보를 가져온다.
	@param[in]	pConfig			설정 파일 포인터
	@return		DWORD			에러코드
	@return		ERROR_SUCCESS	성공
	@date		2020.07.14
	@author		Kevin
*/
DWORD
GetConfiguration(__out PSPRINTER_CONFIG pConfig)
{
	DWORD				dwRet = ERROR_SUCCESS;

	DEBUG(L"Entering %S", __FUNCTION__);

	if (FALSE == GetPrivateProfileStruct(L"PrinterHelper", L"Config", pConfig, sizeof(SPRINTER_CONFIG), g_wszCfgFilePath)) {
		dwRet = GetLastError();

		DEBUG(L"Failed to get Config(%d).", dwRet);

		StringCchCopy(pConfig->wszPrinterName, 64, DEFAULT_PRINTER);
		StringCchCopy(pConfig->wszPortName, 32, DEFAULT_PRINTER_PORT);
	}
	else {
		DEBUG(L"OutputDir : %s", pConfig->wszOutputDir);
		DEBUG(L"FileNameFormat : %s", pConfig->wszFileNameFormat);
	}

	DEBUG(L"Leaving %S", __FUNCTION__);

	return dwRet;
}

/**
	@brief		환경변수가 포함된 문자열을 인지 가능한 수 있는 문자열로 변환한다.
	@param[out]	wszNewMessage		새로운 문자열이 저장될 버퍼
	@param[in]	wszOrgMessage		원본 문자열 버퍼
	@date		2020.07.17
	@author		Kevin
*/
VOID
ConvertStringWithEnvironmentVariables(OUT PWCHAR wszNewMessage, IN PWCHAR wszOrgMessage)
{
	WCHAR		wszEnv[MAX_PATH];
	WCHAR		wszTemp[MAX_PATH] = { 0, };
	size_t		nStart = -1;
	SYSTEMTIME	sysTime;

	DEBUG(L"org : %s", wszOrgMessage);

	GetLocalTime(&sysTime);

	for (size_t i = 0; i < wcslen(wszOrgMessage); i++) {
		if (wszOrgMessage[i] == L'%') {
			if (nStart == -1) {
				nStart = i;
			}
			else {
				StringCchCopyN(wszEnv, MAX_PATH, &wszOrgMessage[nStart + 1], i - nStart - 1);

				DEBUG(L"%s", wszEnv);

				if (wcscmp(wszEnv, L"YYYY") == 0) {
					StringCchPrintf(wszEnv, MAX_PATH, L"%04d\0", sysTime.wYear);
					StringCchCat(wszTemp, MAX_PATH, wszEnv);
				}
				else if (wcscmp(wszEnv, L"YY") == 0) {
					StringCchPrintf(wszEnv, MAX_PATH, L"%04d\0", sysTime.wYear);
					StringCchCat(wszTemp, MAX_PATH, &wszEnv[2]);
				}
				else if (wcscmp(wszEnv, L"MM") == 0) {
					StringCchPrintf(wszEnv, MAX_PATH, L"%02d\0", sysTime.wMonth);
					StringCchCat(wszTemp, MAX_PATH, wszEnv);
				}
				else if (wcscmp(wszEnv, L"DD") == 0) {
					StringCchPrintf(wszEnv, MAX_PATH, L"%02d\0", sysTime.wDay);
					StringCchCat(wszTemp, MAX_PATH, wszEnv);
				}
				else if (wcscmp(wszEnv, L"hh") == 0) {
					StringCchPrintf(wszEnv, MAX_PATH, L"%02d\0", sysTime.wHour);
					StringCchCat(wszTemp, MAX_PATH, wszEnv);
				}
				else if (wcscmp(wszEnv, L"mm") == 0) {
					StringCchPrintf(wszEnv, MAX_PATH, L"%02d\0", sysTime.wMinute);
					StringCchCat(wszTemp, MAX_PATH, wszEnv);
				}
				else if (wcscmp(wszEnv, L"ss") == 0) {
					StringCchPrintf(wszEnv, MAX_PATH, L"%02d\0", sysTime.wSecond);
					StringCchCat(wszTemp, MAX_PATH, wszEnv);
				}
				else if (wcscmp(wszEnv, L"ms") == 0) {
					StringCchPrintf(wszEnv, MAX_PATH, L"%03d\0", sysTime.wMilliseconds);
					StringCchCat(wszTemp, MAX_PATH, wszEnv);
				}
				else {
					GetEnvironmentVariable(wszEnv, wszEnv, MAX_PATH);
					StringCchCat(wszTemp, MAX_PATH, wszEnv);
				}

				DEBUG(L"-> %s = %s", wszEnv, wszTemp);

				nStart = -1;
			}
		}
		else if (nStart == -1) {
			switch (wszOrgMessage[i]) {
			case L'?':
			case L'/':
			case L'\\':
			case L'*':
			case L'<':
			case L'>':
			case L':':
			case L'|':
				StringCchCat(wszTemp, MAX_PATH, L"_");
				break;
			default:
				StringCchCatN(wszTemp, MAX_PATH, &wszOrgMessage[i], 1);
				break;
			}
		}
	}

	StringCchCopy(wszNewMessage, MAX_PATH, wszTemp);

	DEBUG(L"%s -> %s", wszTemp, wszNewMessage);
}

/**
	@brief		출력시 다른이름으로 저장 Dialog를 활성화/비활성화 하는 루틴
	@param[in]	bBypass			Dialog 보임/안보임
	@param[in]	wszFilePath		생성될 파일 경로
	@return		S_OK			성공
	@return		DWORD			에러코드
	@date		2020.07.17
	@author		Kevin
	@note		CutePDF만 해당되는 사항임
*/
DWORD
SetBypassSaveAs(__in BOOL bBypass)
{
	DWORD		lStatus;
	HKEY		hKey = NULL;

	do {
		if (bBypass) {
			WCHAR		wszFilePath[MAX_PATH] = { 0, };
			WCHAR		wszFileName[64] = { 0, };
			WCHAR		wszValue[] = L"1";

			if (g_Config.wszFileNameFormat[0] != L'\0') {
				ConvertStringWithEnvironmentVariables(wszFileName, g_Config.wszFileNameFormat);
			}
			else {
				ConvertStringWithEnvironmentVariables(wszFileName, DEFAULT_FILE_NAME_FORMAT);
			}

			StringCchPrintf(wszFilePath, MAX_PATH, L"%s\\%s.pdf", g_Config.wszOutputDir, wszFileName);

			lStatus = RegCreateKey(HKEY_CURRENT_USER, L"Software\\CutePDF Writer", &hKey);
			if (lStatus != S_OK) {
				DEBUG(L"Failed to create key for BypassSaveAs(0x%08x).", lStatus);
				break;
			}

			lStatus = RegSetValueEx(hKey, L"BypassSaveAs", 0, REG_SZ, (LPBYTE)wszValue, sizeof(WCHAR));
			if (lStatus != S_OK) {
				DEBUG(L"Failed to set BypassSaveAs(0x%08x).", lStatus);
				break;
			}

			lStatus = RegSetValueEx(hKey, L"OutputFile", 0, REG_SZ, (LPBYTE)wszFilePath, static_cast<DWORD>((wcslen(wszFilePath) + 1) * sizeof(WCHAR)));
			if (lStatus != S_OK) {
				DEBUG(L"Failed to set output file path(0x%08x).", lStatus);
				break;
			}
		}
		else {
			lStatus = RegDeleteKey(hKey, L"BypassSaveAs");
			if (lStatus != S_OK) {
				DEBUG(L"Failed to delete BypassSaveAs(0x%08x).", lStatus);
				break;
			}
		}
	} while (FALSE);

	if (hKey) {
		RegCloseKey(hKey);
	}

	return lStatus;
}
