#ifndef RAWINPUTLIB_DLL_H
#define RAWINPUTLIB_DLL_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ULONGLONG timestamp;
    LONG dx;
    LONG dy;
} MouseMoveData;

typedef struct {
    ULONGLONG timestamp;
    UINT button; // which button pressed
    BOOL down; // button down or up event?
} MouseButtonData;

typedef struct {
    ULONGLONG timestamp;
    LONG scrollAmount; // negative = scroll down, positive = scroll up
} MouseScrollData;

typedef struct {
    ULONGLONG timestamp;
    UINT keyCode; // keycode of pressed key
    BOOL down; // button down or up?
} KeyboardData;

__declspec(dllexport) int initialize_raw_input();
__declspec(dllexport) int get_mouse_move_input(MouseMoveData* data);
__declspec(dllexport) int get_mouse_button_input(MouseButtonData* data);
__declspec(dllexport) int get_mouse_scroll_input(MouseScrollData* data);
__declspec(dllexport) int get_keyboard_input(KeyboardData* data);
__declspec(dllexport) void cleanup_raw_input();

#ifdef __cplusplus
}
#endif

#endif // RAWINPUTLIB_DLL_H