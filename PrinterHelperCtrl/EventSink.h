/**
	@file		EventSink.h
	@brief		프로세스 생성 이벤트를 처리하는 클래스가 선언된 파일
*/
#pragma once

#ifndef EVENTSINK_H
#define EVENTSINK_H
#endif

#define _WIN32_DCOM
#include <comdef.h>
#include <WbemIdl.h>

#pragma comment(lib, "wbemuuid.lib")

typedef VOID (*FN_CreateProcessNotify)(DWORD dwProcessId, LPWSTR lpProcessPath);

/**
	@ingroup	group1
	@class		CEventSink
	@brief		프로세스 생성 이벤트를 처리하는 클래스
	@date		2020.07.09
	@author		Kevin
*/
class CEventSink : public IWbemObjectSink
{
public:
	CEventSink() { m_lRef = 0; m_fnCreateProcessNofity = NULL; bDone = FALSE; }
	~CEventSink() { bDone = TRUE; }

	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);
	HRESULT STDMETHODCALLTYPE Indicate(LONG lObjectCout, IWbemClassObject __RPC_FAR *__RPC_FAR *apObjAttry);
	HRESULT STDMETHODCALLTYPE SetStatus(long lFlags, HRESULT hResult, __RPC__in_opt BSTR strParam, __RPC__in_opt IWbemClassObject *pObjParam);

	VOID SetCreateProcessNotifyRoutine(FN_CreateProcessNotify fnCreateProcessNotify);

private:
	LONG			m_lRef;
	bool			bDone;

	FN_CreateProcessNotify		m_fnCreateProcessNofity;	///< 프로세스 생성 알림 콜백
};