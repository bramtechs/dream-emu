#!/bin/bash
if [ -f dev.txt ]; then
    rm -f forge
fi

if [ -f forge/dev.txt ]; then
    rm -f forge/forge
fi

if [ -f forge ]; then
    ./forge $1
    exit 0
fi

if [ -f ./forge/forge ]; then
    ./forge/forge $1
    exit 0
fi

if [ -f forge.cpp ]; then
    g++ -std=c++17 forge.cpp -o forge && ./forge $1
else
    g++ -std=c++17 forge/forge.cpp -o forge/forge && ./forge/forge $1
fi
