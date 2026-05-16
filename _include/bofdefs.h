#pragma once
#pragma intrinsic(memcmp, memcpy, strcpy, strcmp, _stricmp, strlen)

/* i686-w64-mingw32 toolchain fix: older/variant unknwn.h installations fail to
 * define BEGIN_INTERFACE / END_INTERFACE before the COM struct-vtable block.
 * These macros are empty on non-IA64 targets in the official Win32 SDK, so it
 * is safe to provide them as empty fallbacks if the toolchain has not already. */
#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif
#ifndef END_INTERFACE
#define END_INTERFACE
#endif

#include <windows.h>
#include <winternl.h>
#include <psapi.h>

// =============================================================================
// KERNEL32 — memory management
// =============================================================================
WINBASEAPI void * WINAPI KERNEL32$HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
WINBASEAPI LPVOID WINAPI KERNEL32$HeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes);
WINBASEAPI HANDLE WINAPI KERNEL32$GetProcessHeap();
WINBASEAPI BOOL WINAPI KERNEL32$HeapFree(HANDLE, DWORD, PVOID);

// =============================================================================
// KERNEL32 — string / conversion
// =============================================================================
WINBASEAPI int WINAPI KERNEL32$WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWCH lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCCH lpDefaultChar, LPBOOL lpUsedDefaultChar);
WINBASEAPI int WINAPI KERNEL32$lstrlenW(LPCWSTR lpString);

// =============================================================================
// KERNEL32 — error handling
// =============================================================================
WINBASEAPI DWORD WINAPI KERNEL32$GetLastError(VOID);
WINBASEAPI DWORD WINAPI KERNEL32$FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list *Arguments);
DECLSPEC_IMPORT HLOCAL WINAPI KERNEL32$LocalFree(HLOCAL);

// =============================================================================
// KERNEL32 — file I/O (type BOF)
// =============================================================================
WINBASEAPI HANDLE WINAPI KERNEL32$CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
WINBASEAPI WINBOOL WINAPI KERNEL32$ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
WINBASEAPI WINBOOL WINAPI KERNEL32$CloseHandle(HANDLE hObject);

// =============================================================================
// KERNEL32 — directory operations (mkdir, rmdir, cd, pwd BOFs)
// =============================================================================
WINBASEAPI WINBOOL WINAPI KERNEL32$CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
WINBASEAPI WINBOOL WINAPI KERNEL32$RemoveDirectoryW(LPCWSTR lpPathName);
WINBASEAPI DWORD WINAPI KERNEL32$GetCurrentDirectoryW(DWORD nBufferLength, LPWSTR lpBuffer);
WINBASEAPI WINBOOL WINAPI KERNEL32$SetCurrentDirectoryW(LPCWSTR lpPathName);

// =============================================================================
// KERNEL32 — file attributes / find (copy, move, del BOFs)
// =============================================================================
WINBASEAPI DWORD WINAPI KERNEL32$GetFileAttributesW(LPCWSTR lpFileName);
WINBASEAPI HANDLE WINAPI KERNEL32$FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData);
WINBASEAPI WINBOOL WINAPI KERNEL32$FindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData);
WINBASEAPI WINBOOL WINAPI KERNEL32$FindClose(HANDLE hFindFile);

// =============================================================================
// KERNEL32 — file operations (copy, move, del BOFs)
// =============================================================================
WINBASEAPI WINBOOL WINAPI KERNEL32$CopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists);
WINBASEAPI WINBOOL WINAPI KERNEL32$MoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags);
WINBASEAPI WINBOOL WINAPI KERNEL32$DeleteFileW(LPCWSTR lpFileName);

// =============================================================================
// MSVCRT (used by base.c shared by all FS-BOF, and by fserror.h fallback)
// =============================================================================
WINBASEAPI void *__cdecl MSVCRT$calloc(size_t _NumOfElements, size_t _SizeOfElements);
WINBASEAPI void *__cdecl MSVCRT$malloc(size_t _Size);
WINBASEAPI void __cdecl MSVCRT$free(void *_Memory);
WINBASEAPI int __cdecl MSVCRT$vsnprintf(char * __restrict__ d, size_t n, const char * __restrict__ format, va_list arg);
WINBASEAPI int __cdecl MSVCRT$_snprintf(char * __restrict__ _Dest, size_t _Count, const char * __restrict__ _Format, ...);

