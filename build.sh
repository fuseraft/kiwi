#!/bin/bash
mkdir -p ./bin
g++ -rdynamic -o ./bin/usl -Wall ./src/main.cpp

if [ $? -eq 0 ]; then
    ./bin/usl ./tests/test.uslang
else
    echo "Failed to compile."
fi