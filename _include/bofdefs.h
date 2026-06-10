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
#include <tlhelp32.h>

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
// KERNEL32 — process management (steal BOF)
// =============================================================================
WINBASEAPI HANDLE WINAPI KERNEL32$OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);

// =============================================================================
// MSVCRT (used by base.c shared by all FS-BOF, and by fserror.h fallback)
// =============================================================================
WINBASEAPI void *__cdecl MSVCRT$calloc(size_t _NumOfElements, size_t _SizeOfElements);
WINBASEAPI void *__cdecl MSVCRT$malloc(size_t _Size);
WINBASEAPI void __cdecl MSVCRT$free(void *_Memory);
WINBASEAPI void * __cdecl MSVCRT$memset(void *dest, int c, size_t count);
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
// KERNEL32 — process creation (PS-BOF run)
// =============================================================================
WINBASEAPI BOOL   WINAPI KERNEL32$CreateProcessW(LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
WINBASEAPI HANDLE WINAPI KERNEL32$GetStdHandle(DWORD nStdHandle);
WINBASEAPI BOOL   WINAPI KERNEL32$CreatePipe(PHANDLE, PHANDLE, LPSECURITY_ATTRIBUTES, DWORD);
WINBASEAPI BOOL   WINAPI KERNEL32$SetHandleInformation(HANDLE, DWORD, DWORD);
WINBASEAPI BOOL   WINAPI KERNEL32$InitializeProcThreadAttributeList(LPPROC_THREAD_ATTRIBUTE_LIST, DWORD, DWORD, PSIZE_T);
WINBASEAPI BOOL   WINAPI KERNEL32$UpdateProcThreadAttribute(LPPROC_THREAD_ATTRIBUTE_LIST, DWORD, DWORD_PTR, PVOID, SIZE_T, PVOID, PSIZE_T);
WINBASEAPI VOID   WINAPI KERNEL32$DeleteProcThreadAttributeList(LPPROC_THREAD_ATTRIBUTE_LIST);
WINBASEAPI BOOL   WINAPI KERNEL32$DuplicateHandle(HANDLE, HANDLE, HANDLE, LPHANDLE, DWORD, BOOL, DWORD);

// =============================================================================
// ADVAPI32 — process creation with credentials/token (PS-BOF run)
// =============================================================================
WINADVAPI BOOL WINAPI ADVAPI32$CreateProcessWithLogonW(LPCWSTR, LPCWSTR, LPCWSTR, DWORD, LPCWSTR, LPWSTR, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
WINADVAPI BOOL WINAPI ADVAPI32$CreateProcessWithTokenW(HANDLE, DWORD, LPCWSTR, LPWSTR, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);

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
// KERNEL32 — toolhelp (PS-BOF grep)
// =============================================================================
WINBASEAPI HANDLE WINAPI KERNEL32$CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID);
WINBASEAPI BOOL   WINAPI KERNEL32$Thread32First(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
WINBASEAPI BOOL   WINAPI KERNEL32$Thread32Next(HANDLE hSnapshot, LPTHREADENTRY32 lpte);

// =============================================================================
// NTDLL — process info (PS-BOF grep)
// =============================================================================
WINBASEAPI NTSTATUS NTAPI NTDLL$NtQueryInformationProcess(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);

// =============================================================================
// Helper macros (used by base.c shared across FS-BOF sources)
// =============================================================================
#define intAlloc(size) KERNEL32$HeapAlloc(KERNEL32$GetProcessHeap(), HEAP_ZERO_MEMORY, size)
#define intRealloc(ptr, size) (ptr) ? KERNEL32$HeapReAlloc(KERNEL32$GetProcessHeap(), HEAP_ZERO_MEMORY, ptr, size) : KERNEL32$HeapAlloc(KERNEL32$GetProcessHeap(), HEAP_ZERO_MEMORY, size)
#define intFree(addr) KERNEL32$HeapFree(KERNEL32$GetProcessHeap(), 0, addr)
#define intZeroMemory(addr,size) MSVCRT$memset((addr),0,size)

// =============================================================================
// CRYPT32 — data protection (Postex-BOF)
// =============================================================================
#include <wincrypt.h>
WINADVAPI BOOL WINAPI CRYPT32$CryptStringToBinaryA(LPCSTR pszString, DWORD cchString, DWORD dwFlags, BYTE *pbBinary, DWORD *pcbBinary, DWORD *pdwSkip, DWORD *pdwFlags);
WINADVAPI BOOL WINAPI CRYPT32$CryptUnprotectData(DATA_BLOB *pDataIn, LPWSTR *ppszDataDescr, DATA_BLOB *pOptionalEntropy, PVOID pvReserved, void *pPromptStruct, DWORD dwFlags, DATA_BLOB *pDataOut);

// =============================================================================
// ADVAPI32 — registry (Postex-BOF)
// =============================================================================
WINADVAPI LSTATUS WINAPI ADVAPI32$RegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
WINADVAPI LSTATUS WINAPI ADVAPI32$RegCloseKey(HKEY hKey);
WINADVAPI LSTATUS WINAPI ADVAPI32$RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);

// =============================================================================
// KERNEL32 — ASCII process/file ops (Postex-BOF)
// =============================================================================
WINBASEAPI BOOL WINAPI KERNEL32$CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
WINBASEAPI DWORD WINAPI KERNEL32$SearchPathA(LPCSTR lpPath, LPCSTR lpFileName, LPCSTR lpExtension, DWORD nBufferLength, LPSTR lpBuffer, LPSTR *lpFilePart);
WINBASEAPI DWORD WINAPI KERNEL32$GetFileAttributesA(LPCSTR lpFileName);

// =============================================================================
// MSVCRT — string operations (Postex-BOF)
// =============================================================================
WINBASEAPI int __cdecl MSVCRT$strcmp(const char *_Str1, const char *_Str2);
WINBASEAPI size_t __cdecl MSVCRT$strlen(const char *str);
WINBASEAPI char * __cdecl MSVCRT$strcat(char *dest, const char *src);
WINBASEAPI char * __cdecl MSVCRT$strcpy(char *dest, const char *src);
WINBASEAPI void * __cdecl MSVCRT$memcpy(void *dest, const void *src, size_t n);
WINBASEAPI void * __cdecl MSVCRT$realloc(void *ptr, size_t size);
WINBASEAPI char * __cdecl MSVCRT$strstr(const char *haystack, const char *needle);
WINBASEAPI char * __cdecl MSVCRT$strchr(const char *s, int c);

// =============================================================================
// Standard-name aliases for Postex-BOF (cs_veeam_dumper uses undecorated names)
// =============================================================================
#define strcmp              MSVCRT$strcmp
#define strlen              MSVCRT$strlen
#define strcat              MSVCRT$strcat
#define strcpy              MSVCRT$strcpy
#define memcpy              MSVCRT$memcpy
#define realloc             MSVCRT$realloc
#define free                MSVCRT$free
#define malloc              MSVCRT$malloc
#define strstr              MSVCRT$strstr
#define strchr              MSVCRT$strchr
#define SearchPathA         KERNEL32$SearchPathA
#define GetFileAttributesA  KERNEL32$GetFileAttributesA
#define RegOpenKeyExA       ADVAPI32$RegOpenKeyExA
#define RegCloseKey         ADVAPI32$RegCloseKey
#define RegQueryValueExA    ADVAPI32$RegQueryValueExA
#define CreatePipe          KERNEL32$CreatePipe
#define CreateProcessA      KERNEL32$CreateProcessA
#define SetHandleInformation KERNEL32$SetHandleInformation
#define ReadFile            KERNEL32$ReadFile
#define CloseHandle         KERNEL32$CloseHandle
#define CryptStringToBinaryA CRYPT32$CryptStringToBinaryA
#define CryptUnprotectData  CRYPT32$CryptUnprotectData
#define GetLastError        KERNEL32$GetLastError
#define GetProcessHeap      KERNEL32$GetProcessHeap
#define HeapAlloc           KERNEL32$HeapAlloc
#define HeapFree            KERNEL32$HeapFree
#define WideCharToMultiByte KERNEL32$WideCharToMultiByte
