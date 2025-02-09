#!/bin/sh

cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM=/Applications/CLion.app/Contents/bin/ninja/mac/aarch64/ninja -G Ninja -S ./ -B ./cmake-build-debug -DIN_RECEIVING_MODE=1
