@echo off
set succ=1
cl forge.c || set succ=0
if %succ% == 1 (
    pushd ..
    forge\forge.exe check
    popd
)
