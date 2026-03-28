#!/usr/bin/env bash
# Kiwi Language Installer - Linux / macOS
# Usage: ./install.sh [OPTIONS]
# Pipe install: curl -sSL https://raw.githubusercontent.com/fuseraft/kiwi/main/install.sh | bash

set -euo pipefail 2>/dev/null || set -e

# -------------------------------------------------------------------------
# Colors
# -------------------------------------------------------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
BOLD='\033[1m'
NC='\033[0m'

REPO_URL="https://github.com/fuseraft/kiwi"

# -------------------------------------------------------------------------
# Logging
# -------------------------------------------------------------------------
info()    { printf "${BLUE}  •${NC} %s\n" "$*"; }
success() { printf "${GREEN}  ✓${NC} %s\n" "$*"; }
warn()    { printf "${YELLOW}  !${NC} %s\n" "$*"; }
die()     { printf "${RED}  ✗ ERROR:${NC} %s\n" "$*" >&2; exit 1; }
header()  { printf "\n${BOLD}%s${NC}\n" "$*"; }

# -------------------------------------------------------------------------
# OS / Architecture Detection
# -------------------------------------------------------------------------
detect_rid() {
  local os arch
  os="$(uname -s)"
  arch="$(uname -m)"
  case "$os" in
    Linux*)
      case "$arch" in
        x86_64)        echo "linux-x64" ;;
        aarch64|arm64) echo "linux-arm64" ;;
        *) die "Unsupported Linux architecture: $arch" ;;
      esac ;;
    Darwin*)
      case "$arch" in
        x86_64) echo "osx-x64" ;;
        arm64)  echo "osx-arm64" ;;
        *) die "Unsupported macOS architecture: $arch" ;;
      esac ;;
    CYGWIN*|MINGW*|MSYS*)
      echo "win-x64" ;;
    *) die "Unsupported OS: $os. Use install.ps1 on Windows." ;;
  esac
}

# -------------------------------------------------------------------------
# PATH helpers
# -------------------------------------------------------------------------
add_to_path() {
  local bin_dir="$1"
  local shell_rc added=false

  for shell_rc in "$HOME/.bashrc" "$HOME/.bash_profile" "$HOME/.zshrc" "$HOME/.profile"; do
    [[ -f "$shell_rc" ]] || continue
    if ! grep -qF "$bin_dir" "$shell_rc" 2>/dev/null; then
      printf '\n# Added by Kiwi installer\nexport PATH="%s:$PATH"\n' "$bin_dir" >> "$shell_rc"
      info "Updated PATH in $shell_rc"
      added=true
    fi
  done

  # If no rc file found, create ~/.profile
  if ! $added; then
    printf '\n# Added by Kiwi installer\nexport PATH="%s:$PATH"\n' "$bin_dir" >> "$HOME/.profile"
    info "Updated PATH in ~/.profile"
  fi
}

remove_from_path() {
  local bin_dir="$1"
  local shell_rc tmp

  for shell_rc in "$HOME/.bashrc" "$HOME/.bash_profile" "$HOME/.zshrc" "$HOME/.profile"; do
    [[ -f "$shell_rc" ]] || continue
    if grep -qF "$bin_dir" "$shell_rc" 2>/dev/null; then
      tmp="$(mktemp)"
      grep -v "# Added by Kiwi installer" "$shell_rc" \
        | grep -vF "export PATH=\"${bin_dir}" > "$tmp" || true
      mv "$tmp" "$shell_rc"
      info "Removed PATH entry from $shell_rc"
    fi
  done
}

