#!/bin/bash

mkdir -p analyze
cp -r kiwi/include/* analyze
cp kiwi/src/main.cpp analyze
cppcheck --force --check-level=exhaustive --enable=performance analyze
rm -rf analyze