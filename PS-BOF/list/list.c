#include <windows.h>
#include "bofdefs.h"
#include "beacon.h"
#include "adaptix.h"

#ifndef STATUS_BUFFER_TOO_SMALL
#define STATUS_BUFFER_TOO_SMALL ((NTSTATUS)0xC0000023)
#endif

static WCHAR* GetUserByToken(HANDLE token_handle) {
    TOKEN_USER *token_user_ptr = NULL;
    SID_NAME_USE sid_name = SidTypeUnknown;
    NTSTATUS status;
    WCHAR *user_domain = NULL;
    WCHAR *domain = NULL;
    WCHAR *username = NULL;
    ULONG total_len = 0, return_len = 0, domain_len = 0, username_ln = 0;
    BOOL success = FALSE;

    status = NTDLL$NtQueryInformationToken(token_handle, TokenUser, NULL, 0, &return_len);
    if (status != STATUS_BUFFER_TOO_SMALL)
        goto cleanup;

    token_user_ptr = (TOKEN_USER*)MSVCRT$malloc(return_len);
    if (!token_user_ptr) goto cleanup;

    status = NTDLL$NtQueryInformationToken(token_handle, TokenUser,
        token_user_ptr, return_len, &return_len);
    if (!NT_SUCCESS(status)) goto cleanup;

    ADVAPI32$LookupAccountSidW(NULL, token_user_ptr->User.Sid, NULL,
        &username_ln, NULL, &domain_len, &sid_name);
    if (KERNEL32$GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        goto cleanup;

    total_len = username_ln + domain_len + 2;
    user_domain = (WCHAR*)MSVCRT$malloc(total_len * sizeof(WCHAR));
    if (!user_domain) goto cleanup;

    domain   = (WCHAR*)MSVCRT$malloc(domain_len * sizeof(WCHAR));
    username = (WCHAR*)MSVCRT$malloc(username_ln * sizeof(WCHAR));
    if (!domain || !username) goto cleanup;

    success = ADVAPI32$LookupAccountSidW(NULL, token_user_ptr->User.Sid,
        username, &username_ln, domain, &domain_len, &sid_name);
    if (!success) goto cleanup;

    {
        ULONG di = 0, ui = 0;
        while (di < domain_len && domain[di]) user_domain[di] = domain[di++];
        user_domain[di++] = L'\\';
        while (ui < username_ln && username[ui]) user_domain[di++] = username[ui++];
        user_domain[di] = L'\0';
    }

cleanup:
    if (token_user_ptr) MSVCRT$free(token_user_ptr);
    if (domain)         MSVCRT$free(domain);
    if (username)       MSVCRT$free(username);
    if (!success && user_domain) {
        MSVCRT$free(user_domain);
        user_domain = NULL;
    }
    return user_domain;
}

void go(char *args, int len) {
    SYSTEM_PROCESS_INFORMATION *system_proc_info = NULL;
    PVOID  base_sysproc  = NULL;
    ULONG  return_length = 0;
    NTSTATUS status;
    BOOL   Isx64         = FALSE;
    WCHAR *user_token    = NULL;
    HANDLE token_handle  = NULL;
    HANDLE proc_handle   = NULL;

    NTDLL$NtQuerySystemInformation(SystemProcessInformation, NULL, 0, &return_length);

    system_proc_info = (SYSTEM_PROCESS_INFORMATION*)MSVCRT$malloc(return_length);
    if (!system_proc_info) return;

    status = NTDLL$NtQuerySystemInformation(SystemProcessInformation,
        system_proc_info, return_length, &return_length);
    if (!NT_SUCCESS(status)) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to get system process information, error: %d\n",
                     KERNEL32$GetLastError());
        MSVCRT$free(system_proc_info);
        return;
    }

    base_sysproc = system_proc_info;

    do {
        proc_handle  = NULL;
        token_handle = NULL;
        user_token   = NULL;
        Isx64        = FALSE;

        proc_handle = KERNEL32$OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE,
            HandleToUlong(system_proc_info->UniqueProcessId));

        if (proc_handle) {
            KERNEL32$IsWow64Process(proc_handle, &Isx64);
            if (ADVAPI32$OpenProcessToken(proc_handle, TOKEN_QUERY, &token_handle) && token_handle) {
                user_token = GetUserByToken(token_handle);
                KERNEL32$CloseHandle(token_handle);
            }
            KERNEL32$CloseHandle(proc_handle);
        }

        if (system_proc_info->ImageName.Buffer) {
            BeaconPkgBytes((PBYTE)system_proc_info->ImageName.Buffer,
                           system_proc_info->ImageName.Length, NULL);
        } else {
            BeaconPkgBytes((PBYTE)L"[System]",
                           (ULONG)(wcslen(L"[System]") * sizeof(WCHAR)), NULL);
        }

        BeaconPkgInt32((INT32)HandleToUlong(system_proc_info->UniqueProcessId), NULL);
        BeaconPkgInt32((INT32)HandleToUlong(system_proc_info->InheritedFromUniqueProcessId), NULL);
        BeaconPkgInt32((INT32)system_proc_info->SessionId, NULL);

        if (!user_token) {
            BeaconPkgBytes((PBYTE)L"N/A",
                           (ULONG)(wcslen(L"N/A") * sizeof(WCHAR)), NULL);
        } else {
            BeaconPkgBytes((PBYTE)user_token,
                           (ULONG)(wcslen(user_token) * sizeof(WCHAR)), NULL);
            MSVCRT$free(user_token);
        }

        BeaconPkgInt32((INT32)Isx64, NULL);

        if (system_proc_info->NextEntryOffset == 0)
            break;
        system_proc_info = (SYSTEM_PROCESS_INFORMATION*)((UINT_PTR)system_proc_info + system_proc_info->NextEntryOffset);

    } while (1);

    MSVCRT$free(base_sysproc);
}
