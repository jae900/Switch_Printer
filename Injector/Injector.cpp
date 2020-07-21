/**
	@file		Injector.cpp
	@brief		�����Լ��� ���μ����� �������ϱ� ���� �Լ����� ���ǵ� ����
*/
#include "stdafx.h"
#include <TlHelp32.h>
#include "Injector.h"
#include "Debug.h"

LPTHREAD_START_ROUTINE g_pfnLoadLibraryW = NULL;

/**
	@brief		�Էµ� ���μ����� ����� Injection ��Ű�� ��ƾ
	@param[in]	dwProcessId			Injection ��ų ���μ���ID
	@param[in]	wszModuleName		��� �̸�
	@return		ERROR_SUCESS		����
	@return		DWORD				GetLastError(), �����ڵ�
	@date		2020.07.08
	@author		Kevin
*/
DWORD
Injection(__in DWORD dwProcessId,
		  __in PWCHAR wszModulePath)
{
	DWORD		dwRet = ERROR_SUCCESS;
	HMODULE		hKernel32 = NULL;
	HANDLE		hRemoteThread = NULL;
	HANDLE		hProcess = NULL;
	PVOID		pRemoteBuf = NULL;

	do {
		hKernel32 = GetModuleHandle(L"Kernel32.dll");
		if (hKernel32 == NULL) {
			dwRet = GetLastError();
			DEBUG(L"Failed to get module handle(%d).", dwRet);
			break;
		}

		g_pfnLoadLibraryW = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");
		if (g_pfnLoadLibraryW == NULL) {
			dwRet = GetLastError();
			DEBUG(L"Failed to get procedure(%d).", dwRet);
			break;
		}

		hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD, FALSE, dwProcessId);
		if (hProcess == NULL) {
			dwRet = GetLastError();
			DEBUG(L"Failed to open process(%d).", dwRet);
			break;
		}

		pRemoteBuf = VirtualAllocEx(hProcess, NULL, (wcslen(wszModulePath) + 1) * sizeof(WCHAR), MEM_COMMIT, PAGE_READWRITE);
		if (pRemoteBuf == NULL) {
			dwRet = GetLastError();
			DEBUG(L"Failed to allocate virtual memory(%d).", dwRet);
			break;
		}

		if (!WriteProcessMemory(hProcess, pRemoteBuf, wszModulePath, (wcslen(wszModulePath) + 1) * sizeof(WCHAR), NULL)) {
			dwRet = GetLastError();
			DEBUG(L"Failed to write injection code to other process memory(%d).", dwRet);
			break;
		}

		DEBUG(L"Injection Module : %s", wszModulePath);

		PFN_NT_CREATE_THREAD_EX		pfnNtCreateThreadEx = NULL;

		pfnNtCreateThreadEx = (PFN_NT_CREATE_THREAD_EX)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtCreateThreadEx");
		if (pfnNtCreateThreadEx != NULL) {
			dwRet = pfnNtCreateThreadEx(&hRemoteThread, 0x1FFFFF, NULL, hProcess, g_pfnLoadLibraryW, pRemoteBuf, FALSE, 0, 0, 0, NULL);
			if (dwRet != ERROR_SUCCESS) {
				dwRet = GetLastError();

				DEBUG(L"Failed to create thread(%d).", dwRet);

				hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)g_pfnLoadLibraryW, pRemoteBuf, 0, NULL);
			}
		}
		else {
			hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)g_pfnLoadLibraryW, pRemoteBuf, 0, NULL);
		}

		if (hRemoteThread == NULL) {
			dwRet = GetLastError();
			DEBUG(L"Failed to create remote thread(%d).", dwRet);
		}
		else {
			DEBUG(L"Succeeded to create remote thread(PID:%d).", dwProcessId);
		}
	} while (FALSE);

	if (hProcess) {
		if (pRemoteBuf) {
			if (hRemoteThread) {
				if (WAIT_FAILED != WaitForSingleObject(hRemoteThread, 10000)) {
					VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
				}

				CloseHandle(hRemoteThread);
			}
			else {
				VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
			}
		}

		CloseHandle(hProcess);
	}

	return dwRet;
}