# -------------------------------------------------------------------------
# Usage
# -------------------------------------------------------------------------
usage() {
  cat <<EOF
${BOLD}Kiwi Installer${NC} — Linux / macOS

USAGE:
  install.sh [OPTIONS]

OPTIONS:
  --user              Install for current user only in ~/.kiwi  (default)
  --system            Install system-wide to /opt/kiwi          (requires sudo)
  --prefix=PATH       Install to a custom directory
  --url=URL           Download a pre-built binary instead of building from source
  --use-existing      Use the binary already in ./bin/ instead of building
  --uninstall         Remove Kiwi
  --update            Remove the old install and reinstall the latest
  -h, --help          Show this help

EXAMPLES:
  ./install.sh                          # User install, build from source
  ./install.sh --system                 # System-wide install  (sudo)
  ./install.sh --prefix=/usr/local      # Custom prefix
  ./install.sh --url=https://...        # Install a pre-built binary
  ./install.sh --use-existing           # Install the binary already in ./bin/
  ./install.sh --update                 # Update to latest
  ./install.sh --uninstall              # Remove Kiwi

  # One-liner install from the web:
  curl -sSL https://raw.githubusercontent.com/fuseraft/kiwi/main/install.sh | bash

EOF
}

# -------------------------------------------------------------------------
# Argument Parsing
# -------------------------------------------------------------------------
INSTALL_MODE="user"  # user | system | custom
PREFIX=""
BINARY_URL=""
UNINSTALL=false
UPDATE=false
USE_EXISTING=false

for arg in "$@"; do
  case "$arg" in
    --user)         INSTALL_MODE="user" ;;
    --system)       INSTALL_MODE="system" ;;
    --prefix=*)     INSTALL_MODE="custom"; PREFIX="${arg#--prefix=}" ;;
    --url=*)        BINARY_URL="${arg#--url=}" ;;
    --use-existing) USE_EXISTING=true ;;
    --uninstall)    UNINSTALL=true ;;
    --update)       UPDATE=true ;;
    -h|--help)      usage; exit 0 ;;
    *) die "Unknown argument: $arg  (run with --help for usage)" ;;
  esac
done

# Resolve prefix
if [[ -z "$PREFIX" ]]; then
  case "$INSTALL_MODE" in
    user)   PREFIX="$HOME/.kiwi" ;;
    system) PREFIX="/opt/kiwi" ;;
  esac
fi

BIN_DIR="$PREFIX/bin"
LIB_DIR="$PREFIX/lib"

# -------------------------------------------------------------------------
# Uninstall
# -------------------------------------------------------------------------
if $UNINSTALL; then
  header "Uninstalling Kiwi"
  if [[ ! -d "$PREFIX" ]]; then
    warn "No Kiwi installation found at $PREFIX"
    exit 0
  fi
  rm -rf "$PREFIX"
  remove_from_path "$BIN_DIR"
  # Remove system symlink if present
  [[ -L "/usr/local/bin/kiwi" ]] && rm -f "/usr/local/bin/kiwi" && info "Removed /usr/local/bin/kiwi"
  success "Kiwi uninstalled from $PREFIX"
  exit 0
fi

# -------------------------------------------------------------------------
# Banner
# -------------------------------------------------------------------------
printf "${BOLD}${GREEN}"
cat <<'LOGO'

  '||       ||               ||  
   ||  ..  ...  ... ... ... ...  
   || .'    ||   ||  ||  |   ||  
   ||'|.    ||    ||| |||    ||  
  .||. ||. .||.    |   |    .||. 

LOGO
printf "${NC}"
header "Kiwi Installer"

RID="$(detect_rid)"
info "Platform  : $RID"
info "Prefix    : $PREFIX"
[[ -n "$BINARY_URL" ]] && info "Source    : pre-built binary"

# -------------------------------------------------------------------------
# Sudo Check for System Install
# -------------------------------------------------------------------------
if [[ "$INSTALL_MODE" == "system" && "${EUID:-$(id -u)}" -ne 0 ]]; then
  warn "System install requires root. Re-running with sudo..."
  exec sudo bash "$0" "$@"
fi

