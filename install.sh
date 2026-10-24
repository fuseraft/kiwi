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
# Where release archives are downloaded from. CI points this at a local copy.
RELEASES_URL="${KIWI_RELEASES_URL:-$REPO_URL/releases}"

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
# Download helpers
# -------------------------------------------------------------------------
fetch() {
  local url="$1" dest="$2"
  if command -v curl >/dev/null 2>&1; then
    curl -fsSL "$url" -o "$dest"
  elif command -v wget >/dev/null 2>&1; then
    wget -q "$url" -O "$dest"
  else
    die "curl or wget is required to download Kiwi. Install one and retry."
  fi
}

sha256() {
  if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "$1" | cut -d' ' -f1
  elif command -v shasum >/dev/null 2>&1; then
    shasum -a 256 "$1" | cut -d' ' -f1
  else
    die "sha256sum or shasum is required to verify the download."
  fi
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

Downloads a prebuilt release of Kiwi. With --source, or when there's no
prebuilt release for your platform, builds from source with the .NET 10 SDK.

USAGE:
  install.sh [OPTIONS]

OPTIONS:
  --user              Install for current user only in ~/.kiwi  (default)
  --system            Install system-wide to /opt/kiwi          (requires sudo)
  --prefix=PATH       Install to a custom directory
  --version=X.Y.Z     Install this release instead of the latest
  --source            Build from source instead of downloading a release
  --url=URL           Download a binary from URL, with the standard library from main
  --use-existing      Use the binary already in ./bin/ instead of building
  --uninstall         Remove Kiwi
  --update            Remove the old install and reinstall the latest
  -h, --help          Show this help

EXAMPLES:
  ./install.sh                          # User install of the latest release
  ./install.sh --version=1.8.0          # A specific release
  ./install.sh --system                 # System-wide install  (sudo)
  ./install.sh --prefix=/usr/local      # Custom prefix
  ./install.sh --source                 # Build from source
  ./install.sh --use-existing           # Install the binary already in ./bin/
  ./install.sh --update                 # Update to latest
  ./install.sh --uninstall              # Remove Kiwi

  # One-liner install from the web:
  curl -sSL https://raw.githubusercontent.com/fuseraft/kiwi/main/install.sh | bash

  # With options:
  curl -sSL https://raw.githubusercontent.com/fuseraft/kiwi/main/install.sh | bash -s -- --version=1.8.0

EOF
}

# -------------------------------------------------------------------------
# Argument Parsing
# -------------------------------------------------------------------------
INSTALL_MODE="user"  # user | system | custom
PREFIX=""
BINARY_URL=""
VERSION=""
FROM_SOURCE=false
UNINSTALL=false
UPDATE=false
USE_EXISTING=false

for arg in "$@"; do
  case "$arg" in
    --user)         INSTALL_MODE="user" ;;
    --system)       INSTALL_MODE="system" ;;
    --prefix=*)     INSTALL_MODE="custom"; PREFIX="${arg#--prefix=}" ;;
    --version=*)    VERSION="${arg#--version=}"; VERSION="${VERSION#v}" ;;
    --source)       FROM_SOURCE=true ;;
    --url=*)        BINARY_URL="${arg#--url=}" ;;
    --use-existing) USE_EXISTING=true ;;
    --uninstall)    UNINSTALL=true ;;
    --update)       UPDATE=true ;;
    -h|--help)      usage; exit 0 ;;
    *) die "Unknown argument: $arg  (run with --help for usage)" ;;
  esac
done

if [[ -n "$VERSION" ]] && ! [[ "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  die "--version takes a release number such as 1.8.0, not '$VERSION'."
fi

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

# -------------------------------------------------------------------------
# Sudo Check for System Install
# -------------------------------------------------------------------------
if [[ "$INSTALL_MODE" == "system" && "${EUID:-$(id -u)}" -ne 0 ]]; then
  warn "System install requires root. Re-running with sudo..."
  exec sudo bash "$0" "$@"
fi

# Each way of getting Kiwi fills STAGE with bin/ and lib/, which are installed only once they're
# complete, so a failed download or build leaves an existing install as it was.
STAGE="$(mktemp -d)"
CLEANUP=("$STAGE")
trap 'rm -rf "${CLEANUP[@]}"' EXIT
mkdir -p "$STAGE/bin" "$STAGE/lib"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-./install.sh}")" 2>/dev/null && pwd || pwd)"

