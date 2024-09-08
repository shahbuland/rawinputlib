# rawinputlib
Library based roughly off Libuiohook for getting raw key+mouse data for game data collection.
To build DLL:
```
gcc -I./include -shared -o build/rawinputlib.dll src/rawinputlib.c -luser32
```
Put that DLL in your python folder that you want to use this in.
For example usage see `python/mouse_tracking.py` (put the DLL in there first)
I uploaded the DLL in the git repo since I'm chill like that.
