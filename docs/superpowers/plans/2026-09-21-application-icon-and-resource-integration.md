# Application Icon & Windows Resource Integration Plan

**Date:** 2026-09-21  
**Module:** Windows Resource Script (`app.rc`), Multi-Resolution `.ico`, Win32 Window Icon Binding, `build.bat`, and `README.md`  
**Status:** Completed & Verified  

---

## 1. Overview & Objectives

Tobelsoft Macro requires a high-definition, native Windows application icon embedded directly into the standalone binary `TobelsoftMacro.exe` and bound to the Win32 window class, system tray, taskbar, and Alt+Tab switch list.

---

## 2. Implemented Architecture & Asset Pipeline

### 2.1 Multi-Resolution `.ico` Generation
The user-supplied cyberpunk glowing shield emblem was converted into a full standard multi-resolution `.ico` file:
- Mipmap layers: `256x256`, `128x128`, `64x64`, `48x48`, `32x32`, `24x24`, and `16x16`.
- Asset paths:
  - `assets/app_icon.png` (High-resolution branding image for README & documentation)
  - `assets/app_icon.ico` (Master multi-size icon)
  - `c_src/resources/app_icon.png`
  - `c_src/resources/app_icon.ico`

### 2.2 Windows Resource Definitions
1. `c_src/resource.h`:
   ```c
   #define IDI_APP_ICON 101
   ```
2. `c_src/app.rc`:
   ```rc
   #include "resource.h"

   IDI_APP_ICON ICON "resources/app_icon.ico"
   1 24 "app.manifest"
   ```

### 2.3 Build Script Automation (`build.bat`)
`build.bat` automatically detects resource compilers (`llvm-rc.exe`, `windres.exe`, or `rc.exe`), compiles `c_src\app.rc` into `c_src\app.res`, and links it directly into `TobelsoftMacro.exe` using Clang `-O3 -mwindows`.

### 2.4 Win32 Window Class & System Tray Binding
- `c_src/ui/main_window.c`:
  - `wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));`
  - `wc.hIconSm = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_APP_ICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);`
  - `SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)wc.hIcon);`
  - `SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)wc.hIconSm);`
  - System Tray (`NOTIFYICONDATAW`) initializes with `hIconSm`.
- `c_src/ui/ui_dialogs.c`:
  - `AddEditDialogClass` and `CaptureDialogClass` inherit `IDI_APP_ICON`.

---

## 3. Verification & Validation

1. **Binary Resource Extraction:** Verified via Win32 `FindResourceW(h, 101, RT_GROUP_ICON)` returning valid resource pointer `0x307e1e8`.
2. **Build Verification:** `build.bat` compiles cleanly with `llvm-rc` and `clang.exe`.
3. **README.md Brand Showcase:** Top banner updated with centered logo, modern badges, and comprehensive documentation.
