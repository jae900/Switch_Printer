/**
	@defgroup	group1	프로세스 생성 알림 모듈
	@brief		프로세스 생성 알림 모듈
*/

/**
	@file		CProcessCreateNotification.cpp
	@brief		프로세스 생성 알림 클래스가 정의된 파일
*/
#include "stdafx.h"
#include "CProcessCreateNotification.h"
#include "Log.h"

extern CLog		*g_pLog;

/**
	@brief		생성자
*/
CProcessCreateNotify::CProcessCreateNotify()
	: m_pEventSink(NULL)
	, m_pStubSink(NULL)
	, m_pSvc(NULL)
{

}

/**
	@brief		소멸자
*/
CProcessCreateNotify::~CProcessCreateNotify()
{
	Stop();
}

/**
	@brief		프로세스 알림 콜백을 시작한다.
	@param[in]	fnCallback		알림 콜백
	@return		S_OK			성공
	@return		HRESULT			에러코드
	@date		2020.07.10
	@author		Kevin
*/
HRESULT
CProcessCreateNotify::Start(__in FN_CreateProcessNotify fnCallback)
{
	HRESULT				hResult = HRESULT_FROM_WIN32(ERROR_NOT_READY);
	IUnknown			*pStubUnk = NULL;
	IUnsecuredApartment	*pUnsecApp = NULL;
	IWbemLocator		*pLoc = NULL;

	WRITELOG(L"Entering start create process notification(%x).", m_pEventSink);

	do {
		hResult = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (FAILED(hResult)) {
			if (hResult != RPC_E_CHANGED_MODE) {
				WRITELOG(L"Failed to call CoInitiailzeEx(0x%08x).", hResult);
				break;
			}
		}

		hResult = CoInitializeSecurity(NULL,
									   -1,
									   NULL,
									   NULL,
									   RPC_C_AUTHN_LEVEL_DEFAULT,
									   RPC_C_IMP_LEVEL_IMPERSONATE,
									   NULL,
									   EOAC_NONE,
									   NULL);
		if (FAILED(hResult)) {
			if (hResult != 0x80010119) {
				WRITELOG(L"Failed to initialize security(0x%08x).", hResult);
				break;
			}
		}

		hResult = CoCreateInstance(CLSID_WbemLocator,
								   0,
								   CLSCTX_INPROC_SERVER,
								   IID_IWbemLocator,
								   (LPVOID*)&pLoc);
		if (FAILED(hResult)) {
			WRITELOG(L"Failed to create locator instance(0x%08x).", hResult);
			break;
		}

		hResult = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"),
									  NULL,
									  NULL,
									  0,
									  NULL,
									  0,
									  0,
									  &m_pSvc);
		if (FAILED(hResult)) {
			WRITELOG(L"Failed to connect server(0x%08x).", hResult);
			break;
		}

		hResult = CoSetProxyBlanket(m_pSvc,
									RPC_C_AUTHN_WINNT,
									RPC_C_AUTHZ_NONE,
									NULL,
									RPC_C_AUTHN_LEVEL_CALL,
									RPC_C_IMP_LEVEL_IMPERSONATE,
									NULL,
									EOAC_NONE);
		if (FAILED(hResult)) {
			WRITELOG(L"Failed to set proxy blanket(0x%08x).", hResult);
			break;
		}

		hResult = CoCreateInstance(CLSID_UnsecuredApartment,
								   NULL,
								   CLSCTX_LOCAL_SERVER,
								   IID_IUnsecuredApartment,
								   (void**)&pUnsecApp);
		if (FAILED(hResult)) {
			WRITELOG(L"Failed to create UnsecuredApartment(0x%08x).", hResult);
			break;
		}

		m_pEventSink = new CEventSink;
		if (m_pEventSink == NULL) {
			WRITELOG(L"Failed to allocate event sink.");
			break;
		}

		m_pEventSink->AddRef();

		m_pEventSink->SetCreateProcessNotifyRoutine(fnCallback);

		hResult = pUnsecApp->CreateObjectStub(m_pEventSink, &pStubUnk);
		if (FAILED(hResult)) {
			WRITELOG(L"Failed to create object stub(0x%08x).", hResult);
			break;
		}

		hResult = pStubUnk->QueryInterface(IID_IWbemObjectSink, (void**)&m_pStubSink);
		if (FAILED(hResult)) {
			WRITELOG(L"Failed to query interface(0x%08x).", hResult);
			break;
		}

		hResult = m_pSvc->ExecNotificationQueryAsync(_bstr_t("WQL"),
													 _bstr_t("SELECT * FROM __InstanceCreationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process'"),
													 WBEM_FLAG_SEND_STATUS,
													 NULL,
													 m_pStubSink);
		if (FAILED(hResult)) {
			WRITELOG(L"Failed to execute notification query async(0x%08x).", hResult);

			// 0x80041010
			// Win32_Process 클래스가 존재하지 않아서 발생

			// 1) net stop winmgmt
			// 2) C:\Windows\system32\WBEM -> _WBEM 으로 이름변경
			// 3) 재부팅
			// 4) net stop winmgmt
			// * 5) winmgmt /resetRepository
			// 6) 재부팅
		}
	} while (FALSE);

	if (pStubUnk) {
		pStubUnk->Release();
	}

	if (pUnsecApp) {
		pUnsecApp->Release();
	}

	if (pLoc) {
		pLoc->Release();
	}

	if (FAILED(hResult)) {
		if (m_pEventSink) {
			m_pEventSink->Release();
			m_pEventSink = NULL;
		}

		if (m_pStubSink) {
			m_pStubSink->Release();
			m_pStubSink = NULL;
		}

		if (m_pSvc) {
			m_pSvc->Release();
			m_pSvc = NULL;
		}

		CoUninitialize();
	}

	WRITELOG(L"Leaving start create process notification.");

	return hResult;
}

/**
	@brief		프로세스 생성 감시를 중지하는 루틴
	@return		S_OK			성공
	@return		HRESULT			에러코드
	@date		2020.07.10
	@author		Kevin
*/
HRESULT
CProcessCreateNotify::Stop()
{
	HRESULT			hResult = S_OK;

	WRITELOG(L"Entering stop create process notification.");

	if (m_pSvc) {
		hResult = m_pSvc->CancelAsyncCall(m_pStubSink);

		m_pSvc->Release();
		m_pSvc = NULL;

		if (m_pStubSink) {
			m_pStubSink->Release();
			m_pStubSink = NULL;
		}

		if (m_pEventSink) {
			m_pEventSink->Release();
			m_pEventSink = NULL;
		}

		CoUninitialize();
	}

	WRITELOG(L"Leaving stop create process notification.");

	return hResult;
}