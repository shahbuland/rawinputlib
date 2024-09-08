#include <windows.h>
#include "rawinputlib.h"

static HWND hwnd = NULL;
static RAWINPUTDEVICE rid = {0};
static MouseMoveData lastMouseData = {0};
static MouseButtonData lastMouseButtonData = {0};
static MouseScrollData lastMouseScrollData = {0};
static KeyboardData lastKeyboardData = {0};

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_INPUT: {
            UINT dwSize = sizeof(RAWINPUT);
            BYTE lpb[sizeof(RAWINPUT)];
            
            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != (UINT)-1) {
                RAWINPUT* raw = (RAWINPUT*)lpb;
                ULONGLONG currentTime = GetTickCount64();

                if (raw->header.dwType == RIM_TYPEMOUSE) {
                    lastMouseData.timestamp = currentTime;
                    lastMouseData.dx = raw->data.mouse.lLastX;
                    lastMouseData.dy = raw->data.mouse.lLastY;

                    // Handle mouse buttons (yes there's a lot)
                    if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
                        lastMouseButtonData.timestamp = currentTime;
                        lastMouseButtonData.button = VK_LBUTTON;
                        lastMouseButtonData.down = TRUE;
                    } else if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
                        lastMouseButtonData.timestamp = currentTime;
                        lastMouseButtonData.button = VK_LBUTTON;
                        lastMouseButtonData.down = FALSE;
                    } else if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
                        lastMouseButtonData.timestamp = currentTime;
                        lastMouseButtonData.button = VK_RBUTTON;
                        lastMouseButtonData.down = TRUE;
                    } else if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
                        lastMouseButtonData.timestamp = currentTime;
                        lastMouseButtonData.button = VK_RBUTTON;
                        lastMouseButtonData.down = FALSE;
                    } else if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
                        lastMouseButtonData.timestamp = currentTime;
                        lastMouseButtonData.button = VK_MBUTTON;
                        lastMouseButtonData.down = TRUE;
                    } else if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) {
                        lastMouseButtonData.timestamp = currentTime;
                        lastMouseButtonData.button = VK_MBUTTON;
                        lastMouseButtonData.down = FALSE;
                    } else if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) {
                        lastMouseButtonData.timestamp = currentTime;
                        lastMouseButtonData.button = VK_XBUTTON1;
                        lastMouseButtonData.down = TRUE;
                    } else if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP) {
                        lastMouseButtonData.timestamp = currentTime;
                        lastMouseButtonData.button = VK_XBUTTON1;
                        lastMouseButtonData.down = FALSE;
                    } else if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) {
                        lastMouseButtonData.timestamp = currentTime;
                        lastMouseButtonData.button = VK_XBUTTON2;
                        lastMouseButtonData.down = TRUE;
                    } else if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) {
                        lastMouseButtonData.timestamp = currentTime;
                        lastMouseButtonData.button = VK_XBUTTON2;
                        lastMouseButtonData.down = FALSE;
                    }

                    // Handle mouse wheel
                    if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL) {
                        lastMouseScrollData.timestamp = currentTime;
                        lastMouseScrollData.scrollAmount = (SHORT)raw->data.mouse.usButtonData;
                    }
                } else if (raw->header.dwType == RIM_TYPEKEYBOARD) { // handle keyboard
                    lastKeyboardData.timestamp = currentTime;
                    lastKeyboardData.keyCode = raw->data.keyboard.VKey;
                    lastKeyboardData.down = !(raw->data.keyboard.Flags & RI_KEY_BREAK);
                }
            }
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Get raw device input ready
__declspec(dllexport) int initialize_raw_input() {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "RawInputWindowClass";
    
    if (!RegisterClass(&wc)) return 0;

    hwnd = CreateWindowEx(0, "RawInputWindowClass", NULL, 0, 0, 0, 0, 0, 
                          HWND_MESSAGE, NULL, GetModuleHandle(NULL), NULL);
    if (hwnd == NULL) return 0;

    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;  // Mouse
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = hwnd;

    RAWINPUTDEVICE rids[2];
    rids[0] = rid;

    rids[1] = rid;
    rids[1].usUsage = 0x06;  // Keyboard

    return RegisterRawInputDevices(rids, 2, sizeof(RAWINPUTDEVICE));
}

// call to get last logged mouse move input
__declspec(dllexport) int get_mouse_move_input(MouseMoveData* data) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (lastMouseData.dx != 0 || lastMouseData.dy != 0) {
        *data = lastMouseData;
        lastMouseData.dx = 0;
        lastMouseData.dy = 0;
        return 1;
    }
    return 0;
}

// Call to get last logged mouse button input
__declspec(dllexport) int get_mouse_button_input(MouseButtonData* data) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (lastMouseButtonData.timestamp != 0) {
        *data = lastMouseButtonData;
        lastMouseButtonData.timestamp = 0;
        return 1;
    }
    return 0;
}

// Call to get last logged scroll input
__declspec(dllexport) int get_mouse_scroll_input(MouseScrollData* data) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (lastMouseScrollData.timestamp != 0) {
        *data = lastMouseScrollData;
        lastMouseScrollData.timestamp = 0;
        return 1;
    }
    return 0;
}

// Call to get last keyboard input data logged
__declspec(dllexport) int get_keyboard_input(KeyboardData* data) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (lastKeyboardData.timestamp != 0) {
        *data = lastKeyboardData;
        lastKeyboardData.timestamp = 0;
        return 1;
    }
    return 0;
}

__declspec(dllexport) void cleanup_raw_input() {
    if (hwnd != NULL) {
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    UnregisterClass("RawInputWindowClass", GetModuleHandle(NULL));
}