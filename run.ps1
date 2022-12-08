rm .\build\space\Debug\space.exe
cmake --build build -j $Env:NUMBER_OF_PROCESSORS
.\build\space\Debug\space.exe
