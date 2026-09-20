@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   Tobelsoft Macro - C Native High-Performance Build
echo ===================================================

set CLANG_BIN=C:\Users\Administrator\AppData\Local\Programs\Swift\Toolchains\6.0.3+Asserts\usr\bin\clang.exe
if not exist "%CLANG_BIN%" (
    where clang >nul 2>nul
    if %errorlevel% equ 0 (
        set CLANG_BIN=clang
    ) else (
        echo [ERROR] clang.exe not found!
        exit /b 1
    )
)

echo [INFO] Compiler: %CLANG_BIN%

set RC_BIN=C:\Users\Administrator\AppData\Local\Programs\Swift\Toolchains\6.0.3+Asserts\usr\bin\llvm-rc.exe
if not exist "%RC_BIN%" (
    where llvm-rc >nul 2>nul
    if %errorlevel% equ 0 (
        set RC_BIN=llvm-rc
    ) else (
        where windres >nul 2>nul
        if %errorlevel% equ 0 (
            set RC_BIN=windres
        ) else (
            where rc >nul 2>nul
            if %errorlevel% equ 0 (
                set RC_BIN=rc
            ) else (
                set RC_BIN=
            )
        )
    )
)

taskkill /F /IM TobelsoftMacro.exe 2>nul

set RES_OBJ=
if defined RC_BIN (
    echo [INFO] Compiling Windows resource: %RC_BIN%
    if exist "c_src\app.rc" (
        "%RC_BIN%" /I c_src /fo c_src\app.res c_src\app.rc 2>nul
        if exist "c_src\app.res" (
            set RES_OBJ=c_src\app.res
        )
    )
)

set SRC_FILES=c_src\main.c c_src\common\types.c c_src\common\utils.c c_src\common\cJSON.c c_src\storage\config_manager.c c_src\core\input_sender.c c_src\core\input_hook.c c_src\core\macro_engine.c c_src\ui\animation.c c_src\ui\theme.c c_src\ui\ui_icons.c c_src\ui\ui_dialogs.c c_src\ui\main_window.c %RES_OBJ%
set LIBS=-luser32 -lgdi32 -lmsimg32 -lwinmm -lcomctl32 -lcomdlg32 -lshell32 -lole32 -ldwmapi -luxtheme -lgdiplus
set CFLAGS=-O3 -Wall -Wextra -Wno-unused-parameter -mwindows

echo [INFO] Compiling and linking TobelsoftMacro.exe...
"%CLANG_BIN%" %CFLAGS% %SRC_FILES% %LIBS% -o TobelsoftMacro.exe

if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    exit /b %errorlevel%
)

echo.
echo [SUCCESS] TobelsoftMacro.exe generated successfully with native icon resources!
echo ===================================================
