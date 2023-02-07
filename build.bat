@echo off
rem launch this from msvc-enabled console

WHERE cl
IF %ERRORLEVEL% NEQ 0 call shell.bat 

IF NOT EXIST .\build mkdir .\build
pushd .\build

set CFLAGS=/WX /std:c++14 /FC /Zi /nologo /EHsc
cl.exe %CFLAGS% ..\src\win32_magma.cpp user32.lib gdi32.lib

popd
echo Built application...
