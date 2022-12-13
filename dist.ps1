$BUILD_DIR= ".\build\dream\Release"

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022"
cmake --build build -j $Env:NUMBER_OF_PROCESSORS --config Release
Copy-Item -Force -Recurse assets $BUILD_DIR

# make ZIP

Compress-Archive -Force -Path $BUILD_DIR\* -DestinationPath .\build\DREAM_EMU.ZIP

