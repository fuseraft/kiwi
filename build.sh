#!/bin/bash
mkdir -p ./bin
find . -iname "*.cpp" -o -iname "*.h" | xargs clang-format -i --style=file
g++ -rdynamic -o ./bin/usl -Wall ./src/main.cpp

if [ $? -eq 0 ]; then
    ./bin/usl ./tests/test.uslang
else
    echo "Failed to compile."
fi