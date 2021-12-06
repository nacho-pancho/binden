#!/bin/sh
mkdir -p build
cmake -S code -B build
make -C build

