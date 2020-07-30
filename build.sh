#!/bin/bash
rm -rf build
mkdir build
cd build
#cmake ..
cmake -G Ninja ..
#cmake --build . -- -j16
ninja

