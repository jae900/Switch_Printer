/**
	@file		Log.cpp
	@brief		로그 기록을 위한 CLog 클래스가 정의된 파일
*/
#include "stdafx.h"
#include "Log.h"
#include <strsafe.h>

CLog *CLog::Log_ = NULL;

/**
	@brief		소멸자
*/
CLog::~CLog()
{
	if (Log_) {
		delete Log_;
	}
}

CLog* CLog::GetInstance()
{
	if (Log_ == NULL) {
		Log_ = new CLog();
	}

	return Log_;
}

/**
	@brief		CLog 클래스의 초기화 루틴
	@param[in]	wszLogFile		로그 파일 경로
	@return		ERROR_SUCCESS	성공
	@return		DWORD			에러코드
	@date		2020.07.10
	@author		Kevin
*/
DWORD
CLog::InitLog(__in LPCWSTR lpszLogFile)
{
	DWORD	dwRet = ERROR_SUCCESS;
	DWORD	dwFileSize;
	HANDLE	hFile;

	do {
		StringCchCopy(m_wszLogFilePath, MAX_PATH, lpszLogFile);

		hFile = CreateFile(lpszLogFile, GENERIC_ALL, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			dwRet = GetLastError();
			OutputDebugString(L"CLog : INVALID_HANDLE_VALUE");
			break;
		}

		dwFileSize = GetFileSize(hFile, NULL);
		if (dwFileSize >= 1024 * 1024) {
			WCHAR		wszNewLogFile[MAX_PATH];

			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;

			StringCchCopy(wszNewLogFile, MAX_PATH, lpszLogFile);
			StringCchCat(wszNewLogFile, MAX_PATH, L".bak");

			MoveFile(lpszLogFile, wszNewLogFile);
		}
		else {
			if (dwFileSize == 0) {
				WORD	wUnicode = 0xFEFF;
				DWORD	dwLength;

				WriteFile(hFile, &wUnicode, 2, &dwLength, NULL);
			}

			CloseHandle(hFile);
		}
	} while (FALSE);

	return dwRet;
}

/**
	@brief		로그를 작성하는 루틴
	@param[in]	wszFormat		로그 문자열 포멧
	@date		2020.07.10
	@author		Kevin
*/
VOID
CLog::WriteLog(__in LPCSTR lpFunctionName, __in int nLineNumber, __in LPCWSTR lpszFormat, ...)
{
	DWORD	dwLength;
	WCHAR	wszMessage[1024];
	HANDLE	hFile = INVALID_HANDLE_VALUE;

	do {
		ZeroMemory(wszMessage, 1024);

		SYSTEMTIME	sysTime;
		GetLocalTime(&sysTime);
		StringCchPrintf(wszMessage, 1024, L"[%02d-%02d, %02d:%02d:%02d.%03d] (%S:%d) ",
											sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds,
											lpFunctionName, nLineNumber);

		

		size_t nLength = wcslen(wszMessage);

		va_list	argList;
		va_start(argList, lpszFormat);
		StringCchVPrintf(&wszMessage[nLength], 1024 - nLength, lpszFormat, argList);
		va_end(argList);
		
		StringCchCat(wszMessage, 1024, L"\r\n");

		hFile = CreateFile(m_wszLogFilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			OutputDebugString(wszMessage);

			StringCchPrintf(wszMessage, 1024, L"Failed to create lof file(%d).", GetLastError());

			OutputDebugString(wszMessage);
			break;
		}

		dwLength = GetFileSize(hFile, NULL);

		SetFilePointer(hFile, dwLength, NULL, FILE_BEGIN);

		if (FALSE == WriteFile(hFile, wszMessage, (DWORD)(wcslen(wszMessage) * 2), &dwLength, NULL)) {
			WCHAR	wszError[16];

			StringCchPrintf(wszError, 16, L"Error : %d\r\n", GetLastError());
			StringCchCat(wszMessage, 1024, wszError);

			OutputDebugString(wszMessage);
			break;
		}
	} while (FALSE);

	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
	}
}