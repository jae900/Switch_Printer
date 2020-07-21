/**
	@file		Config.h
	@brief		파일맵 읽어오는 함수가 선언된 파일
*/
#pragma once

#define DEFAULT_FILE_NAME_FORMAT		L"%YY%-%MM%-%DD%.%hh%.%mm%.%ss%"	///< 기본 파일 이름 포멧

DWORD
GetConfiguration(__out PSPRINTER_CONFIG pConfig);

DWORD
SetBypassSaveAs(__in BOOL bBypass);