/**
	@brief		���μ����� �ε��� ����� �ּҸ� �����´�.
	@param[in]	dwProcessId		���μ���ID
	@param[in]	wszModuleName	��� �̸�
	@return		HMODULE			��� �ڵ�(�ּ�)
	@date		2020.07.14
	@author		Kevin
*/
HMODULE
GetModuleAddress(__in DWORD dwProcessId, __in PWCHAR wszModuleName)
{
	DWORD			dwRet = ERROR_SUCCESS;
	HMODULE			hMod = NULL;
	MODULEENTRY32	me;
	HANDLE			hSnapshot = NULL;

	do {
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
		if (hSnapshot == INVALID_HANDLE_VALUE) {
			dwRet = GetLastError();
			DEBUG(L"Failed to creat tool help(%d).", dwRet);
			break;
		}

		me.dwSize = sizeof(MODULEENTRY32);

		if (FALSE == Module32First(hSnapshot, &me)) {
			dwRet = GetLastError();
			DEBUG(L"Failed to enumerate modules(%d).", dwRet);
			break;
		}

		dwRet = ERROR_NOT_FOUND;

		do {
			if (_wcsicmp(me.szModule, wszModuleName) == 0) {
				hMod = (HMODULE)me.modBaseAddr;
				dwRet = ERROR_SUCCESS;
				break;
			}
		} while (Module32Next(hSnapshot, &me));
	} while (FALSE);

	if (hSnapshot) {
		CloseHandle(hSnapshot);
	}

	if (dwRet != ERROR_SUCCESS) {
		SetLastError(dwRet);
	}

	return hMod;
}

/**
	@brief		�����ǵ� DLL�� �и��ϴ� ��ƾ
	@param[in]	dwProcessId		�и��ϰ��� �ϴ� ���μ����� ���μ���ID
	@return		ERROR_SUCCESS	����
	@return		DWORD			�����ڵ�
	@date		2020.07.14
	@author		Kevin
*/
DWORD
Detach(__in DWORD dwProcessId, __in PVOID lpModule, __in PWCHAR wszModuleName)
{
	DWORD		dwRet = ERROR_SUCCESS;
	HMODULE		hKernel32 = NULL;
	HANDLE		hProcess = NULL;
	HANDLE		hRemoteThread = NULL;

	do {
		if (lpModule == NULL) {
			lpModule = GetModuleAddress(dwProcessId, wszModuleName);
			if (lpModule == NULL) {
				dwRet = GetLastError();
				DEBUG(L"Failed to get module address(%d).", dwRet);
				break;
			}
		}

		hKernel32 = GetModuleHandle(L"Kernel32.dll");
		if (hKernel32 == NULL) {
			dwRet = GetLastError();
			DEBUG(L"Failed to get module handle(%d).", dwRet);
			break;
		}

		g_pfnLoadLibraryW = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "FreeLibrary");
		if (g_pfnLoadLibraryW == NULL) {
			dwRet = GetLastError();
			DEBUG(L"Failed to get procedure(%d).", dwRet);
			break;
		}

		hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_CREATE_THREAD, FALSE, dwProcessId);
		if (hProcess == NULL) {
			dwRet = GetLastError();
			DEBUG(L"Failed to open process(%d).", dwRet);
			break;
		}

		PFN_NT_CREATE_THREAD_EX		pfnNtCreateThreadEx = NULL;

		pfnNtCreateThreadEx = (PFN_NT_CREATE_THREAD_EX)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtCreateThreadEx");
		if (pfnNtCreateThreadEx == NULL) {
			dwRet = pfnNtCreateThreadEx(&hRemoteThread, 0x1FFFFF, NULL, hProcess, g_pfnLoadLibraryW, lpModule, FALSE, 0, 0, 0, NULL);
			if (dwRet != ERROR_SUCCESS) {
				dwRet = GetLastError();

				hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)g_pfnLoadLibraryW, lpModule, 0, NULL);
			}
		}
		else {
			hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)g_pfnLoadLibraryW, lpModule, 0, NULL);
		}

		if (hRemoteThread == NULL) {
			dwRet = GetLastError();
			DEBUG(L"Failed to create remote thread for free library(%d).", dwRet);
		}
		else {
			DEBUG(L"Succeeded to create remote thread for free library(PID:%d).", dwProcessId);
		}
	} while (FALSE);

	if (hProcess) {
		if (hRemoteThread) {
			CloseHandle(hRemoteThread);
		}

		CloseHandle(hProcess);
	}

	return dwRet;
}