# -------------------------------------------------------------------------
# Download a release. Returns 1 when the archive isn't available.
# -------------------------------------------------------------------------
download_release() {
  local asset="kiwi-$RID.tar.gz" base tmp expected actual
  if [[ -n "$VERSION" ]]; then
    base="$RELEASES_URL/download/v$VERSION"
  else
    base="$RELEASES_URL/latest/download"
  fi

  tmp="$(mktemp -d)"
  CLEANUP+=("$tmp")
  info "Downloading $base/$asset"
  fetch "$base/$asset" "$tmp/$asset" 2>/dev/null || return 1
  fetch "$base/SHA256SUMS" "$tmp/SHA256SUMS" 2>/dev/null \
    || die "Couldn't download $base/SHA256SUMS to verify the archive."

  expected="$(awk -v f="$asset" '$2 == f || $2 == "*" f { print $1 }' "$tmp/SHA256SUMS")"
  [[ -n "$expected" ]] || die "SHA256SUMS has no checksum for $asset."
  actual="$(sha256 "$tmp/$asset")"
  [[ "$actual" == "$expected" ]] \
    || die "The download doesn't match its checksum (expected $expected, got $actual). Nothing was installed."
  success "Checksum verified"

  tar -xzf "$tmp/$asset" -C "$tmp"
  [[ -x "$tmp/kiwi/bin/kiwi" && -d "$tmp/kiwi/lib" ]] || die "$asset doesn't contain kiwi/bin/kiwi and kiwi/lib."
  cp -R "$tmp/kiwi/bin/." "$STAGE/bin/"
  cp -R "$tmp/kiwi/lib/." "$STAGE/lib/"
  success "Archive downloaded"
}

# -------------------------------------------------------------------------
# Build from source
# -------------------------------------------------------------------------
build_from_source() {
  header "Building Kiwi from source"

  # .NET SDK check
  command -v dotnet >/dev/null 2>&1 \
    || die ".NET 10 SDK not found. Install from: https://dotnet.microsoft.com/download/dotnet/10.0"

  local dotnet_ver dotnet_major repo_dir build_tmp
  dotnet_ver="$(dotnet --version 2>/dev/null || echo "0.0.0")"
  dotnet_major="${dotnet_ver%%.*}"
  [[ "$dotnet_major" -ge 10 ]] || die "Kiwi requires .NET 10 or higher. Found: $dotnet_ver"
  info ".NET SDK $dotnet_ver"

  # Locate repo — running from a clone, or need to clone fresh
  if [[ -f "$SCRIPT_DIR/src/kiwi.csproj" && -z "$VERSION" ]]; then
    repo_dir="$SCRIPT_DIR"
    info "Using local repository at $repo_dir"
  else
    command -v git >/dev/null 2>&1 \
      || die "git is required to clone the Kiwi repository."
    repo_dir="$(mktemp -d)"
    CLEANUP+=("$repo_dir")
    info "Cloning Kiwi repository${VERSION:+ at v$VERSION}..."
    git clone --depth=1 ${VERSION:+--branch "v$VERSION"} "$REPO_URL" "$repo_dir" > /dev/null 2>&1 \
      || die "Couldn't clone $REPO_URL${VERSION:+ at v$VERSION}."
    success "Repository cloned"
  fi

  build_tmp="$(mktemp -d)"
  CLEANUP+=("$build_tmp")

  info "Compiling... (this may take a minute)"

  local build_args=(
    "$repo_dir/src/kiwi.csproj"
    -c Release -r "$RID" --self-contained true
    -p:PublishAot=true
    -p:PublishTrimmed=true
    -p:PublishReadyToRun=true
    -p:StripSymbols=true
    -p:DebugType=none
    -p:IncludeNativeLibrariesForSelfExtract=true
    -p:UseSystemTextJsonSourceGeneration=true
    -p:JsonSerializerIsReflectionEnabled=false
    -o "$build_tmp"
  )

  if dotnet publish "${build_args[@]}" > /dev/null 2>&1; then
    success "AOT build succeeded"
  else
    warn "AOT build failed — falling back to non-AOT build"
    local fallback_args=(
      "$repo_dir/src/kiwi.csproj"
      -c Release -r "$RID" --self-contained true
      -p:PublishTrimmed=true
      -p:PublishReadyToRun=true
      -p:StripSymbols=true
      -p:DebugType=none
      -p:IncludeNativeLibrariesForSelfExtract=true
      -o "$build_tmp"
    )
    dotnet publish "${fallback_args[@]}" > /dev/null 2>&1 \
      || die "Build failed. Run './build.sh' directly to see detailed output."
    success "Non-AOT build succeeded"
  fi

  cp "$build_tmp/kiwi" "$STAGE/bin/"
  [[ -f "$build_tmp/kiwi-settings.json" ]] && cp "$build_tmp/kiwi-settings.json" "$STAGE/bin/"
  # Native shared libraries produced by the build
  find "$build_tmp" -maxdepth 1 \( -name "*.so" -o -name "*.dylib" \) \
    -exec cp {} "$STAGE/bin/" \; 2>/dev/null || true
  cp -R "$repo_dir/lib/." "$STAGE/lib/"
}

