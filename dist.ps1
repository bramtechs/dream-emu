$BUILD_DIR= ".\build\dream\Release"
$ErrorActionPreference = "Stop"

.\deps.ps1
 
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022"
cmake --build build -j $Env:NUMBER_OF_PROCESSORS --config Release
  
.\tools.ps1

# package assets
.\mgtools\deflation.exe raw_assets assets.mga

Copy-Item -Force -Recurse assets.mga $BUILD_DIR

# make ZIP
Compress-Archive -Force -Path $BUILD_DIR\* -DestinationPath .\build\DREAM_EMU.ZIP
