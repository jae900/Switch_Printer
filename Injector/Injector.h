/**
	@file		Injector.h
	@brief		���μ����� ������ ��Ű�� ���� �ʿ��� �Լ��� ����� ����
*/
#ifndef NTSTATUS
typedef ULONG NTSTATUS;
#endif

typedef NTSTATUS(WINAPI *PFN_NT_CREATE_THREAD_EX) (
	HANDLE*     hThread,
	ACCESS_MASK DesiredAccess,
	LPVOID      ObjectAttributes,
	HANDLE      ProcessHandle,
	LPTHREAD_START_ROUTINE  lpStartAddress,
	LPVOID      lpParameter,
	BOOL        CreateSuspended,
	ULONG       StackZeroBits,
	ULONG       SizeOfStackCommit,
	ULONG       SizeOfStackReserve,
	LPVOID      lpBytesBuffer
	);