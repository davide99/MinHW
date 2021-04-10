#pragma once

#include <Windows.h>
#include <winternl.h>
#include <stdint.h>

#define KERNEL32DLL_UCASE_HASH 0x2B5F0u
#define KERNEL32DLL_LCASE_HASH 0x2B610u

//KERNEL32.DLL
typedef HMODULE(__stdcall* GetModuleHandleA_t)(LPCSTR);
typedef LPSTR(__stdcall* GetCommandLineA_t)();
typedef void(__stdcall* GetStartupInfoA_t)(LPSTARTUPINFOA);
typedef void(__stdcall* ExitProcess_t)(UINT);
typedef HMODULE(__stdcall* LoadLibraryA_t)(LPCSTR);

//USER32.DLL
typedef HICON(__stdcall* LoadIconA_t)(HINSTANCE, LPCSTR);
typedef HCURSOR(__stdcall* LoadCursorA_t)(HINSTANCE, LPCSTR);
typedef ATOM(__stdcall* RegisterClassExA_t)(const WNDCLASSEXA*);
typedef HWND(__stdcall* CreateWindowExA_t)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
typedef BOOL(__stdcall* UpdateWindow_t)(HWND);
typedef BOOL(__stdcall* GetMessageA_t)(LPMSG, HWND, UINT, UINT);
typedef BOOL(__stdcall* TranslateMessage_t)(const MSG*);
typedef LRESULT(__stdcall* DispatchMessageA_t)(const MSG*);
typedef void(__stdcall* PostQuitMessage_t)(int);
typedef HDC(__stdcall* BeginPaint_t)(HWND, LPPAINTSTRUCT);
typedef BOOL(_stdcall* GetClientRect_t)(HWND, LPRECT);
typedef int(__stdcall* DrawTextA_t)(HDC, LPCSTR, int, LPRECT, UINT);
typedef BOOL(__stdcall* EndPaint_t)(HWND, const PAINTSTRUCT*);
typedef LRESULT(__stdcall* DefWindowProcA_t)(HWND, UINT, WPARAM, LPARAM);

//GDI32.DLL
typedef int(__stdcall* SetBkMode_t)(HDC, int);

//KERNEL32
LoadLibraryA_t AI_LoadLibraryA;
GetModuleHandleA_t AI_GetModuleHandleA;
GetCommandLineA_t AI_GetCommandLineA;
GetStartupInfoA_t AI_GetStartupInfoA;
ExitProcess_t AI_ExitProcess;

//USER32
LoadIconA_t AI_LoadIconA;
LoadCursorA_t AI_LoadCursorA;
RegisterClassExA_t AI_RegisterClassExA;
CreateWindowExA_t AI_CreateWindowExA;
UpdateWindow_t AI_UpdateWindow;
GetMessageA_t AI_GetMessageA;
TranslateMessage_t AI_TranslateMessage;
DispatchMessageA_t AI_DispatchMessageA;
PostQuitMessage_t AI_PostQuitMessage;
BeginPaint_t AI_BeginPaint;
GetClientRect_t AI_GetClientRect;
DrawTextA_t AI_DrawTextA;
EndPaint_t AI_EndPaint;
DefWindowProcA_t AI_DefWindowProcA;

//GDI32
SetBkMode_t AI_SetBkMode;

