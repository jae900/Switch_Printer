/**
	@file		Hook.cpp
	@brief		���޹��� �Լ��� Trampoline ������� ��ŷ�ϴ� �Լ��� ���ǵ� ����
*/
#include "stdafx.h"
#include "Hook.h"
#include "distorm.h"
#include "mnemonics.h"
#include "Debug.h"

HOOKINFO	g_HookInfo[MAX_FUNCTION] = { 0, };		///< ��ŷ�� �Լ��� ������ ��ϵ� ����ü �迭

/**
	@brief		���ڸ� ���ڷ� �����ϴ� ��ƾ
	@param[in]	���ڷ� ������ ����
	@return		����� ����
	@date		2020.07.13
	@author		Kevin
*/
ULONG_PTR
StrToNumber(IN PCHAR szNumber)
{
	ULONG_PTR	ulRet = 0;
	BOOLEAN		bHex = FALSE;

	for (size_t i = 0; i < strlen(szNumber); i++) {
		if ((szNumber[i] == ']')
			&& ulRet)
		{
			break;
		}

		ulRet = ulRet << 4;				// �� ���ڴ� 1byte(8bits)�� ���̹Ƿ�

		if (szNumber[i] == 'x') {		// 16����
			bHex = TRUE;
			ulRet = 0;
		}
		else if (szNumber[i] == 'n') {	// 10����
			bHex = FALSE;
			ulRet = 0;
		}
		else {
			if (szNumber[i] >= '0' && szNumber[i] <= '9') {
				ulRet += szNumber[i] - '0';
			}
			else if (bHex) {
				if (szNumber[i] >= 'a' && szNumber[i] <= 'f') {
					ulRet += szNumber[i] - 'a' + 0xA;
				}
				else if (szNumber[i] >= 'A' && szNumber[i] <= 'F') {
					ulRet += szNumber[i] - 'A' + 0xA;
				}
			}
		}
	}

	return ulRet;
}

