#!/bin/bash

mkdir -p analyze
cp -r kiwi/include/* analyze
cp kiwi/src/main.cpp analyze
cppcheck --enable=performance analyze
rm -rf analyze