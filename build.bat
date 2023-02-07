@echo off

WHERE cl
IF %ERRORLEVEL% NEQ 0 call shell.bat 

IF EXIST "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    echo Modern build...
    IF NOT EXIST build mkdir build 
    pushd build
    cl -DHANDMADE_WIN32=1 -FC -Zi ..\src\win32_magma.cpp user32.lib gdi32.lib
    popd
    
) ELSE (
    echo Legacy build...
    IF NOT EXIST build_xp mkdir build_xp
    pushd build_xp
    cl -DHANDMADE_WIN32=1 -FC -Zi ..\src\win32_magma.cpp user32.lib gdi32.lib
    popd
    
)

