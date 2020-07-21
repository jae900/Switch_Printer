/**
	@file		PrinterHelper/Debug.cpp
	@brief		디버그 출력 함수가 정의된 파일
*/
#include "stdafx.h"
#include "Debug.h"
#include <strsafe.h>

/**
	@brief		디버그 메시지를 출력한다.
	@param[in]	wszFormat		메시지 문자열 포멧
	@param[in]	arglist			파라메터
	@date		2020.07.13
	@author		Kevin
*/
VOID
DebugMsg(__in PWCHAR wszFormat, ...)
{
	WCHAR		wszMessage[1024];

	StringCchCopy(wszMessage, 1024, DBGHDR);

	va_list	argList;
	va_start(argList, wszFormat);
	StringCchVPrintf(&wszMessage[16], 1024 - 16, wszFormat, argList);
	va_end(argList);

	StringCchCat(wszMessage, 1024, L"\n");

	OutputDebugString(wszMessage);
}