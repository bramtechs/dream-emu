#!/bin/bash
set -xe

ls

source ../emsdk/emsdk_env.sh

rm -rf build-web
emcmake cmake -S . -B build-web
cmake --build build-web
