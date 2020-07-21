/**
	@file		PrinterHelper/Debug.cpp
	@brief		����� ��� �Լ��� ���ǵ� ����
*/
#include "stdafx.h"
#include "Debug.h"
#include <strsafe.h>

/**
	@brief		����� �޽����� ����Ѵ�.
	@param[in]	wszFormat		�޽��� ���ڿ� ����
	@param[in]	arglist			�Ķ����
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