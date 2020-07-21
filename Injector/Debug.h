/**
	@file		Injector/Debug.h
	@brief		디버그 출력 위한 함수가 선언된 파일
*/
#pragma once

#ifdef _DEBUG
#define DEBUG		DebugMsg
#else
#define DEBUG
#endif

VOID
DebugMsg(__in PWCHAR wszFormat, ...);