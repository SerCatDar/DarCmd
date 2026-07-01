# Dar Commander

A terminal file manager for Windows, written in C++.

## Features

- Two-panel file navigation
- Russian (CP1251) filename support
- File operations: copy, move, rename, delete
- Open files with their default applications
- Junction point / symlink detection
- Search in file names
- Keyboard-driven interface

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| ↑ / ↓ | Navigate files |
| → | Open file or enter directory |
| ← | Go up one directory |
| Tab | Switch between panels |
| `c` | Copy file |
| `m` | Move file to other panel |
| `r` | Rename file |
| `d` | Delete file or directory |
| `n` | Create new directory |
| `s` | Search in file names |
| `g` | Go to folder |
| `h` | Show version info |
| Enter | Exit |

## Building

Requires MSYS2 with MinGW64 and ncurses:

```bash
pacman -S mingw-w64-x86_64-ncurses
```

Compile:

```bash
g++ darcmd.cpp -o darcmd.exe -DNCURSES_STATIC -I C:\msys64\mingw64\include\ncurses -static -static-libgcc -static-libstdc++ C:\msys64\mingw64\lib\libncursesw.a
```

## Dependencies

- ncurses (libncursesw)
- Windows API (windows.h, shellapi.h)
- C++17 filesystem