/**
	@brief		��� ���μ����� Injection�� ����� �����Ѵ�.
	@param[in]	wszModuleName		������ ��� �̸�
	@return		0					����� Detach�� ���μ����� ����
	@return		1+					Detach�� ���μ����� ��
	@date		2020.07.14
	@author		Kevin
*/
DWORD
DetachToWholeProcesses(__in PWCHAR wszModuleName)
{
	DWORD			dwRet = ERROR_SUCCESS;
	DWORD			dwRet1;
	HANDLE			hSnapshot = NULL;
	PROCESSENTRY32	pe = { 0, };
	DWORD			dwCount = 0;

	do {
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) {
			dwRet = GetLastError();
			DEBUG(L"Failed to create tool help for enumerating process(%d).", dwRet);
			break;
		}

		pe.dwSize = sizeof(PROCESSENTRY32);

		if (!Process32First(hSnapshot, &pe)) {
			dwRet = GetLastError();
			DEBUG(L"Failed to enumerate process(%d).", dwRet);
			break;
		}

		do {
			// ���μ������� DLL�� detach �Ѵ�.
			dwRet1 = Detach(pe.th32ProcessID, NULL, wszModuleName);
			if (dwRet1 == ERROR_SUCCESS) {
				dwCount++;
				DEBUG(L"Detach : %d(%d)", pe.th32ProcessID, dwCount);
			}
			else if(dwRet1 != ERROR_NOT_FOUND) {
				// ������ ����
				dwRet = dwRet1;
			}
		} while (Process32Next(hSnapshot, &pe));
	} while (FALSE);

	if (hSnapshot) {
		CloseHandle(hSnapshot);
	}

	if ((dwRet != ERROR_SUCCESS)
		&& (dwRet != ERROR_NOT_FOUND))
	{
		SetLastError(dwRet);
	}

	return dwCount;
}

/**
	@brief		���� �Լ�
	@date		2020.07.08
	@author		Kevin
*/
int
WINAPI
wWinMain(__in HINSTANCE hInstance,
		 __in_opt HINSTANCE hPrevInstance,
		 __in_opt LPWSTR lpCmdLine,
		 __in int nShowCmd)
{
	DWORD		dwRet = ERROR_SUCCESS;
	LPWSTR		*wszArgList = NULL;
	int			nArgs;
	DWORD		dwProcessId;

	DEBUG(L"CMD : %s", lpCmdLine);

	wszArgList = CommandLineToArgvW(lpCmdLine, &nArgs);
	if (wszArgList) {
		if (_wcsicmp(wszArgList[0], L"-i") == 0) {
			dwProcessId = _wtoi(wszArgList[1]);
			if ((dwProcessId != 0)
				&& (dwProcessId != 4))
			{
				// Attach
				dwRet = Injection(dwProcessId, wszArgList[2]);
				if (dwRet != ERROR_SUCCESS) {
					DEBUG(L"Failed to inject process(%d).", dwRet);
				}
			}
		}
		else if (_wcsicmp(wszArgList[0], L"-d") == 0) {
			dwProcessId = _wtoi(wszArgList[1]);
			if (dwProcessId != 4)
			{
				if (dwProcessId == 0) {
					dwRet = DetachToWholeProcesses(wszArgList[2]);
				}
				else {
					dwRet = Detach(dwProcessId, NULL, wszArgList[2]);
				}
			}
		}

		LocalFree(wszArgList);
	}

	return dwRet;
}
