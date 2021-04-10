#include "api.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
const char className[] = "MyWinClass";
const char appName[] = "Dave's Tiny App";

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int main() {
    init();

    HINSTANCE hInstance = AI_GetModuleHandleA(NULL); //Get the instance handle of this application
    LPSTR cmdLine = AI_GetCommandLineA();            //Get a pointer to the command line

    STARTUPINFOA sui;
    AI_GetStartupInfoA(&sui);
    int ret;

    if (sui.dwFlags == STARTF_USESHOWWINDOW) {
        ret = WinMain(hInstance, NULL, cmdLine, SW_SHOWDEFAULT);
    } else {
        ret = WinMain(hInstance, NULL, cmdLine, sui.wShowWindow);
    }

    AI_ExitProcess((UINT)ret);

    return 0;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
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

    wc.hIcon = AI_LoadIconA(NULL, IDI_APPLICATION);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = AI_LoadCursorA(NULL, IDC_ARROW);

    AI_RegisterClassExA(&wc);
    hwnd = AI_CreateWindowExA(0, className, appName, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);
    if (hwnd == NULL)
        return 1;

    AI_UpdateWindow(hwnd);

    while (AI_GetMessageA(&msg, NULL, 0, 0)) {
        AI_TranslateMessage(&msg);
        AI_DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    RECT rect;
    HDC hdc;

    switch (message) {
    case WM_DESTROY:
        AI_PostQuitMessage(0);
        break;

    case WM_PAINT:
        hdc = AI_BeginPaint(hwnd, &ps);
        {
            AI_SetBkMode(hdc, TRANSPARENT);
            AI_GetClientRect(hwnd, &rect);
            AI_DrawTextA(hdc, appName, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        }
        AI_EndPaint(hwnd, &ps);
        break;

    default:
        return AI_DefWindowProcA(hwnd, message, wParam, lParam);
    }

    return 0;
}