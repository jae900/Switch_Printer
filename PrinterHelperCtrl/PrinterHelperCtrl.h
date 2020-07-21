/**
	@file		PrinterHelperCtrl.h
	@brief		SPrinter ���� ����� Export�� �Լ����� ����� ����
*/
#ifdef PRINTERHELPERCTRL_EXPORTS
#define PRINTERHELPERCTRL_API extern "C" __declspec(dllexport)
#else
#define PRINTERHELPERCTRL_API extern "C" __declspec(dllimport)
#endif

/**
	@brief		���� ���õ� ���� ������ ������ ��ȯ�Ѵ�.
	@param[out]	wszPrinterName		������ �̸�
	@param[out]	wszPort				��Ʈ
	@return		ERROR_SUCCESS		����
	@return		DWORD				�����ڵ�
*/
PRINTERHELPERCTRL_API
DWORD
GetSelectedFilePrinter(__out PWCHAR wszPrinterName, __out PWCHAR wszPort, __in BOOL bInit);

/**
	@brief		����� �����͸� ����
	@parma[in]	wszPrinterName		������ �̸�
	@param[in]	wszPort				��Ʈ
	@return		ERROR_SUCCESS		����
	@return		DWORD				�����ڵ�
*/
PRINTERHELPERCTRL_API
VOID
PrinterRedirection(__in PWCHAR wszPrinterName, __in PWCHAR wszPort);

/**
	@brief		CFG ������ �����Ͽ� ���� �����͸� ��ġ�Ѵ�.
*/
PRINTERHELPERCTRL_API
VOID
InstallFilePrinter();

/**
	@brief		SPrinter ����� ������ �����Ѵ�.
	@param[in]	bRun			���� ����(On/Off)
	@param[in]	lpDestDir		��� ������ ��ġ
	@return		ERROR_SUCCESS	����
	@return		DWORD			�����ڵ�
*/
PRINTERHELPERCTRL_API
DWORD
SetConfig(__in BOOL bRun, __in PWCHAR lpDestDir);

/**
	@brief		SPrinter ����� ���� ������ �����Ѵ�.
	@param[out]	wszOutputDir		��� ������ ��ġ
	@param[out]	wszFileNameFormat	���� �̸� ����
	@return		ERROR_SUCCESS	����
	@return		DWORD			�����ڵ�
*/
PRINTERHELPERCTRL_API
DWORD
GetConfig(__out PWCHAR wszOutputDir = NULL, __out PWCHAR wszFileNameFormat = NULL);

/**
	@brief		������ ���μ����� SPrinter ����� Attach �Ѵ�.
	@param[in]	dwProcessId		���μ��� ID
	@return		ERROR_SUCCESS	����
	@return		DWORD			�����ڵ�
*/
PRINTERHELPERCTRL_API
DWORD
ManualAttach(__in DWORD dwProcessId);

/**
	@brief		�ڵ����� ���μ����� Attach �ϵ��� �����Ѵ�.
	@param[in]	bAuto			�ڵ� ����(TRUE:�ڵ�, FALSE:����)
	@return		ERROR_SUCCESS	����
	@return		DWORD			�����ڵ�
*/
PRINTERHELPERCTRL_API
DWORD
AutoAttach(__in BOOL bAuto);

/**
	@brief		�ڵ� Attach Ȱ��ȭ�� Attach ��Ű�� ���� ���μ����� ����Ѵ�.
	@param[in]	lpProcessPath		���μ��� ���
*/
PRINTERHELPERCTRL_API
VOID
InsertExceptedProcess(__in LPWSTR lpProcessPath);

/**
	@brief		�ڵ� Attach Ȱ��ȭ�� Attach ��Ű�� ���� ���μ��� ��Ͽ��� ���μ����� �����Ѵ�.
	@param[in]	lpProcessPath		���μ��� ���
*/
PRINTERHELPERCTRL_API
VOID
RemoveExceptedProcess(__in LPWSTR lpProcessPath);

/**
	@brief		�ڵ� Attach Ȱ��ȭ�� Attach ��Ű�� ���� ���μ��� ����� ��� �����Ѵ�.
	@param[in]	lpProcessPath		���μ��� ���
*/
PRINTERHELPERCTRL_API
VOID
CleanupExceptedProcesses();

typedef DWORD (*FN_GetSelectedFilePrinter)(__out PWCHAR wszPrinterName, __out PWCHAR wszPort, __in BOOL bInit);
typedef VOID (*FN_PrinterRedirection)(__in PWCHAR wszPrinterName, __in PWCHAR wszPort);
typedef DWORD (*FN_SetConfig)(__in BOOL bRun, __in PWCHAR lpDestDir);
typedef DWORD (*FN_GetConfig)(__out PWCHAR wszOutputDir, __out PWCHAR wszFileNameFormat);
typedef DWORD (*FN_ManualAttach)(__in DWORD dwProcessId);
typedef DWORD (*FN_AutoAttach)(__in BOOL bAuto);
typedef VOID (*FN_InsertExceptedProcess)(__in LPWSTR lpProcessPath);
typedef VOID (*FN_RemoveExceptedProcess)(__in LPWSTR lpProcessPath);
typedef VOID (*FN_CleanupExceptedProcesses)();