/**
	@brief		������ �Լ��� ��ŷ�ϴ� ��ƾ
	@param[in]	hMod			����ڵ�
	@param[in]	pfnOrgFunction	��ŷ�� �Լ��� �ּ�
	@param[out]	pHookInfo		��ŷ ���� ����ü ������
	@param[in]	pfnNewFunction	��ü�� �� �Լ� �ּ�
	@return		ERROR_SUCCESS	����
	@return		DWORD			�����ڵ�(GetLastError())
	@date		2020.07.13
	@author		Kevin
	@note		Trampoline ��ŷ�� ���� Instruction�� ���� ������ JMP �ڵ忡 ���� ��ŷ ����Ʈ ������ ����\n
				Distorm�̶�� Free library�� �����. BSD ���̼����� ����뵵�ε� ���� ����� �����ϸ�, ���� �� �ش� ���̺귯�� ����� ����ϸ� ��
*/
DWORD
HookFunction(__in HMODULE hMod, __in PVOID pfnOrgFunction, __in PHOOKINFO pHookInfo, __in PVOID pfnNewFunction, __in BOOL bDeepFindJmp)
{
	DWORD			dwRet = ERROR_SUCCESS;
	SIZE_T			nLength;
	_DecodedInst	*pDecodedInst = NULL;
	_DecodeResult	dResult;
	uint32_t		i;

	DEBUG(L"Entering %S", __FUNCTION__);

	do {
		pHookInfo->pfnHookedFunction = pfnOrgFunction;
		// 1. ���� �Լ��� ����� �ڵ带 �����ϱ� ���� �޸𸮸� �Ҵ��Ѵ�.
		pHookInfo->pfnOrgFunction = (PUCHAR)VirtualAllocEx(GetCurrentProcess(), NULL, 50, MEM_COMMIT, PAGE_READWRITE);
		if (pHookInfo->pfnOrgFunction == NULL) {
			dwRet = ERROR_INSUFFICIENT_BUFFER;
			DEBUG(L"Failed to allocate pool for original function code.");
			break;
		}

		// 2. ���� �Լ��� ����� �ڵ带 �Ҵ�� ���۷� �о�´�.
		if (FALSE == ReadProcessMemory(GetCurrentProcess(), pfnOrgFunction, pHookInfo->pfnOrgFunction, 50, &nLength)) {
			dwRet = GetLastError();
			DEBUG(L"Failed to read original function code(%d).", dwRet);
			break;
		}
		
		UCHAR	returnCode[JUMP_CODE] = 
#ifdef _AMD64_
				{ 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0 };
#else
				{ 0xE9, 0x00, 0x00, 0x00, 0x00, };
#endif
		
		// 3. Distorm�� �̿��Ͽ� ����� �ڵ� ������ ���Ѵ�.
		// 3-1. Decode ����� ����� ���� �Ҵ�
		pDecodedInst = (_DecodedInst*)LocalAlloc(LPTR, sizeof(_DecodedInst) * 10);
		if (pDecodedInst == NULL) {
			dwRet = ERROR_INSUFFICIENT_BUFFER;
			DEBUG(L"Failed to allocate pool for decode instruction.");
			break;
		}

		UINT	usedInstCount;
		SIZE_T	nRead;
		SIZE_T	nTmpRead;	// JMP�� ���� Deep Search �� ������ ��� ���� Read ���� �ǵ��� �ش�.
		BOOL	bAgain = TRUE;
		DWORD	dwOldProtect;

		// 3-1. ����� �ڵ带 Ȯ���Ͽ� �ڵ� ��ü(Trampoline ��ŷ) ���ɿ��θ� �Ǵ��Ѵ�.
		// �⺻ 2ȸ
		do {
			bAgain = FALSE;

			dResult = distorm_decode(0, pHookInfo->pfnOrgFunction, static_cast<int>(nLength), DECODETYPE, pDecodedInst, 10, &usedInstCount);
			if ((dResult != DECRES_SUCCESS)
				&& (dResult != DECRES_MEMORYERR))
			{
				dwRet = dResult;
				DEBUG(L"Failed to decode(%d).", dwRet);
				break;
			}

			DEBUG(L"distorm_decode(%d)", usedInstCount);

			nRead = 0;
			nTmpRead = 0;
			for (i = 0; i < usedInstCount; i++) {
				DEBUG(L"Instruction : %S - %S/%S(%d/%d)", pDecodedInst[i].instructionHex.p, pDecodedInst[i].mnemonic.p, pDecodedInst[i].operands.p, pDecodedInst[i].size, nRead);
				// JMP �ڵ� Ȯ��
				if (strncmp((char*)&pDecodedInst[i].mnemonic.p, "JMP", 3) == 0) {
					DEBUG(L"Address : " POINTER, pHookInfo->pfnHookedFunction);
#ifdef _AMD64_
					if (strncmp((char*)pDecodedInst[i].instructionHex.p, "48ff25", 6) == 0) {
						pHookInfo->pfnHookedFunction = (PVOID)((ULONG_PTR)pHookInfo->pfnHookedFunction + pDecodedInst[i].size + StrToNumber((char*)pDecodedInst[i].operands.p));
						pHookInfo->pfnHookedFunction = *(PVOID*)pHookInfo->pfnHookedFunction;
					
						if (FALSE == ReadProcessMemory(GetCurrentProcess(), pHookInfo->pfnHookedFunction, pHookInfo->pfnOrgFunction, 50, &nLength)) {
							dwRet = GetLastError();
							DEBUG(L"Failed to read process memory(%d).", dwRet);
							break;
						}
						bAgain = TRUE;
					}
					else if (strncmp((char*)pDecodedInst[i].instructionHex.p, "eb", 2) == 0) {
						pHookInfo->pfnHookedFunction = (PVOID)((ULONG_PTR)pHookInfo->pfnHookedFunction + StrToNumber((char*)pDecodedInst[i].operands.p));
						
						if (FALSE == ReadProcessMemory(GetCurrentProcess(), pHookInfo->pfnHookedFunction, pHookInfo->pfnOrgFunction, 50, &nLength)) {
							dwRet = GetLastError();
							DEBUG(L"Failed to read process memory(%d).", dwRet);
							break;
						}
						bAgain = TRUE;
					}
					else 
#endif
					if (strncmp((char*)pDecodedInst[i].instructionHex.p, "ff25", 4) == 0) {
						DEBUG(L"0xFF25 - BaseAddress:" POINTER L", instructionLength:%d, %x", pHookInfo->pfnHookedFunction, pDecodedInst[i].size, StrToNumber((char*)pDecodedInst[i].operands.p));
#ifdef _AMD64_
						pHookInfo->pfnHookedFunction = (PVOID)((ULONG_PTR)pHookInfo->pfnHookedFunction + pDecodedInst[i].size + StrToNumber((char*)pDecodedInst[i].operands.p));
						pHookInfo->pfnHookedFunction = *(PVOID*)pHookInfo->pfnHookedFunction;
#else
						pHookInfo->pfnHookedFunction = (PVOID)(ULONG_PTR)StrToNumber((char*)pDecodedInst[i].operands.p);
						pHookInfo->pfnHookedFunction = *(PVOID*)pHookInfo->pfnHookedFunction;
#endif
					
						if (FALSE == ReadProcessMemory(GetCurrentProcess(), pHookInfo->pfnHookedFunction, pHookInfo->pfnOrgFunction, 50, &nLength)) {
							dwRet = GetLastError();
							DEBUG(L"Failed to read process memory(%d).", dwRet);
							break;
						}
						bAgain = TRUE;
					}
					else if (strncmp((char*)pDecodedInst[i].instructionHex.p, "e9", 2) == 0) {
						DEBUG(L"0xE9 - BaseAddress:" POINTER L", instructionLength:%d, %x", pHookInfo->pfnHookedFunction, pDecodedInst[i].size, StrToNumber((char*)pDecodedInst[i].operands.p));

						pHookInfo->pfnHookedFunction = (PVOID)((ULONG_PTR)pHookInfo->pfnHookedFunction + StrToNumber((char*)pDecodedInst[i].operands.p));

						if (FALSE == ReadProcessMemory(GetCurrentProcess(), pHookInfo->pfnHookedFunction, pHookInfo->pfnOrgFunction, 50, &nLength)) {
							dwRet = GetLastError();
							DEBUG(L"Failed to read process memory(%d).", dwRet);
							break;
						}
						bAgain = TRUE;
					}

					if (bAgain) {
						break;
					}
				} // MOV �ڵ� Ȯ��(JMP �ڵ忡 ������ �ֱ� ������ Ȯ���� �ʿ���
				else
#ifdef _AMD64_
				if (strncmp((char*)pDecodedInst[i].mnemonic.p, "CALL", 4) == 0) {
					dwRet = ERROR_NOT_SUPPORTED;
					break;
				} else if (strncmp((char*)pDecodedInst[i].mnemonic.p, "MOV", 3) == 0) {
					if ((strncmp((char*)pDecodedInst[i].operands.p, "RAX", 3) == 0)
						|| (strncmp((char*)pDecodedInst[i].operands.p, "[RAX", 3) == 0)
						|| (strncmp((char*)pDecodedInst[i].operands.p, "EAX", 3) == 0))
					{
						// MOV RAX, RSP�� �Լ��� ������
						// RAX ��� �Ұ�, rcx, rdx, r8, r9�� �Լ��� �Ķ���ͷ� ����
						// RAX�� ��� ���Ŀ� ���� �� ���� ����
						nTmpRead = 0;
					}
				}
				// OpenPrinter2W�� �ڵ带 ���� Ȯ���Ͽ� ó����
				// RBX�� JUMP �ڵ�� ����ϱ� ���� �ش� Instruction�� Ȯ��
				else if (strncmp((char*)pDecodedInst[i].mnemonic.p, "PUSH", 4) == 0) {
					if ((strncmp((char*)pDecodedInst[i].operands.p, "RBX", 3) == 0)
						&& nTmpRead)
					{
						nTmpRead = 0;
						// RBX ���
						returnCode[1] = 0xBB;
						returnCode[11] = 0xE3;
					}
				}
#endif

				nRead += pDecodedInst[i].size;
				if (nRead >= JUMP_CODE) {
					if (bDeepFindJmp == FALSE) {		// Jmp Deep Search�� ������� ���� ���
						break;
					} else {
						if (nTmpRead == 0) {
							nTmpRead = nRead;
						}
					}
				}
			} // for (int i = 0; i < usedInstCount; i++) {

			if (dwRet == ERROR_NOT_SUPPORTED) {
				break;
			}

			if ((nRead >= JUMP_CODE)
				&& (bAgain == FALSE))
			{
				ULONG_PTR	temp = (ULONG_PTR)pHookInfo->pfnHookedFunction;	// ���� ��ġ
																			// ����� ��ƾ�� JMP �ڵ� ����
				DEBUG(L"nTmpRead : %d(nRead : %d)", nTmpRead, nRead);
				if (nTmpRead != 0) {
					nRead = nTmpRead;
				}
#ifdef _AMD64_
				temp += nRead;

				RtlCopyMemory(&returnCode[2], &temp, sizeof(ULONG_PTR));
#else
				temp -= (ULONG_PTR)pHookInfo->pfnOrgFunction + JUMP_CODE;
				RtlCopyMemory(&returnCode[1], &temp, sizeof(ULONG));
#endif
				DEBUG(L"Write Memory : " POINTER L" -> " POINTER, temp, pHookInfo->pfnOrgFunction);

				if (FALSE == WriteProcessMemory(GetCurrentProcess(), &pHookInfo->pfnOrgFunction[nRead], returnCode, JUMP_CODE, NULL)) {
					dwRet = GetLastError();
				}
				VirtualProtect(pHookInfo->pfnOrgFunction, nRead + JUMP_CODE, PAGE_EXECUTE, &dwOldProtect);

				FlushInstructionCache(GetCurrentProcess(), pHookInfo->pfnOrgFunction, nRead + JUMP_CODE);

				// returnCode ����
				temp = (ULONG_PTR)pfnNewFunction;

				DEBUG(L"New Function : " POINTER, temp);

#ifdef _AMD64_
				returnCode[1] = 0xB8;
				returnCode[11] = 0xE0;

				if (((PUCHAR)temp)[0] == 0xE9) {
					temp += *(PULONG)&((PUCHAR)temp)[1];
					temp += 5;
				}

				RtlCopyMemory(&returnCode[2], &temp, sizeof(ULONG_PTR));
#else
				if (((PUCHAR)temp)[0] == 0xE9) {
					temp += *(PULONG)&((PUCHAR)temp)[1];
				}
				temp -= (ULONG_PTR)pHookInfo->pfnHookedFunction;

				RtlCopyMemory(&returnCode[1], &temp, sizeof(ULONG));
#endif
				DEBUG(L"Write Memory(Original) : " POINTER L" -> " POINTER, temp, pHookInfo->pfnHookedFunction);

				// Original ��ƾ �ּҿ� JUMP �ڵ� ����
				VirtualProtect(pHookInfo->pfnHookedFunction, JUMP_CODE, PAGE_EXECUTE_READWRITE, &dwOldProtect);
				if (FALSE == WriteProcessMemory(GetCurrentProcess(), pHookInfo->pfnHookedFunction, returnCode, JUMP_CODE, NULL)) {
					dwRet = GetLastError();
				}
				VirtualProtect(pHookInfo->pfnHookedFunction, JUMP_CODE, dwOldProtect, &dwOldProtect);

				// 
				FlushInstructionCache(GetCurrentProcess(), pHookInfo->pfnHookedFunction, JUMP_CODE);
				break;
			}
		} while (bAgain);
	} while (FALSE);

	if (pDecodedInst) {
		LocalFree(pDecodedInst);
	}

	DEBUG(L"Leaving %S", __FUNCTION__);

	return dwRet;
}

