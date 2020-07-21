/**
	@file		PrinterHelper/Debug.h
	@brief		PrinterHelper�� Debug ���
*/
#pragma once

#ifdef _DEBUG
#define DEBUG		DebugMsg
#else
#define DEBUG
#endif

#define DBGHDRLENGTH  16
#define DBGHDR		L"[PrinterHelper] "

VOID
DebugMsg(__in PWCHAR wszFormat, ...);