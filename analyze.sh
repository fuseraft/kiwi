#!/bin/bash

mkdir -p analyze
cp -r include/* analyze
cp src/main.cpp analyze
cppcheck --enable=performance analyze
rm -rf analyze