# -------------------------------------------------------------------------
# Download, Build or Copy
# -------------------------------------------------------------------------
if $USE_EXISTING; then
  header "Using existing binary"

  LOCAL_BIN="$SCRIPT_DIR/bin/kiwi"
  [[ -f "$LOCAL_BIN" ]] || die "No existing binary found at $LOCAL_BIN. Build first with ./build.sh or omit --use-existing."

  cp "$LOCAL_BIN" "$STAGE/bin/"
  [[ -f "$SCRIPT_DIR/bin/kiwi-settings.json" ]] && cp "$SCRIPT_DIR/bin/kiwi-settings.json" "$STAGE/bin/"
  find "$SCRIPT_DIR/bin" -maxdepth 1 \( -name "*.so" -o -name "*.dylib" \) \
    -exec cp {} "$STAGE/bin/" \; 2>/dev/null || true
  cp -R "$SCRIPT_DIR/lib/." "$STAGE/lib/"
  success "Binary copied from $LOCAL_BIN"

elif [[ -n "$BINARY_URL" ]]; then
  header "Downloading Kiwi binary"
  fetch "$BINARY_URL" "$STAGE/bin/kiwi" || die "Couldn't download $BINARY_URL."
  success "Binary downloaded"

  # Fetch stdlib from the repo (still needed even with a pre-built binary)
  header "Fetching standard library"
  command -v git >/dev/null 2>&1 || die "git is required to fetch the standard library."

  STDLIB_TMP="$(mktemp -d)"
  CLEANUP+=("$STDLIB_TMP")
  git clone --depth=1 "$REPO_URL" "$STDLIB_TMP" > /dev/null 2>&1
  cp -R "$STDLIB_TMP/lib/." "$STAGE/lib/"
  cp "$STDLIB_TMP/src/kiwi-settings.json" "$STAGE/bin/"

elif $FROM_SOURCE; then
  build_from_source

else
  [[ "$RID" == win-* ]] && die "Use install.ps1 to install a release on Windows, or pass --source."
  header "Downloading Kiwi${VERSION:+ $VERSION}"
  if ! download_release; then
    [[ -n "$VERSION" ]] && die "Kiwi $VERSION has no prebuilt release for $RID. Pass --source to build it."
    if command -v dotnet >/dev/null 2>&1; then
      warn "The latest release has no prebuilt binary for $RID. Building from source instead."
      build_from_source
    else
      die "Couldn't download a prebuilt Kiwi for $RID, and the .NET 10 SDK isn't installed to build it from source."
    fi
  fi
fi

# -------------------------------------------------------------------------
# Install
# -------------------------------------------------------------------------
chmod +x "$STAGE/bin/kiwi"

# A binary that can't start on this system (a missing libicu, an older glibc) isn't installed.
if ! RUN_OUTPUT="$("$STAGE/bin/kiwi" -v 2>&1)"; then
  printf '%s\n' "$RUN_OUTPUT" | head -3 >&2
  die "Kiwi can't run on this system, so nothing was installed. Prebuilt Linux releases need glibc 2.34 or later and libicu."
fi

# Update: wipe binary + lib before reinstalling
if $UPDATE && [[ -d "$PREFIX" ]]; then
  info "Removing previous installation..."
  rm -rf "$BIN_DIR" "$LIB_DIR"
  success "Old installation removed"
fi

mkdir -p "$BIN_DIR" "$LIB_DIR"
cp -R "$STAGE/bin/." "$BIN_DIR/"
cp -R "$STAGE/lib/." "$LIB_DIR/"
success "Installed $RUN_OUTPUT to $PREFIX"

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
