@echo off

echo Building Astral with MSVC

if not exist build\NUL mkdir build
if not exist bin\NUL mkdir bin

cl /std:c++17 /W4 /Iinclude /c src\main.cpp /Fo:build\main.obj

if %ERRORLEVEL% neq 0 (
  echo Build failed.
  exit /b %ERRORLEVEL%
)

echo Linking...

link build\main.obj /OUT:bin\astral.exe

if %ERRORLEVEL% neq 0 (
  echo Linking failed.
  exit /b %ERRORLEVEL%
)

echo Build succeeded.