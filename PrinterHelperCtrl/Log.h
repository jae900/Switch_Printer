/**
	@file		Log.h
	@brief		���� �α� ����� ���� CLog Ŭ������ ����� ����
*/
#pragma once

/**
	@class		CLog
	@brief		���� �α� ������ ���� Ŭ����
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
	WCHAR	m_wszLogFilePath[MAX_PATH];			///< �α� ���� ���
};

extern CLog		*g_pLog;
#define WRITELOG(msg,...)		g_pLog->WriteLog(__FUNCTION__, __LINE__, msg, __VA_ARGS__)
