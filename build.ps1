.\deps.ps1

if (-not(Test-Path -Path "build")){
    cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -T v141_xp
}

cmake --build build -j $Env:NUMBER_OF_PROCESSORS
#msbuild .\build\dream_emu.sln /p:XPDepreciationWarning=false /p:Platform=x64