/**
	@brief		�Լ��� ��ŷ�ϴ� ��ƾ
	@param[in]	lpModuleName		����̸�
	@param[in]	lpFunctionName		�Լ� �̸�
	@param[inout]	pHookInfo		��ŷ ������ ����� ����ü ������
	@param[in]	pNewFunction		���� ���ǵ� �Լ�
	@return		ERROR_SUCCESS		����
	@return		DWORD				GetLastError(), �����ڵ�
	@date		2020.07.08
	@author		Kevin
*/
DWORD
HookFunction(__in LPCWSTR lpModuleName, __in LPCSTR lpFunctionName, __inout PHOOKINFO pHookInfo, __in PVOID pNewFunction, __in BOOL bDeepFindJmp)
{
	DWORD			dwRet = ERROR_SUCCESS;
	HMODULE			hMod;
	PVOID			pfnOrgFunction;

	DEBUG(L"Entering %S(%S)", __FUNCTION__, lpFunctionName);

	do {
		hMod = GetModuleHandle(lpModuleName);
		if (hMod == NULL) {
			dwRet = GetLastError();
			DEBUG(L"Failed to get module handle(%d).", dwRet);
			break;
		}

		pfnOrgFunction = GetProcAddress(hMod, lpFunctionName);
		if (pfnOrgFunction == NULL) {
			dwRet = GetLastError();
			DEBUG(L"Failed to get procedure(%d).", dwRet);
			break;
		}

		dwRet = HookFunction(hMod, pfnOrgFunction, pHookInfo, pNewFunction, bDeepFindJmp);
	} while (FALSE);

	DEBUG(L"Leaving %S", __FUNCTION__);

	return dwRet;
}

/**
	@brief		��ŷ�� �Լ��� ������ �Լ��� �����ϴ� ��ƾ
	@date		2020.07.14
	@author		Kevin
*/
DWORD
UnhookFunction(__in PHOOKINFO	pHookInfo)
{
	DWORD	dwRet = ERROR_SUCCESS;
	DWORD	dwOldProtect;
	
	DEBUG(L"Entering %S(" POINTER L")", __FUNCTION__, pHookInfo->pfnHookedFunction);

	VirtualProtect(pHookInfo->pfnHookedFunction, JUMP_CODE, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	if (FALSE == WriteProcessMemory(GetCurrentProcess(), pHookInfo->pfnHookedFunction, pHookInfo->pfnOrgFunction, JUMP_CODE, NULL)) {
		dwRet = GetLastError();
		DEBUG(L"Failed to write original instructions to process memory(%d).", dwRet);
	}
	VirtualProtect(pHookInfo->pfnHookedFunction, JUMP_CODE, dwOldProtect, &dwOldProtect);

	FlushInstructionCache(GetCurrentProcess(), pHookInfo->pfnHookedFunction, JUMP_CODE);

	DEBUG(L"Leaving %S", __FUNCTION__);
	
	return dwRet;
}