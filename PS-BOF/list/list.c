#include <windows.h>
#include "bofdefs.h"
#include "beacon.h"

#ifndef STATUS_BUFFER_TOO_SMALL
#define STATUS_BUFFER_TOO_SMALL ((NTSTATUS)0xC0000023)
#endif
#ifndef STATUS_INFO_LENGTH_MISMATCH
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004)
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

static char* WideToUtf8(LPCWSTR wide, int wide_chars) {
    int needed = KERNEL32$WideCharToMultiByte(CP_UTF8, 0, wide, wide_chars, NULL, 0, NULL, NULL);
    if (needed <= 0) return NULL;
    char *buf = (char*)MSVCRT$malloc(needed + 1);
    if (!buf) return NULL;
    KERNEL32$WideCharToMultiByte(CP_UTF8, 0, wide, wide_chars, buf, needed, NULL, NULL);
    buf[needed] = '\0';
    return buf;
}

void go(char *args, int len) {
    SYSTEM_PROCESS_INFORMATION *system_proc_info = NULL;
    PVOID  base_sysproc  = NULL;
    ULONG  return_length = 0;
    NTSTATUS status;
    BOOL   IsWow64         = FALSE;
    WCHAR *user_token    = NULL;
    HANDLE token_handle  = NULL;
    HANDLE proc_handle   = NULL;

    do {
        NTDLL$NtQuerySystemInformation(SystemProcessInformation, NULL, 0, &return_length);
        return_length += 4096;
        if (base_sysproc) MSVCRT$free(base_sysproc);
        base_sysproc = MSVCRT$malloc(return_length);
        if (!base_sysproc) return;
        status = NTDLL$NtQuerySystemInformation(SystemProcessInformation,
            base_sysproc, return_length, &return_length);
    } while (status == STATUS_INFO_LENGTH_MISMATCH);

    if (!NT_SUCCESS(status)) {
        BeaconPrintf(CALLBACK_ERROR, "Failed to get system process information, error: %d\n",
                     KERNEL32$GetLastError());
        MSVCRT$free(base_sysproc);
        return;
    }

    system_proc_info = (SYSTEM_PROCESS_INFORMATION*)base_sysproc;

    BeaconPrintf(CALLBACK_OUTPUT, "%-50s %6s %6s %7s  %-35s  %s\n",
                 "Name", "PID", "PPID", "Session", "User", "Arch");
    BeaconPrintf(CALLBACK_OUTPUT, "%-50s %6s %6s %7s  %-35s  %s\n",
                 "----", "---", "----", "-------", "----", "----");

    do {
        proc_handle  = NULL;
        token_handle = NULL;
        user_token   = NULL;
        IsWow64        = FALSE;

        proc_handle = KERNEL32$OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE,
            HandleToUlong(system_proc_info->UniqueProcessId));

        if (proc_handle) {
            KERNEL32$IsWow64Process(proc_handle, &IsWow64);
            if (ADVAPI32$OpenProcessToken(proc_handle, TOKEN_QUERY, &token_handle) && token_handle) {
                user_token = GetUserByToken(token_handle);
                KERNEL32$CloseHandle(token_handle);
            }
            KERNEL32$CloseHandle(proc_handle);
        }

        char *name = NULL;
        char *user = NULL;

        if (system_proc_info->ImageName.Buffer && system_proc_info->ImageName.Length > 0) {
            name = WideToUtf8(system_proc_info->ImageName.Buffer,
                              system_proc_info->ImageName.Length / sizeof(WCHAR));
        }

        if (user_token) {
            user = WideToUtf8(user_token, KERNEL32$lstrlenW(user_token));
            MSVCRT$free(user_token);
        }

        BeaconPrintf(CALLBACK_OUTPUT, "%-50s %6lu %6lu %7lu  %-35s  %s\n",
                     name ? name : "[System]",
                     HandleToUlong(system_proc_info->UniqueProcessId),
                     HandleToUlong(system_proc_info->InheritedFromUniqueProcessId),
                     (ULONG)system_proc_info->SessionId,
                     user ? user : "N/A",
                     IsWow64 ? "x86" : "x64");

        if (name) MSVCRT$free(name);
        if (user) MSVCRT$free(user);

        if (system_proc_info->NextEntryOffset == 0)
            break;
        system_proc_info = (SYSTEM_PROCESS_INFORMATION*)((UINT_PTR)system_proc_info + system_proc_info->NextEntryOffset);

    } while (1);

    MSVCRT$free(base_sysproc);
}
