#pragma once

#include <Windows.h>

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
    LoadLibraryA_t _LoadLibraryA;
    GetModuleHandleA_t _GetModuleHandleA;
    GetCommandLineA_t _GetCommandLineA;
    GetStartupInfoA_t _GetStartupInfoA;
    ExitProcess_t _ExitProcess;

    //USER32
    LoadIconA_t _LoadIconA;
    LoadCursorA_t _LoadCursorA;
    RegisterClassExA_t _RegisterClassExA;
    CreateWindowExA_t _CreateWindowExA;
    UpdateWindow_t _UpdateWindow;
    GetMessageA_t _GetMessageA;
    TranslateMessage_t _TranslateMessage;
    DispatchMessageA_t _DispatchMessageA;
    PostQuitMessage_t _PostQuitMessage;
    BeginPaint_t _BeginPaint;
    GetClientRect_t _GetClientRect;
    DrawTextA_t _DrawTextA;
    EndPaint_t _EndPaint;
    DefWindowProcA_t _DefWindowProcA;

    //GDI32
    SetBkMode_t _SetBkMode;
} API;
#pragma pack(pop)

void initAPI(API* api);