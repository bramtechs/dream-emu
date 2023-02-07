@echo off
rem launch this from msvc-enabled console

echo Building engine...

IF NOT EXIST .\build mkdir .\build
pushd .\build

set CFLAGS=/WX /std:c++14 /FC /Zi /nologo /EHsc
cl.exe %CFLAGS% ..\engine\win32_magma.cpp user32.lib gdi32.lib

popd
