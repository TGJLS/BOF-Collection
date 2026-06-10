#include <windows.h>
#include <tlhelp32.h>
#include "bofdefs.h"
#include "beacon.h"

#ifndef ProcessCommandLineInformation
#define ProcessCommandLineInformation ((PROCESSINFOCLASS)60)
#endif

static char* WideToUtf8(LPCWSTR wide, int wide_chars) {
    int needed = KERNEL32$WideCharToMultiByte(CP_UTF8, 0, wide, wide_chars, NULL, 0, NULL, NULL);
    if (needed <= 0) return NULL;
    char *buf = (char*)MSVCRT$malloc(needed + 1);
    if (!buf) return NULL;
    KERNEL32$WideCharToMultiByte(CP_UTF8, 0, wide, wide_chars, buf, needed, NULL, NULL);
    buf[needed] = '\0';
    return buf;
}

static void get_tokens(HANDLE process_handle) {
    HANDLE                token_handle   = NULL;
    TOKEN_USER           *token_user_ptr = NULL;
    TOKEN_MANDATORY_LABEL *integrity     = NULL;
    TOKEN_ELEVATION        elevation     = {0};
    ULONG                  return_len    = 0;
    NTSTATUS               status;

    BeaconPrintf(CALLBACK_OUTPUT, "\n[Token]\n");

    if (!ADVAPI32$OpenProcessToken(process_handle, TOKEN_QUERY, &token_handle)) {
        BeaconPrintf(CALLBACK_ERROR, "OpenProcessToken failed: %d\n", KERNEL32$GetLastError());
        return;
    }

    status = NTDLL$NtQueryInformationToken(token_handle, TokenUser, NULL, 0, &return_len);
    if (return_len > 0) {
        token_user_ptr = (TOKEN_USER*)MSVCRT$malloc(return_len);
        if (token_user_ptr) {
            status = NTDLL$NtQueryInformationToken(token_handle, TokenUser,
                token_user_ptr, return_len, &return_len);
            if (NT_SUCCESS(status)) {
                WCHAR        domain[MAX_PATH]   = {0};
                WCHAR        username[MAX_PATH] = {0};
                DWORD        dom_len = MAX_PATH, usr_len = MAX_PATH;
                SID_NAME_USE sid_type;
                if (ADVAPI32$LookupAccountSidW(NULL, token_user_ptr->User.Sid,
                        username, &usr_len, domain, &dom_len, &sid_type)) {
                    char *dom_n = WideToUtf8(domain, -1);
                    char *usr_n = WideToUtf8(username, -1);
                    BeaconPrintf(CALLBACK_OUTPUT, "  User:      %s\\%s\n",
                                 dom_n ? dom_n : "?", usr_n ? usr_n : "?");
                    if (dom_n) MSVCRT$free(dom_n);
                    if (usr_n) MSVCRT$free(usr_n);
                }
            }
            MSVCRT$free(token_user_ptr);
        }
    }

    return_len = sizeof(elevation);
    status = NTDLL$NtQueryInformationToken(token_handle, TokenElevation,
                                            &elevation, return_len, &return_len);
    if (NT_SUCCESS(status)) {
        BeaconPrintf(CALLBACK_OUTPUT, "  Elevated:  %s\n",
                     elevation.TokenIsElevated ? "Yes" : "No");
    }

    NTDLL$NtQueryInformationToken(token_handle, TokenIntegrityLevel, NULL, 0, &return_len);
    if (return_len > 0) {
        integrity = (TOKEN_MANDATORY_LABEL*)MSVCRT$malloc(return_len);
        if (integrity) {
            status = NTDLL$NtQueryInformationToken(token_handle, TokenIntegrityLevel,
                integrity, return_len, &return_len);
            if (NT_SUCCESS(status)) {
                SID  *isid  = (SID*)integrity->Label.Sid;
                ULONG level = isid->SubAuthority[isid->SubAuthorityCount - 1];
                const char *lvl_str = "Untrusted";
                if      (level >= SECURITY_MANDATORY_SYSTEM_RID) lvl_str = "System";
                else if (level >= SECURITY_MANDATORY_HIGH_RID)   lvl_str = "High";
                else if (level >= SECURITY_MANDATORY_MEDIUM_RID) lvl_str = "Medium";
                else if (level >= SECURITY_MANDATORY_LOW_RID)    lvl_str = "Low";
                BeaconPrintf(CALLBACK_OUTPUT, "  Integrity: %s\n", lvl_str);
            }
            MSVCRT$free(integrity);
        }
    }

    KERNEL32$CloseHandle(token_handle);
}

