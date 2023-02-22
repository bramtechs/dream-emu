@echo off
set succ=1

if exist dev.txt (
    echo Deleting old build...
    del /F forge.exe
)

if exist forge.exe goto RUN
if exist forge/forge.exe goto RUN

echo Compiling for the first time...
if exist forge.cpp (
    cl /EHsc /std:c++17 /O2 forge.cpp /DEBUG:FULL || set succ=0
) else (
    pushd forge
    cl /EHsc /std:c++17 /O2 forge.cpp /DEBUG:FULL || set succ=0
    popd ..
)

:RUN
if %succ% == 1 (
    pushd ..
    forge\forge.exe %*
    popd
)
