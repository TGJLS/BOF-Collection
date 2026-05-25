#pragma once
#include "../_include/bofdefs.h"
#include <windows.h>
#include <winternl.h>

// =============================================================================
// ADVAPI32 — token acquisition
// =============================================================================
WINADVAPI BOOL WINAPI ADVAPI32$DuplicateTokenEx(HANDLE hExistingToken, DWORD dwDesiredAccess, LPSECURITY_ATTRIBUTES lpTokenAttributes, SECURITY_IMPERSONATION_LEVEL ImpersonationLevel, TOKEN_TYPE TokenType, PHANDLE phNewToken);

// =============================================================================
// ADVAPI32 — impersonation
// =============================================================================
WINADVAPI BOOL WINAPI ADVAPI32$ImpersonateLoggedOnUser(HANDLE hToken);
WINADVAPI BOOL WINAPI ADVAPI32$RevertToSelf(VOID);

// =============================================================================
// ADVAPI32 — credential-based token creation
// =============================================================================
WINADVAPI BOOL WINAPI ADVAPI32$LogonUserW(LPCWSTR lpszUsername, LPCWSTR lpszDomain, LPCWSTR lpszPassword, DWORD dwLogonType, DWORD dwLogonProvider, PHANDLE phToken);

// =============================================================================
// ADVAPI32 — token introspection and modification
// =============================================================================
WINADVAPI BOOL WINAPI ADVAPI32$GetTokenInformation(HANDLE TokenHandle, TOKEN_INFORMATION_CLASS TokenInformationClass, LPVOID TokenInformation, DWORD TokenInformationLength, PDWORD ReturnLength);

// =============================================================================
// ADVAPI32 — thread token
// =============================================================================
WINADVAPI BOOL WINAPI ADVAPI32$OpenThreadToken(HANDLE ThreadHandle, DWORD DesiredAccess, BOOL OpenAsSelf, PHANDLE TokenHandle);

// =============================================================================
// KERNEL32 — pseudo-handles
// =============================================================================
WINBASEAPI HANDLE WINAPI KERNEL32$GetCurrentThread(VOID);

// =============================================================================
// NTDLL — handle close (rm BOF uses NtClose to close token handle)
// =============================================================================
WINBASEAPI NTSTATUS NTAPI NTDLL$NtClose(HANDLE Handle);
