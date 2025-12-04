#!/usr/bin/env bash

# Ensure Bash is available (works in MinGW/Git Bash)
if ! type -p bash >/dev/null 2>&1; then
  echo "Bash is required to run this script. Please run in Git Bash, MSYS2, or another Bash-compatible environment."
  exit 1
fi

# Enable strict mode, with Windows compatibility
set -euo pipefail 2>/dev/null || set -e

# Check for dotnet CLI
if ! type -p dotnet >/dev/null 2>&1; then
  echo "The 'dotnet' CLI could not be found. Please install the .NET 8 SDK and try again."
  exit 1
fi

# Expect .NET 8
DOTNET_VERSION=$(dotnet --version 2>/dev/null || echo "unknown")
DOTNET_VERSION_MAJOR=${DOTNET_VERSION%%.*}

if [[ "$DOTNET_VERSION_MAJOR" -lt 8 ]]; then
  echo "Kiwi requires .NET 8 or higher. Found: $DOTNET_VERSION"
  exit 1
fi

SOLUTION_PATH="src/kiwi.csproj"
OUTPUT_DIR="bin"
# Detect OS for default RID
case "$(uname -s)" in
  Linux*)   DEFAULT_RID="linux-x64" ;;
  Darwin*)  DEFAULT_RID="osx-x64" ;;
  CYGWIN*|MINGW*|MSYS*) DEFAULT_RID="win-x64" ;;
  *)        DEFAULT_RID="win-x64" ;; # Fallback to win-x64 for Windows
esac
RUNTIME_ID="$DEFAULT_RID"  # Default RID; override with env var or argument

# Allow overriding runtime identifier via environment variable or argument
if [[ -n "${HAYWARD_RUNTIME_ID:-}" ]]; then
  RUNTIME_ID="$HAYWARD_RUNTIME_ID"
elif [[ $# -gt 0 ]]; then
  RUNTIME_ID="$1"
fi

# Validate common RIDs
case "$RUNTIME_ID" in
  "linux-x64"|"win-x64"|"osx-x64"|"osx-arm64"|"linux-arm64")
    ;;
  *)
    echo "Unsupported runtime identifier: $RUNTIME_ID. Supported: linux-x64, win-x64, osx-x64, osx-arm64, linux-arm64"
    exit 1
    ;;
esac

# Convert paths for Windows (MinGW/Git Bash)
if [[ "$RUNTIME_ID" == "win-x64" ]]; then
  SOLUTION_PATH=$(cygpath -m "$SOLUTION_PATH" 2>/dev/null || echo "$SOLUTION_PATH")
  OUTPUT_DIR=$(cygpath -m "$OUTPUT_DIR" 2>/dev/null || echo "$OUTPUT_DIR")
fi

BUILD_OUTPUT=""
echo "Building Kiwi with AOT and trimming for $RUNTIME_ID..."
if ! BUILD_OUTPUT=$(dotnet publish "$SOLUTION_PATH" -c Release -r "$RUNTIME_ID" --self-contained true \
  -p:PublishAot=true \
  -p:PublishTrimmed=true \
  -p:PublishReadyToRun=true \
  -p:StripSymbols=true \
  -p:DebugType=none \
  -p:IncludeNativeLibrariesForSelfExtract=true \
  -p:UseSystemTextJsonSourceGeneration=true \
  -p:JsonSerializerIsReflectionEnabled=false \
  -o "$OUTPUT_DIR" 2>&1); then
  echo "AOT build failed. Output:"
  echo "$BUILD_OUTPUT"
  echo "Falling back to non-AOT build..."
  if ! BUILD_OUTPUT=$(dotnet publish "$SOLUTION_PATH" -c Release -r "$RUNTIME_ID" --self-contained true \
    -p:PublishTrimmed=true \
    -p:PublishReadyToRun=true \
    -p:StripSymbols=true \
    -p:DebugType=none \
    -p:IncludeNativeLibrariesForSelfExtract=true \
    -o "$OUTPUT_DIR" 2>&1); then
    echo "Fallback build failed. Output:"
    echo "$BUILD_OUTPUT"
    exit 1
  fi
  echo "Fallback build succeeded (non-AOT). Consider fixing AOT issues for optimal performance."
fi

# Clean up unnecessary directories (Windows-compatible)
for dir in "src/bin" "src/obj" "obj"; do
  if [[ -d "$dir" ]]; then
    if [[ "$RUNTIME_ID" == "win-x64" ]]; then
      rmdir /s /q "$dir" 2>/dev/null || rm -rf "$dir" 2>/dev/null
    else
      rm -rf "$dir" 2>/dev/null
    fi
  fi
done

# Adjust execution message for Windows
if [[ "$RUNTIME_ID" == "win-x64" ]]; then
  echo "Build succeeded! Try running '$OUTPUT_DIR\\kiwi.exe -h' and happy coding!"
else
  echo "Build succeeded! Try running './$OUTPUT_DIR/kiwi -h' and happy coding!"
fi