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

#pragma pack(push, 1)
typedef __declspec(align(4)) struct {
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
} AI;
#pragma pack(pop)

uint32_t hash(uint8_t* str) {
    uint32_t hash = 5381;
    uint8_t c;

    while (c = *str++)
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

#pragma pack(push, 1)
struct dll_info {
    uint32_t exported_functions;
    uintptr_t address_table_RVA;
    uintptr_t name_pointer_table_RVA;
    uintptr_t ordinal_table_RVA;
};
#pragma pack(pop)

void findFunc(uintptr_t dllBase, uint32_t* hashes, void** ptrs, size_t size) {
    uintptr_t PE_RVA = *(uintptr_t*)((uint8_t*)dllBase + 0x3Cu);
    uintptr_t PE = dllBase + *(uintptr_t*)((uint8_t*)dllBase + 0x3Cu);

    uintptr_t export_table_RVA = *(uintptr_t*)((uint8_t*)PE + 0x78u);
    struct dll_info dll = *(struct dll_info*)((uint8_t*)dllBase + export_table_RVA + 0x18u);

    uintptr_t name_pointer_table_entry_RVA = dll.name_pointer_table_RVA;
    uint32_t i, j;

    uintptr_t ordinal_function_RVA;
    uint16_t ordinal_function;
    uintptr_t function_RVA;

    for (i=0; i < dll.exported_functions; i++, name_pointer_table_entry_RVA += 4) {
        uintptr_t function_name_RVA = *(uintptr_t*)((uint8_t*)dllBase + name_pointer_table_entry_RVA);
        char* function_name = (uint8_t*)dllBase + function_name_RVA;
        uint32_t function_hash = hash(function_name);

        for (j = 0; j < size; j++) {
            if (function_hash == hashes[j]) {
                ordinal_function_RVA = dll.ordinal_table_RVA + i * 2;
                ordinal_function = *(uint16_t*)((uint8_t*)dllBase + ordinal_function_RVA);
                function_RVA = *(uintptr_t*)((uint8_t*)dllBase + dll.address_table_RVA + ordinal_function * 4);

                ptrs[j] = (uint8_t*)dllBase + function_RVA;

                break;
            }
        }
    }
}

void init(AI *ai) {
    PPEB peb = __readfsdword(0x30);
    uintptr_t kernel32Base = 0;

    for (PLIST_ENTRY ptr = peb->Ldr->InMemoryOrderModuleList.Flink; kernel32Base == 0; ptr = ptr->Flink) {
        PLDR_DATA_TABLE_ENTRY e = CONTAINING_RECORD(ptr, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        UNICODE_STRING* BaseDllName = (UNICODE_STRING*)e->Reserved4;
        uint32_t dllNameHash = hash((uint8_t*)BaseDllName->Buffer);

        if (dllNameHash == KERNEL32DLL_UCASE_HASH || dllNameHash == KERNEL32DLL_LCASE_HASH)
            kernel32Base = (uintptr_t)e->DllBase;
    }

    if (kernel32Base == 0)
        return;

    uint32_t Kernel32Hashes[] = {
        0x5FBFF0FB,
        0x5A153F58,
        0xB511FC4D,
        0x348B7545,
        0xB769339E
    };
    findFunc(kernel32Base, Kernel32Hashes, &ai->AI_LoadLibraryA, ARRAYSIZE(Kernel32Hashes));

    HMODULE user32 = ai->AI_LoadLibraryA("USER32.DLL");
    uint32_t User32Hashes[] = {
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
    findFunc((uintptr_t)user32, User32Hashes, &ai->AI_LoadIconA, ARRAYSIZE(User32Hashes));

    HMODULE gdi32 = ai->AI_LoadLibraryA("GDI32.DLL");
    uint32_t Gdi32Hash = 0x6F828843;
    findFunc((uintptr_t)gdi32, &Gdi32Hash, &ai->AI_SetBkMode, 1);
}