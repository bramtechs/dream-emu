clear
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j $Env:NUMBER_OF_PROCESSORS
