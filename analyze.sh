#!/bin/bash

mkdir -p analyze
cp -r include/* analyze
cp src/main.cpp analyze
cppcheck --enable=all analyze
rm -rf analyze