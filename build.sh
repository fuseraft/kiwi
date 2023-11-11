#!/bin/bash
g++ -rdynamic -o noctis -Wall src/main.cpp

if [ $? -eq 0 ]; then
    ./noctis test.ns
else
    echo "Failed to compile."
fi