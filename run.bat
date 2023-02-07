@echo off

IF EXIST "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call build.bat && build\win32_magma.exe
) ELSE (
    call build.bat && build_xp\win32_magma.exe
)
