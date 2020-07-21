/**
	@file		EventSink.cpp
	@brief		프로세스 생성 이벤트를 처리하는 클래스가 정의된 파일
*/
#include "stdafx.h"
#include "EventSink.h"
#include "Log.h"

ULONG
STDMETHODCALLTYPE
CEventSink::AddRef()
{
	return InterlockedIncrement(&m_lRef);
}

ULONG
STDMETHODCALLTYPE
CEventSink::Release()
{
	LONG lRef = InterlockedDecrement(&m_lRef);
	if (lRef == 0)
		delete this;

	return lRef;
}

/**
	@brief		EventSink의 인터페이스를 반환하는 루틴
	@date		2020.07.13
	@author		Kevin
*/
HRESULT
STDMETHODCALLTYPE
CEventSink::QueryInterface(REFIID riid, void** ppv)
{
	HRESULT		hResult = E_NOINTERFACE;

	if ((riid == IID_IUnknown)
		|| (riid == IID_IWbemObjectSink))
	{
		*ppv = (IWbemObjectSink *) this;
		AddRef();
		hResult = WBEM_S_NO_ERROR;
	}
	
	return hResult;
}

/**
	@brief		실제 프로세스 생성 알림을 처리하는 루틴
	@date		2020.07.09
	@author		Kevin
*/
HRESULT
STDMETHODCALLTYPE
CEventSink::Indicate(LONG lObjectCout, IWbemClassObject __RPC_FAR *__RPC_FAR *apObjAttry)
{
	HRESULT		hResult;
	_variant_t	vtProp;

	WRITELOG(L"Entering %S", __FUNCTION__);

	do {
		if (m_fnCreateProcessNofity == NULL) {
			WRITELOG(L"Create process notify routine is not registered.");
			break;
		}

		for (int i = 0; i < lObjectCout; i++) {
			do {
				hResult = apObjAttry[i]->Get(_bstr_t(L"TargetInstance"), 0, &vtProp, 0, 0);
				if (FAILED(hResult)) {
					WRITELOG(L"Failed to get instance(0x%08x).", hResult);
					continue;
				}

				IUnknown	*str = vtProp;
				hResult = str->QueryInterface(IID_IWbemClassObject, reinterpret_cast<void**>(&apObjAttry[i]));
				if (FAILED(hResult)) {
					WRITELOG(L"Failed to query interface(0x%08x).", hResult);
					break;
				}
				
				DWORD	dwProcessId;

				_variant_t		cn;
				hResult = apObjAttry[i]->Get(L"ProcessId", 0, &cn, NULL, NULL);
				if (SUCCEEDED(hResult)
					&& (cn.vt == VT_I4))
				{
					// vt == VT_BSTR, (Caption, ExecutablePath, CommandLine), 문자열
					// vt == VT_I4, (ProcessId, SessionId), 프로세스ID
					dwProcessId = cn.ulVal;
					VariantClear(&cn);
				}
				else {
					WRITELOG(L"Failed to get process id(0x%08x).", hResult);
					break;
				}

				hResult = apObjAttry[i]->Get(L"ExecutablePath", 0, &cn, NULL, NULL);
				if (SUCCEEDED(hResult)
					&& !bDone
					&& (cn.vt == VT_BSTR))
				{
					WRITELOG(L"New Process - %d:%s(VariantType : %d", dwProcessId, cn.bstrVal, cn.vt);
					m_fnCreateProcessNofity(dwProcessId, cn.bstrVal);
					VariantClear(&cn);
				}
			} while (FALSE);

			VariantClear(&vtProp);
		}
	} while (FALSE);

	return WBEM_S_NO_ERROR;
}

/**
	@brief		상태가 입력되는 루틴
*/
HRESULT
STDMETHODCALLTYPE
CEventSink::SetStatus(long lFlags, HRESULT hResult, __RPC__in_opt BSTR strParam, __RPC__in_opt IWbemClassObject *pObjParam)
{
	if (lFlags == WBEM_STATUS_COMPLETE) {
		WRITELOG(L"Call complete. hResult = 0x%08x\n", hResult);
		if (hResult == 0x80041032) {	// Cancel 명령이 전달됨

		}
	}
	else if (lFlags == WBEM_STATUS_PROGRESS) {
		WRITELOG(L"Call in progress.\n");
	}

	return WBEM_S_NO_ERROR;
}

/**
	@brief		프로세스 알림 콜백을 등록한다.
	@param[in]	fnCreateProcessNotify		등록할 프로세스 알림 콜백
	@date		2020.07.09
	@author		Kevin
*/
VOID
CEventSink::SetCreateProcessNotifyRoutine(FN_CreateProcessNotify fnCreateProcessNotify)
{
	m_fnCreateProcessNofity = fnCreateProcessNotify;
}