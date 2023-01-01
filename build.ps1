# cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 17 2022"
cmake --build build -j $Env:NUMBER_OF_PROCESSORS
