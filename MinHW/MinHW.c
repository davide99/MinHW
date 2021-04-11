#include "api.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
const char className[] = "MyWinClass";
const char appName[] = "Dave's Tiny App";

int __stdcall WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

API api;
WNDCLASSEX wc = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WndProc,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hbrBackground = COLOR_3DSHADOW + 1,
        .lpszMenuName = NULL,
        .lpszClassName = className
};

int main() {
    initAPI(&api);

    HINSTANCE hInstance = api._GetModuleHandleA(NULL); //Get the instance handle of this application
    LPSTR cmdLine = api._GetCommandLineA();            //Get a pointer to the command line

    STARTUPINFOA sui;
    api._GetStartupInfoA(&sui);
    int ret;

    if (sui.dwFlags == STARTF_USESHOWWINDOW) {
        ret = WinMain(hInstance, NULL, cmdLine, SW_SHOWDEFAULT);
    } else {
        ret = WinMain(hInstance, NULL, cmdLine, sui.wShowWindow);
    }

    api._ExitProcess((UINT)ret);

    return 0;
}

int __stdcall WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nCmdShow) {
    MSG msg;
    HWND hwnd;

    wc.hInstance = hInstance;
    wc.hIcon = api._LoadIconA(NULL, IDI_APPLICATION);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = api._LoadCursorA(NULL, IDC_ARROW);

    api._RegisterClassExA(&wc);
    hwnd = api._CreateWindowExA(0, className, appName, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);
    if (hwnd == NULL)
        return 1;

    api._UpdateWindow(hwnd);

    while (api._GetMessageA(&msg, NULL, 0, 0)) {
        api._TranslateMessage(&msg);
        api._DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    RECT rect;
    HDC hdc;

    switch (message) {
    case WM_DESTROY:
        api._PostQuitMessage(0);
        break;

    case WM_PAINT:
        hdc = api._BeginPaint(hwnd, &ps);
        {
            api._SetBkMode(hdc, TRANSPARENT);
            api._GetClientRect(hwnd, &rect);
            api._DrawTextA(hdc, appName, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
        }
        api._EndPaint(hwnd, &ps);
        break;

    default:
        return api._DefWindowProcA(hwnd, message, wParam, lParam);
    }

    return 0;
}