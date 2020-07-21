#include "stdafx.h"
#include "Debug.h"
#include <strsafe.h>

VOID
DebugMsg(__in PWCHAR wszFormat, ...)
{
	WCHAR		wszMessage[1024];

	va_list	argList;
	va_start(argList, wszFormat);
	StringCchVPrintf(wszMessage, 1024, wszFormat, argList);
	va_end(argList);

	StringCchCat(wszMessage, 1024, L"\n");

	OutputDebugString(wszMessage);
}