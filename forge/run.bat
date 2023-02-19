@echo off
set succ=1
cl /std:c++17 forge.cpp || set succ=0
if %succ% == 1 (
    pushd ..
    forge\forge.exe %*
    popd
)
