/**
	@file		NewFunction.h
	@brief		후킹을 위해 새로 정의된 함수와 함수형이 선언된 파일
*/
#pragma once

#include <winspool.h>

typedef HDC (WINAPI *FN_CREATEDCW)(LPCWSTR pwszDriver, LPCWSTR pwszDevice, LPCWSTR pszPort,	const DEVMODEW *pdm);
typedef HMODULE (WINAPI *FN_LOADLIBRARYEXW)(__in LPCWSTR lpFileName, __reserved HANDLE hFile, __in DWORD dwFlags);
typedef HRESULT (WINAPI *FN_STARTXPSPRINTJOB)(__in const LPCWSTR printerName, __in const LPCWSTR jobName, __in const LPCWSTR outputFileName, __in HANDLE progressEvent,	__in HANDLE completionEvent, __in UINT8 *printablePagesOn, __in UINT32 printablePagesOnCount, __out PVOID *xpsPrintJob,	__out PVOID *documentStream, __out PVOID *printTicketStream);
typedef BOOL (WINAPI *FN_OPENPRINTER2W)(__in LPTSTR pPrinterName, __out LPHANDLE phPrinter, __in LPPRINTER_DEFAULTS pDefault, __in PPRINTER_OPTIONSW pOptions);

HDC
WINAPI
NewCreateDCW(__in LPCWSTR        pwszDriver,
			 __in LPCWSTR        pwszDevice,
			 __in LPCWSTR        pszPort,
			 __in const DEVMODEW *pdm);

BOOL
WINAPI
NewOpenPrinter2W(__in LPTSTR pPrinterName,
				 __out LPHANDLE phPrinter,
				 __in LPPRINTER_DEFAULTS pDefault,
				 __in PPRINTER_OPTIONSW pOptions);

HMODULE
WINAPI
NewLoadLibraryExW(
	__in       LPCWSTR lpFileName,
	__reserved HANDLE  hFile,
	__in       DWORD   dwFlags);

HRESULT
WINAPI
NewStartXpsPrintJob(
	__in  const LPCWSTR printerName,
	__in  const LPCWSTR jobName,
	__in  const LPCWSTR outputFileName,
	__in HANDLE progressEvent,
	__in HANDLE completionEvent,
	__in UINT8 *printablePagesOn,
	__in UINT32 printablePagesOnCount,
	__out PVOID *xpsPrintJob,
	__out PVOID *documentStream,
	__out PVOID *printTicketStream);