static void get_modules(HANDLE process_handle) {
    HMODULE    modules[256];
    DWORD      needed    = 0;
    WCHAR      wide_name[MAX_PATH];
    MODULEINFO mod_info;
    DWORD      mod_count, i;

    BeaconPrintf(CALLBACK_OUTPUT, "\n[Modules]\n");

    if (!PSAPI$EnumProcessModulesEx(process_handle, modules, sizeof(modules), &needed, 3)) {
        BeaconPrintf(CALLBACK_ERROR, "EnumProcessModulesEx failed: %d\n", KERNEL32$GetLastError());
        return;
    }

    mod_count = needed / sizeof(HMODULE);
    {
        DWORD buf_capacity = sizeof(modules) / sizeof(HMODULE);
        if (mod_count > buf_capacity) mod_count = buf_capacity;
    }
    for (i = 0; i < mod_count; i++) {
        char *name_n = NULL;
        if (PSAPI$GetModuleFileNameExW(process_handle, modules[i], wide_name, MAX_PATH)) {
            name_n = WideToUtf8(wide_name, -1);
        }
        if (PSAPI$GetModuleInformation(process_handle, modules[i], &mod_info, sizeof(mod_info))) {
            BeaconPrintf(CALLBACK_OUTPUT, "  %-60s base=0x%p  entry=0x%p  size=0x%lx\n",
                         name_n ? name_n : "?",
                         mod_info.lpBaseOfDll,
                         mod_info.EntryPoint,
                         (unsigned long)mod_info.SizeOfImage);
        }
        if (name_n) MSVCRT$free(name_n);
    }
}

static void get_cmdline(HANDLE process_handle) {
    PVOID           buffer     = NULL;
    PUNICODE_STRING cmdline    = NULL;
    ULONG           return_len = 0;
    NTSTATUS        status;

    BeaconPrintf(CALLBACK_OUTPUT, "\n[Cmdline]\n");

    NTDLL$NtQueryInformationProcess(process_handle, ProcessCommandLineInformation,
                                     NULL, 0, &return_len);
    if (return_len > 0) {
        buffer = MSVCRT$malloc(return_len);
        if (!buffer) {
            BeaconPrintf(CALLBACK_ERROR, "get_cmdline: malloc failed\n");
            return;
        }
        status = NTDLL$NtQueryInformationProcess(process_handle, ProcessCommandLineInformation,
                                                  buffer, return_len, &return_len);
        if (NT_SUCCESS(status)) {
            char *narrow;
            cmdline = (PUNICODE_STRING)buffer;
            narrow = WideToUtf8(cmdline->Buffer, -1);
            if (narrow) {
                BeaconPrintf(CALLBACK_OUTPUT, "  %s\n", narrow);
                MSVCRT$free(narrow);
            }
        } else {
            BeaconPrintf(CALLBACK_ERROR, "NtQueryInformationProcess(cmdline) failed: 0x%lx\n",
                         (unsigned long)status);
        }
        MSVCRT$free(buffer);
    } else {
        BeaconPrintf(CALLBACK_ERROR, "get_cmdline: size probe returned 0\n");
    }
}

static void get_threads(DWORD pid) {
    HANDLE        snapshot = INVALID_HANDLE_VALUE;
    THREADENTRY32 te;

    BeaconPrintf(CALLBACK_OUTPUT, "\n[Threads]\n");

    snapshot = KERNEL32$CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        BeaconPrintf(CALLBACK_ERROR, "CreateToolhelp32Snapshot failed: %d\n", KERNEL32$GetLastError());
        return;
    }

    te.dwSize = sizeof(THREADENTRY32);
    if (KERNEL32$Thread32First(snapshot, &te)) {
        do {
            if (te.th32OwnerProcessID == pid) {
                BeaconPrintf(CALLBACK_OUTPUT, "  TID: %lu\n", (unsigned long)te.th32ThreadID);
            }
        } while (KERNEL32$Thread32Next(snapshot, &te));
    }

    KERNEL32$CloseHandle(snapshot);
}

void go(char *args, int len) {
    HANDLE process_handle = NULL;
    datap  data_parser    = {0};
    INT32  pid;

    BeaconDataParse(&data_parser, args, len);
    pid = BeaconDataInt(&data_parser);

    process_handle = KERNEL32$OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                           FALSE, (DWORD)pid);
    if (!process_handle) {
        DWORD open_err = KERNEL32$GetLastError();
        process_handle = KERNEL32$OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
                                               FALSE, (DWORD)pid);
        if (!process_handle) {
            BeaconPrintf(CALLBACK_ERROR, "process grep: OpenProcess failed for PID %d (error %d)\n",
                         pid, open_err);
            return;
        }
    }

    get_tokens(process_handle);
    get_modules(process_handle);
    get_cmdline(process_handle);
    get_threads((DWORD)pid);

    goto cleanup;

cleanup:
    if (process_handle) KERNEL32$CloseHandle(process_handle);
}
