/**
	@file		CProcessCreateNotification.h
	@brief		프로세스 생성 알림 클래스가 선언된 파일
*/
#pragma once

#include "stdafx.h"
#include "EventSink.h"

/**
	@ingroup group1
	@class		CProcessCreateNotify
	@brief		프로세스 생성 알림 클래스
	@date		2020.07.10
	@author		Kevin
*/
class CProcessCreateNotify {
public:
	CProcessCreateNotify();
	~CProcessCreateNotify();

	HRESULT Start(__in FN_CreateProcessNotify fnCallback);
	HRESULT Stop();

private:
	IWbemObjectSink	*m_pStubSink;
	CEventSink		*m_pEventSink;
	IWbemServices	*m_pSvc;

};