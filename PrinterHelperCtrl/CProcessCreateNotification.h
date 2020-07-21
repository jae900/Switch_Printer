/**
	@file		CProcessCreateNotification.h
	@brief		���μ��� ���� �˸� Ŭ������ ����� ����
*/
#pragma once

#include "stdafx.h"
#include "EventSink.h"

/**
	@ingroup group1
	@class		CProcessCreateNotify
	@brief		���μ��� ���� �˸� Ŭ����
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