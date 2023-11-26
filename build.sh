#!/bin/bash
g++ -rdynamic -o usl -Wall src/main.cpp

if [ $? -eq 0 ]; then
    ./usl test.uslang
else
    echo "Failed to compile."
fi