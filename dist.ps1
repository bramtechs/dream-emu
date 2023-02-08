$BUILD_DIR= ".\build\dream\Release"
$ErrorActionPreference = "Stop"

# install windows xp compilation support
& "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vs_installer.exe" modify --installPath 'C:\Program Files\Microsoft Visual Studio\2022\Preview' --add 'Microsoft.VisualStudio.Component.WinXP' --quiet --norestart --nocache --force

# download all dependencies
.\deps.ps1
 
# build everything in release mode
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64 -T v141_xp
cmake --build build -j $Env:NUMBER_OF_PROCESSORS --config Release

# build and copy tools
.\tools.ps1
New-Item -ItemType Directory -Path $BUILD_DIR\tools -Force
Copy-Item -Force -Recurse .\mgtools\* $BUILD_DIR\tools

# build dream
New-Item -ItemType Directory -Path $BUILD_DIR\dream -Force
.\mgtools\deflation.exe raw_assets core_assets $BUILD_DIR\dream\assets.mga --compress
Move-Item $BUILD_DIR\dream.exe $BUILD_DIR\dream\dream.exe -Force

# build and include temple as well
New-Item -ItemType Directory -Path $BUILD_DIR\temple -Force
Copy-Item -Force -Recurse .\build\temple\Release\temple.exe $BUILD_DIR\temple
.\mgtools\deflation.exe temple\raw_assets core_assets $BUILD_DIR\temple\assets.mga --compress

# make ZIP
Compress-Archive -Force -Path $BUILD_DIR\* -DestinationPath .\build\DREAM_EMU.ZIP
