#pragma once
#include <windows.h>
#include <winternl.h>

// =============================================================================
// ADVAPI32 — token acquisition
// =============================================================================
WINBASEAPI BOOL WINAPI ADVAPI32$OpenProcessToken(HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);
WINBASEAPI BOOL WINAPI ADVAPI32$DuplicateTokenEx(HANDLE hExistingToken, DWORD dwDesiredAccess, LPSECURITY_ATTRIBUTES lpTokenAttributes, SECURITY_IMPERSONATION_LEVEL ImpersonationLevel, TOKEN_TYPE TokenType, PHANDLE phNewToken);

// =============================================================================
// ADVAPI32 — impersonation
// =============================================================================
WINBASEAPI BOOL WINAPI ADVAPI32$ImpersonateLoggedOnUser(HANDLE hToken);
WINBASEAPI BOOL WINAPI ADVAPI32$RevertToSelf(VOID);

// =============================================================================
// ADVAPI32 — credential-based token creation
// =============================================================================
WINBASEAPI BOOL WINAPI ADVAPI32$LogonUserA(LPCSTR lpszUsername, LPCSTR lpszDomain, LPCSTR lpszPassword, DWORD dwLogonType, DWORD dwLogonProvider, PHANDLE phToken);

// =============================================================================
// ADVAPI32 — token introspection and modification
// =============================================================================
WINBASEAPI BOOL WINAPI ADVAPI32$GetTokenInformation(HANDLE TokenHandle, TOKEN_INFORMATION_CLASS TokenInformationClass, LPVOID TokenInformation, DWORD TokenInformationLength, PDWORD ReturnLength);
WINBASEAPI BOOL WINAPI ADVAPI32$AdjustTokenPrivileges(HANDLE TokenHandle, BOOL DisableAllPrivileges, PTOKEN_PRIVILEGES NewState, DWORD BufferLength, PTOKEN_PRIVILEGES PreviousState, PDWORD ReturnLength);

// =============================================================================
// NTDLL — handle close (rm BOF uses NtClose to close token handle)
// =============================================================================
WINBASEAPI NTSTATUS NTAPI NTDLL$NtClose(HANDLE Handle);