uint32_t hash(uint8_t* str) {
    uint32_t hash = 5381;
    uint8_t c;

    while (c = *str++)
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

void findFunc(uint32_t dllBase, uint32_t* hashes, void*** ptrs, size_t size) {
    uintptr_t PE_RVA = *(uintptr_t*)((uint8_t*)dllBase + 0x3Cu);
    char* PE = (char*)dllBase + PE_RVA;

    uintptr_t export_table_RVA = *(uintptr_t*)((uint8_t*)PE + 0x78u);
    uint32_t exported_functions = *(uint32_t*)((uint8_t*)dllBase + export_table_RVA + 0x18u);
    uintptr_t address_table_RVA = *(uintptr_t*)((uint8_t*)dllBase + export_table_RVA + 0x1cu);
    uintptr_t name_pointer_table_RVA = *(uintptr_t*)((uint8_t*)dllBase + export_table_RVA + 0x20u);
    uintptr_t ordinal_table_RVA = *(uintptr_t*)((uint8_t*)dllBase + export_table_RVA + 0x24u);

    uintptr_t name_pointer_table_entry_RVA = name_pointer_table_RVA;
    uint32_t i, j;

    uintptr_t ordinal_function_RVA;
    uint16_t ordinal_function;
    uintptr_t function_RVA;
    int found;

    for (i=0; i < exported_functions; i++, name_pointer_table_entry_RVA += 4) {
        uintptr_t function_name_RVA = *(uintptr_t*)((uint8_t*)dllBase + name_pointer_table_entry_RVA);
        char* function_name = (uint8_t*)dllBase + function_name_RVA;
        uint32_t function_hash = hash(function_name);
        found = 0;

        for (j = 0; j < size && !found; j++) {
            if (function_hash == hashes[j])
                found = 1;
        }
        j--;
        
        if (found){
            ordinal_function_RVA = ordinal_table_RVA + i * 2;
            ordinal_function = *(uint16_t*)((uint8_t*)dllBase + ordinal_function_RVA);

            function_RVA = *(uintptr_t*)((uint8_t*)dllBase + address_table_RVA + ordinal_function * 4);

            *ptrs[j] = (uint8_t*)dllBase + function_RVA;
        }
    }
}

__declspec(naked)
void* getPEB() {
    __asm mov EAX, FS: [30h]
    __asm ret
}

void* init() {
    PPEB peb = getPEB();
    uintptr_t kernel32Base = 0;

    for (PLIST_ENTRY ptr = peb->Ldr->InMemoryOrderModuleList.Flink;; ptr = ptr->Flink) {
        PLDR_DATA_TABLE_ENTRY e = CONTAINING_RECORD(ptr, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        UNICODE_STRING* BaseDllName = e->Reserved4;
        uint32_t dllNameHash = hash(BaseDllName->Buffer);

        if (dllNameHash == KERNEL32DLL_UCASE_HASH || dllNameHash == KERNEL32DLL_LCASE_HASH) {
            kernel32Base = e->DllBase;
            break;
        }

        if (e->DllBase == 0)
            break;
    }

    if (kernel32Base == 0)
        return;

    uint32_t KernelH[] = { 0x5FBFF0FB, 0x5A153F58, 0xB511FC4D, 0x348B7545, 0xB769339E };
    void* arrayH[] = { &AI_LoadLibraryA ,&AI_GetModuleHandleA ,&AI_GetCommandLineA, &AI_GetStartupInfoA ,&AI_ExitProcess };
    findFunc(kernel32Base, KernelH, arrayH, 5);

    HMODULE user32 = AI_LoadLibraryA("USER32.DLL");
    uint32_t UserH[] = {
        0x110F756F,
        0xEF1BD604,
        0x932EB07E,
        0x1C82E26F,
        0xE870E800,
        0xCBCEB9AB,
        0xE5425A58,
        0xABA0605B,
        0x218F96D3,
        0xDD158C66,
        0xB05422B2,
        0x5F0BBD59,
        0x2C1A4AD8,
        0x68F05E41,
    };
    void* arrayU[] = {
         &AI_LoadIconA,
         &AI_LoadCursorA,
         &AI_RegisterClassExA,
         &AI_CreateWindowExA,
         &AI_UpdateWindow,
         &AI_GetMessageA,
         &AI_TranslateMessage,
         &AI_DispatchMessageA,
         &AI_PostQuitMessage,
         &AI_BeginPaint,
         &AI_GetClientRect,
         &AI_DrawTextA,
         &AI_EndPaint,
         &AI_DefWindowProcA
    };
    findFunc(user32, UserH, arrayU, 14);

    HMODULE gdi32 = AI_LoadLibraryA("GDI32.DLL");
    uint32_t GdiH[] = { 0x6F828843 };
    void* arrayG[] = { &AI_SetBkMode };
    findFunc(gdi32, GdiH, arrayG, 1);
}