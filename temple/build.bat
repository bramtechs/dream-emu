@echo off
rem launch this from msvc-enabled console

IF NOT EXIST .\build mkdir .\build
pushd .\build

set CFLAGS=/WX /std:c++14 /FC /Zi /nologo /EHsc /I ..\..\magma\engine
cl.exe %CFLAGS% ..\source\temple.cpp user32.lib gdi32.lib

popd
echo Built temple game...