// =============================================================================
// NTDLL (Exit BOFs only)
// =============================================================================
WINBASEAPI VOID NTAPI NTDLL$RtlExitUserProcess(NTSTATUS Status);
WINBASEAPI VOID NTAPI NTDLL$RtlExitUserThread(NTSTATUS Status);

// =============================================================================
// NTDLL — process control (PS-BOF)
// =============================================================================
WINBASEAPI NTSTATUS NTAPI NTDLL$NtQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
WINBASEAPI NTSTATUS NTAPI NTDLL$NtSuspendProcess(HANDLE ProcessHandle);
WINBASEAPI NTSTATUS NTAPI NTDLL$NtResumeProcess(HANDLE ProcessHandle);

// =============================================================================
// KERNEL32 — process management (PS-BOF)
// =============================================================================
WINBASEAPI HANDLE WINAPI KERNEL32$OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
WINBASEAPI BOOL WINAPI KERNEL32$TerminateProcess(HANDLE hProcess, UINT uExitCode);
WINBASEAPI BOOL WINAPI KERNEL32$IsWow64Process(HANDLE hProcess, PBOOL Wow64Process);
WINBASEAPI HANDLE WINAPI KERNEL32$GetCurrentProcess(VOID);

// =============================================================================
// ADVAPI32 — token / privilege (PS-BOF)
// =============================================================================
WINADVAPI BOOL WINAPI ADVAPI32$OpenProcessToken(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);
WINADVAPI BOOL WINAPI ADVAPI32$LookupAccountSidW(LPCWSTR lpSystemName, PSID Sid, LPWSTR Name, LPDWORD cchName, LPWSTR ReferencedDomainName, LPDWORD cchReferencedDomainName, PSID_NAME_USE peUse);
WINADVAPI BOOL WINAPI ADVAPI32$AdjustTokenPrivileges(HANDLE TokenHandle, BOOL DisableAllPrivileges, PTOKEN_PRIVILEGES NewState, DWORD BufferLength, PTOKEN_PRIVILEGES PreviousState, PDWORD ReturnLength);
WINADVAPI BOOL WINAPI ADVAPI32$LookupPrivilegeValueW(LPCWSTR lpSystemName, LPCWSTR lpName, PLUID lpLuid);

// =============================================================================
// NTDLL — token query (PS-BOF)
// =============================================================================
WINBASEAPI NTSTATUS NTAPI NTDLL$NtQueryInformationToken(HANDLE TokenHandle, TOKEN_INFORMATION_CLASS TokenInformationClass, PVOID TokenInformation, ULONG TokenInformationLength, PULONG ReturnLength);

// =============================================================================
// PSAPI — module enumeration (PS-BOF grep)
// =============================================================================
WINBASEAPI WINBOOL WINAPI PSAPI$EnumProcessModulesEx(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded, DWORD dwFilterFlag);
WINBASEAPI DWORD   WINAPI PSAPI$GetModuleFileNameExW(HANDLE hProcess, HMODULE hModule, LPWSTR lpFilename, DWORD nSize);
WINBASEAPI WINBOOL WINAPI PSAPI$GetModuleInformation(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb);

// =============================================================================
// Helper macros (used by base.c shared across FS-BOF sources)
// =============================================================================
#define intAlloc(size) KERNEL32$HeapAlloc(KERNEL32$GetProcessHeap(), HEAP_ZERO_MEMORY, size)
#define intRealloc(ptr, size) (ptr) ? KERNEL32$HeapReAlloc(KERNEL32$GetProcessHeap(), HEAP_ZERO_MEMORY, ptr, size) : KERNEL32$HeapAlloc(KERNEL32$GetProcessHeap(), HEAP_ZERO_MEMORY, size)
#define intFree(addr) KERNEL32$HeapFree(KERNEL32$GetProcessHeap(), 0, addr)
#define intZeroMemory(addr,size) MSVCRT$memset((addr),0,size)
