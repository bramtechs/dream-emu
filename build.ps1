.\deps.ps1

if (-not(Test-Path -Path "build")){
    .\gen.ps1
}

cmake --build build -j $Env:NUMBER_OF_PROCESSORS
#msbuild .\build\dream_emu.sln /p:XPDepreciationWarning=false /p:Platform=x64
