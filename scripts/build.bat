@echo off

echo Building Kiwi with MSVC

cd ..

if not exist build\NUL mkdir build
if not exist bin\NUL mkdir bin

cl /std:c++17 /W4 /O2 /GL /Ikiwi\include /c kiwi\src\main.cpp /Fo:build\main.obj

if %ERRORLEVEL% neq 0 (
  echo Build failed.
  exit /b %ERRORLEVEL%
)

echo Linking...

link /LTCG build\main.obj /OUT:bin\kiwi.exe

if %ERRORLEVEL% neq 0 (
  echo Linking failed.
  exit /b %ERRORLEVEL%
)

echo Build succeeded.
