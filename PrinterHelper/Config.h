/**
	@file		Config.h
	@brief		���ϸ� �о���� �Լ��� ����� ����
*/
#pragma once

#define DEFAULT_FILE_NAME_FORMAT		L"%YY%-%MM%-%DD%.%hh%.%mm%.%ss%"	///< �⺻ ���� �̸� ����

DWORD
GetConfiguration(__out PSPRINTER_CONFIG pConfig);

DWORD
SetBypassSaveAs(__in BOOL bBypass);
