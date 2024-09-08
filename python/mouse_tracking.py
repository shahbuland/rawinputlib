import asyncio
import ctypes
from ctypes import Structure, c_ulonglong, c_long, c_uint, c_bool
import pygame
import math

class MouseMoveData(Structure):
    _fields_ = [("timestamp", c_ulonglong),
                ("dx", c_long),
                ("dy", c_long)]

class MouseButtonData(Structure):
    _fields_ = [("timestamp", c_ulonglong),
                ("button", c_uint),
                ("down", c_bool)]

class MouseScrollData(Structure):
    _fields_ = [("timestamp", c_ulonglong),
                ("scrollAmount", c_long)]

class KeyboardData(Structure):
    _fields_ = [("timestamp", c_ulonglong),
                ("keyCode", c_uint),
                ("down", c_bool)]

class InputTracker:
    def __init__(self):
        self.rawinput_lib = ctypes.CDLL("./rawinputlib.dll")
        self.rawinput_lib.initialize_raw_input.restype = ctypes.c_int
        self.rawinput_lib.get_mouse_move_input.argtypes = [ctypes.POINTER(MouseMoveData)]
        self.rawinput_lib.get_mouse_move_input.restype = ctypes.c_int
        self.rawinput_lib.get_mouse_button_input.argtypes = [ctypes.POINTER(MouseButtonData)]
        self.rawinput_lib.get_mouse_button_input.restype = ctypes.c_int
        self.rawinput_lib.get_mouse_scroll_input.argtypes = [ctypes.POINTER(MouseScrollData)]
        self.rawinput_lib.get_mouse_scroll_input.restype = ctypes.c_int
        self.rawinput_lib.get_keyboard_input.argtypes = [ctypes.POINTER(KeyboardData)]
        self.rawinput_lib.get_keyboard_input.restype = ctypes.c_int
        self.rawinput_lib.cleanup_raw_input.restype = None
        self.running = False

    async def run(self, callbacks):
        if not self.rawinput_lib.initialize_raw_input():
            raise RuntimeError("Failed to initialize raw input")

        self.running = True
        try:
            while self.running:
                mouse_move_data = MouseMoveData()
                mouse_button_data = MouseButtonData()
                mouse_scroll_data = MouseScrollData()
                keyboard_data = KeyboardData()

                if self.rawinput_lib.get_mouse_move_input(ctypes.byref(mouse_move_data)):
                    callbacks['mouse_move'](mouse_move_data.timestamp, mouse_move_data.dx, mouse_move_data.dy)
                
                if self.rawinput_lib.get_mouse_button_input(ctypes.byref(mouse_button_data)):
                    callbacks['mouse_button'](mouse_button_data.timestamp, mouse_button_data.button, mouse_button_data.down)
                
                if self.rawinput_lib.get_mouse_scroll_input(ctypes.byref(mouse_scroll_data)):
                    callbacks['mouse_scroll'](mouse_scroll_data.timestamp, mouse_scroll_data.scrollAmount)
                
                if self.rawinput_lib.get_keyboard_input(ctypes.byref(keyboard_data)):
                    callbacks['keyboard'](keyboard_data.timestamp, keyboard_data.keyCode, keyboard_data.down)

                await asyncio.sleep(0.001)  # Small sleep to reduce CPU usage
        finally:
            self.rawinput_lib.cleanup_raw_input()

    def stop(self):
        self.running = False


if __name__ == "__main__":
    pygame.init()
    screen = pygame.display.set_mode((800, 600))
    font = pygame.font.Font(None, 36)

    active_buttons = set()
    active_keys = set()
    scroll_direction = 0
    mouse_vector = [0, 0]

    def update_display():
        screen.fill((0, 0, 0))
        
        # Mouse movement vector
        if mouse_vector[0] != 0 or mouse_vector[1] != 0:
            start_pos = (400, 300)
            end_pos = (400 + mouse_vector[0], 300 + mouse_vector[1])
            pygame.draw.line(screen, (255, 0, 0), start_pos, end_pos, 2)
            angle = math.atan2(mouse_vector[1], mouse_vector[0])
            pygame.draw.polygon(screen, (255, 0, 0), 
                                [end_pos, 
                                 (end_pos[0] - 10 * math.cos(angle - math.pi/6), 
                                  end_pos[1] - 10 * math.sin(angle - math.pi/6)),
                                 (end_pos[0] - 10 * math.cos(angle + math.pi/6), 
                                  end_pos[1] - 10 * math.sin(angle + math.pi/6))])
        
        # Mouse buttons
        for i, button in enumerate(active_buttons):
            text = font.render(f"Button: {button}", True, (255, 255, 255))
            screen.blit(text, (600, 500 + i * 30))
        
        # Scroll wheel
        if scroll_direction != 0:
            start_pos = (50, 300)
            end_pos = (50, 300 - scroll_direction * 50)
            pygame.draw.line(screen, (0, 255, 0), start_pos, end_pos, 2)
            pygame.draw.polygon(screen, (0, 255, 0), 
                                [end_pos, (end_pos[0] - 5, end_pos[1] + 10 * scroll_direction), 
                                 (end_pos[0] + 5, end_pos[1] + 10 * scroll_direction)])
        
        # Keyboard
        for i, key in enumerate(active_keys):
            text = font.render(f"Key: {key}", True, (255, 255, 255))
            screen.blit(text, (50, 500 + i * 30))
        
        pygame.display.flip()

    def handle_mouse_move(timestamp, dx, dy):
        global mouse_vector
        mouse_vector = [dx, dy]
        update_display()

    def handle_mouse_button(timestamp, button, down):
        if down:
            active_buttons.add(button)
        else:
            active_buttons.discard(button)
        update_display()

    def handle_mouse_scroll(timestamp, scroll_amount):
        global scroll_direction
        scroll_direction = 1 if scroll_amount > 0 else -1
        update_display()

    def handle_keyboard(timestamp, key_code, down):
        if down:
            active_keys.add(key_code)
        else:
            active_keys.discard(key_code)
        update_display()

    async def main():
        tracker = InputTracker()
        callbacks = {
            'mouse_move': handle_mouse_move,
            'mouse_button': handle_mouse_button,
            'mouse_scroll': handle_mouse_scroll,
            'keyboard': handle_keyboard
        }
        
        try:
            task = asyncio.create_task(tracker.run(callbacks))
            while True:
                for event in pygame.event.get():
                    if event.type == pygame.QUIT:
                        return
                await asyncio.sleep(0.01)
        finally:
            tracker.stop()
            await task
            pygame.quit()

    # Run the main coroutine
    asyncio.run(main())