# -------------------------------------------------------------------------
# Update: wipe binary + lib before reinstalling
# -------------------------------------------------------------------------
if $UPDATE && [[ -d "$PREFIX" ]]; then
  info "Removing previous installation..."
  rm -rf "$BIN_DIR" "$LIB_DIR"
  success "Old installation removed"
fi

mkdir -p "$BIN_DIR" "$LIB_DIR"

# -------------------------------------------------------------------------
# Build or Download
# -------------------------------------------------------------------------
if $USE_EXISTING; then
  # -------------------------------------------------------------------------
  # Use the binary already present in ./bin/
  # -------------------------------------------------------------------------
  header "Using existing binary"

  SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-./install.sh}")" 2>/dev/null && pwd || pwd)"
  LOCAL_BIN="$SCRIPT_DIR/bin/kiwi"

  [[ -f "$LOCAL_BIN" ]] || die "No existing binary found at $LOCAL_BIN. Build first with ./build.sh or omit --use-existing."

  cp "$LOCAL_BIN" "$BIN_DIR/"
  chmod +x "$BIN_DIR/kiwi"
  success "Binary copied from $LOCAL_BIN"

  # Copy settings file if present
  [[ -f "$SCRIPT_DIR/bin/kiwi-settings.json" ]] && cp "$SCRIPT_DIR/bin/kiwi-settings.json" "$BIN_DIR/"

  # Copy any native libraries produced alongside the binary
  find "$SCRIPT_DIR/bin" -maxdepth 1 \( -name "*.so" -o -name "*.dylib" \) \
    -exec cp {} "$BIN_DIR/" \; 2>/dev/null || true

  # Install stdlib from the local repo
  cp -r "$SCRIPT_DIR/lib/." "$LIB_DIR/"
  success "Standard library installed"

elif [[ -n "$BINARY_URL" ]]; then
  # -------------------------------------------------------------------------
  # Download pre-built binary
  # -------------------------------------------------------------------------
  header "Downloading Kiwi binary"

  if command -v curl >/dev/null 2>&1; then
    curl -fL --progress-bar "$BINARY_URL" -o "$BIN_DIR/kiwi"
  elif command -v wget >/dev/null 2>&1; then
    wget -q --show-progress "$BINARY_URL" -O "$BIN_DIR/kiwi"
  else
    die "curl or wget is required to download Kiwi. Install one and retry."
  fi

  chmod +x "$BIN_DIR/kiwi"
  success "Binary downloaded"

  # Fetch stdlib from the repo (still needed even with a pre-built binary)
  header "Fetching standard library"
  command -v git >/dev/null 2>&1 || die "git is required to fetch the standard library."

  STDLIB_TMP="$(mktemp -d)"
  trap 'rm -rf "$STDLIB_TMP"' EXIT
  git clone --depth=1 "$REPO_URL" "$STDLIB_TMP" > /dev/null 2>&1
  cp -r "$STDLIB_TMP/lib/." "$LIB_DIR/"
  cp "$STDLIB_TMP/src/kiwi-settings.json" "$BIN_DIR/"
  success "Standard library installed"

