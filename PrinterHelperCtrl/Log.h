/**
	@file		Log.h
	@brief		파일 로그 기록을 위한 CLog 클래스가 선언된 파일
*/
#pragma once

/**
	@class		CLog
	@brief		파일 로그 저장을 위한 클래스
*/
class CLog {
protected:
	CLog() { ZeroMemory(m_wszLogFilePath, MAX_PATH * sizeof(WCHAR)); }

	static CLog* Log_;

public:
	CLog(CLog &other) {}
	~CLog();

	void operator=(const CLog&) {}
	static CLog *GetInstance();

public:
	DWORD InitLog(__in LPCWSTR lpszLogFile);
	VOID WriteLog(__in LPCSTR lpFunctionName, __in int nLineNumber, __in LPCWSTR lpszFormat,...);

private:
	WCHAR	m_wszLogFilePath[MAX_PATH];			///< 로그 파일 경로
};

extern CLog		*g_pLog;
#define WRITELOG(msg,...)		g_pLog->WriteLog(__FUNCTION__, __LINE__, msg, __VA_ARGS__)
