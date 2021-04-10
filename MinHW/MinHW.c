#include "api.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
const char className[] = "MyWinClass";
const char appName[] = "Dave's Tiny App";

int __stdcall WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

AI ai;

int main() {
    init(&ai);

    HINSTANCE hInstance = ai.AI_GetModuleHandleA(NULL); //Get the instance handle of this application
    LPSTR cmdLine = ai.AI_GetCommandLineA();            //Get a pointer to the command line

    STARTUPINFOA sui;
    ai.AI_GetStartupInfoA(&sui);
    int ret;

    if (sui.dwFlags == STARTF_USESHOWWINDOW) {
        ret = WinMain(hInstance, NULL, cmdLine, SW_SHOWDEFAULT);
    } else {
        ret = WinMain(hInstance, NULL, cmdLine, sui.wShowWindow);
    }

    ai.AI_ExitProcess((UINT)ret);

    return 0;
}

int __stdcall WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow) {
    WNDCLASSEX wc = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WndProc,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = hInstance,
        .hbrBackground = COLOR_3DSHADOW + 1,
        .lpszMenuName = NULL,
        .lpszClassName = className
    };
    MSG msg;
    HWND hwnd;

    wc.hIcon = ai.AI_LoadIconA(NULL, IDI_APPLICATION);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = ai.AI_LoadCursorA(NULL, IDC_ARROW);

    ai.AI_RegisterClassExA(&wc);
    hwnd = ai.AI_CreateWindowExA(0, className, appName, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);
    if (hwnd == NULL)
        return 1;

    ai.AI_UpdateWindow(hwnd);

    while (ai.AI_GetMessageA(&msg, NULL, 0, 0)) {
        ai.AI_TranslateMessage(&msg);
        ai.AI_DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    RECT rect;
    HDC hdc;

    switch (message) {
    case WM_DESTROY:
        ai.AI_PostQuitMessage(0);
        break;

    case WM_PAINT:
        hdc = ai.AI_BeginPaint(hwnd, &ps);
        {
            ai.AI_SetBkMode(hdc, TRANSPARENT);
            ai.AI_GetClientRect(hwnd, &rect);
            ai.AI_DrawTextA(hdc, appName, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        }
        ai.AI_EndPaint(hwnd, &ps);
        break;

    default:
        return ai.AI_DefWindowProcA(hwnd, message, wParam, lParam);
    }

    return 0;
}