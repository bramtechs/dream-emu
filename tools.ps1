if (-not(Test-Path -Path "mgtools")){
    New-Item -ItemType Directory -Path "mgtools"
}

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022"
cmake --build build -j $Env:NUMBER_OF_PROCESSORS --config Release

cp .\build\magma\tools\deflation\cli\Release\deflation_cli.exe mgtools\deflation.exe 
cp .\build\magma\tools\palgen\Release\palgen.exe mgtools\palgen.exe 
