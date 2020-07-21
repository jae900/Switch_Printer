/**
	@file		Hook.cpp
	@brief		전달받은 함수에 Trampoline 방식으로 후킹하는 함수가 정의된 파일
*/
#include "stdafx.h"
#include "Hook.h"
#include "distorm.h"
#include "mnemonics.h"
#include "Debug.h"

HOOKINFO	g_HookInfo[MAX_FUNCTION] = { 0, };		///< 후킹된 함수의 정보가 기록된 구조체 배열

/**
	@brief		문자를 숫자로 변경하는 루틴
	@param[in]	숫자로 변경할 문자
	@return		변경된 숫자
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

		ulRet = ulRet << 4;				// 한 문자는 1byte(8bits)의 반이므로

		if (szNumber[i] == 'x') {		// 16진수
			bHex = TRUE;
			ulRet = 0;
		}
		else if (szNumber[i] == 'n') {	// 10진수
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
	@brief		실제로 함수에 후킹하는 루틴
	@param[in]	hMod			모듈핸들
	@param[in]	pfnOrgFunction	후킹할 함수의 주소
	@param[out]	pHookInfo		후킹 정보 구조체 포인터
	@param[in]	pfnNewFunction	대체할 새 함수 주소
	@return		ERROR_SUCCESS	성공
	@return		DWORD			에러코드(GetLastError())
	@date		2020.07.13
	@author		Kevin
	@note		Trampoline 후킹을 위해 Instruction의 길이 측정과 JMP 코드에 의한 후킹 포인트 갱신을 위해\n
				Distorm이라는 Free library를 사용함. BSD 라이센스로 상업용도로도 무료 사용이 가능하며, 배포 시 해당 라이브러리 사용을 명시하면 됨
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
		// 1. 원래 함수의 어셈블리 코드를 복사하기 위한 메모리를 할당한다.
		pHookInfo->pfnOrgFunction = (PUCHAR)VirtualAllocEx(GetCurrentProcess(), NULL, 50, MEM_COMMIT, PAGE_READWRITE);
		if (pHookInfo->pfnOrgFunction == NULL) {
			dwRet = ERROR_INSUFFICIENT_BUFFER;
			DEBUG(L"Failed to allocate pool for original function code.");
			break;
		}

		// 2. 원래 함수의 어셈블리 코드를 할당된 버퍼로 읽어온다.
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
		
		// 3. Distorm을 이용하여 어셈블리 코드 정보를 구한다.
		// 3-1. Decode 결과가 저장될 버퍼 할당
		pDecodedInst = (_DecodedInst*)LocalAlloc(LPTR, sizeof(_DecodedInst) * 10);
		if (pDecodedInst == NULL) {
			dwRet = ERROR_INSUFFICIENT_BUFFER;
			DEBUG(L"Failed to allocate pool for decode instruction.");
			break;
		}

		UINT	usedInstCount;
		SIZE_T	nRead;
		SIZE_T	nTmpRead;	// JMP에 대한 Deep Search 가 실패할 경우 이전 Read 값을 되돌려 준다.
		BOOL	bAgain = TRUE;
		DWORD	dwOldProtect;

		// 3-1. 어셈블리 코드를 확인하여 코드 대체(Trampoline 후킹) 가능여부를 판단한다.
		// 기본 2회
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
				// JMP 코드 확인
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
				} // MOV 코드 확인(JMP 코드에 영향을 주기 때문에 확인이 필요함
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
						// MOV RAX, RSP로 함수가 시작함
						// RAX 사용 불가, rcx, rdx, r8, r9는 함수의 파라메터로 사용됨
						// RAX의 경우 이후에 재사용 할 수도 있음
						nTmpRead = 0;
					}
				}
				// OpenPrinter2W의 코드를 직접 확인하여 처리함
				// RBX를 JUMP 코드로 사용하기 위해 해당 Instruction을 확인
				else if (strncmp((char*)pDecodedInst[i].mnemonic.p, "PUSH", 4) == 0) {
					if ((strncmp((char*)pDecodedInst[i].operands.p, "RBX", 3) == 0)
						&& nTmpRead)
					{
						nTmpRead = 0;
						// RBX 사용
						returnCode[1] = 0xBB;
						returnCode[11] = 0xE3;
					}
				}
#endif

				nRead += pDecodedInst[i].size;
				if (nRead >= JUMP_CODE) {
					if (bDeepFindJmp == FALSE) {		// Jmp Deep Search를 사용하지 않을 경우
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
				ULONG_PTR	temp = (ULONG_PTR)pHookInfo->pfnHookedFunction;	// 목적 위치
																			// 복사된 루틴의 JMP 코드 삽입
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

				// returnCode 변경
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

				// Original 루틴 주소에 JUMP 코드 삽입
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
	@brief		함수에 후킹하는 루틴
	@param[in]	lpModuleName		모듈이름
	@param[in]	lpFunctionName		함수 이름
	@param[inout]	pHookInfo		후킹 정보가 저장된 구조체 포인터
	@param[in]	pNewFunction		새로 정의된 함수
	@return		ERROR_SUCCESS		성공
	@return		DWORD				GetLastError(), 에러코드
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
	@brief		후킹된 함수를 원래의 함수로 변경하는 루틴
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