else
  # -------------------------------------------------------------------------
  # Build from source
  # -------------------------------------------------------------------------
  header "Building Kiwi from source"

  # .NET SDK check
  command -v dotnet >/dev/null 2>&1 \
    || die ".NET 9 SDK not found. Install from: https://dotnet.microsoft.com/download/dotnet/9.0"

  DOTNET_VER="$(dotnet --version 2>/dev/null || echo "0.0.0")"
  DOTNET_MAJOR="${DOTNET_VER%%.*}"
  [[ "$DOTNET_MAJOR" -ge 9 ]] || die "Kiwi requires .NET 9 or higher. Found: $DOTNET_VER"
  info ".NET SDK $DOTNET_VER"

  # Locate repo — running from a clone, or need to clone fresh
  CLEANUP_REPO=false
  SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-./install.sh}")" 2>/dev/null && pwd || pwd)"

  if [[ -f "$SCRIPT_DIR/src/kiwi.csproj" ]]; then
    REPO_DIR="$SCRIPT_DIR"
    info "Using local repository at $REPO_DIR"
  else
    command -v git >/dev/null 2>&1 \
      || die "git is required to clone the Kiwi repository."
    REPO_DIR="$(mktemp -d)"
    CLEANUP_REPO=true
    info "Cloning Kiwi repository..."
    git clone --depth=1 "$REPO_URL" "$REPO_DIR" > /dev/null 2>&1
    success "Repository cloned"
  fi

  BUILD_TMP="$(mktemp -d)"
  trap '[[ "$CLEANUP_REPO" == "true" ]] && rm -rf "$REPO_DIR"; rm -rf "$BUILD_TMP"' EXIT

  info "Compiling... (this may take a minute)"

  BUILD_ARGS=(
    "$REPO_DIR/src/kiwi.csproj"
    -c Release -r "$RID" --self-contained true
    -p:PublishAot=true
    -p:PublishTrimmed=true
    -p:PublishReadyToRun=true
    -p:StripSymbols=true
    -p:DebugType=none
    -p:IncludeNativeLibrariesForSelfExtract=true
    -p:UseSystemTextJsonSourceGeneration=true
    -p:JsonSerializerIsReflectionEnabled=false
    -o "$BUILD_TMP"
  )

  if dotnet publish "${BUILD_ARGS[@]}" > /dev/null 2>&1; then
    success "AOT build succeeded"
  else
    warn "AOT build failed — falling back to non-AOT build"
    FALLBACK_ARGS=(
      "$REPO_DIR/src/kiwi.csproj"
      -c Release -r "$RID" --self-contained true
      -p:PublishTrimmed=true
      -p:PublishReadyToRun=true
      -p:StripSymbols=true
      -p:DebugType=none
      -p:IncludeNativeLibrariesForSelfExtract=true
      -o "$BUILD_TMP"
    )
    dotnet publish "${FALLBACK_ARGS[@]}" > /dev/null 2>&1 \
      || die "Build failed. Run './build.sh' directly to see detailed output."
    success "Non-AOT build succeeded"
  fi

  # Install binary
  cp "$BUILD_TMP/kiwi" "$BIN_DIR/"
  chmod +x "$BIN_DIR/kiwi"

  # Install settings file
  [[ -f "$BUILD_TMP/kiwi-settings.json" ]] && cp "$BUILD_TMP/kiwi-settings.json" "$BIN_DIR/"

  # Install any native shared libraries produced by the build
  find "$BUILD_TMP" -maxdepth 1 \( -name "*.so" -o -name "*.dylib" \) \
    -exec cp {} "$BIN_DIR/" \; 2>/dev/null || true

  # Install stdlib
  cp -r "$REPO_DIR/lib/." "$LIB_DIR/"
  success "Standard library installed"
fi

# -------------------------------------------------------------------------
# PATH / Symlink
# -------------------------------------------------------------------------
header "Configuring PATH"

if [[ "$INSTALL_MODE" == "system" ]]; then
  SYMLINK="/usr/local/bin/kiwi"
  ln -sf "$BIN_DIR/kiwi" "$SYMLINK"
  success "Symlinked: $SYMLINK → $BIN_DIR/kiwi"
else
  add_to_path "$BIN_DIR"
fi

# -------------------------------------------------------------------------
# Done
# -------------------------------------------------------------------------
header "Installation complete!"
printf "  Binary  : %s/kiwi\n" "$BIN_DIR"
printf "  Stdlib  : %s\n" "$LIB_DIR"
printf "\n"

if [[ "$INSTALL_MODE" != "system" ]]; then
  printf "${YELLOW}To start using Kiwi, restart your shell or run:${NC}\n"
  printf '  source ~/.bashrc   # or ~/.zshrc / ~/.profile\n'
  printf "\n"
fi

printf "Run ${BOLD}kiwi --help${NC} to get started.\n\n"
