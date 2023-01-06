.\deps.ps1

if (-not(Test-Path -Path "build")){
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 17 2022" -DCMAKE_EXPORT_COMPILE_COMMANDS=1
}

cmake --build build -j $Env:NUMBER_OF_PROCESSORS
