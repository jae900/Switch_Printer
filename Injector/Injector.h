/**
	@file		Injector.h
	@brief		프로세스에 인젝션 시키기 위해 필요한 함수가 선언된 파일
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