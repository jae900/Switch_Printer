/**
	@file		Injector/Debug.h
	@brief		����� ��� ���� �Լ��� ����� ����
*/
#pragma once

#ifdef _DEBUG
#define DEBUG		DebugMsg
#else
#define DEBUG
#endif

VOID
DebugMsg(__in PWCHAR wszFormat, ...);