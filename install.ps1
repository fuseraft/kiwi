#Requires -Version 5.1
<#
.SYNOPSIS
  Kiwi Language Installer for Windows

.DESCRIPTION
  Installs the Kiwi programming language on Windows.
  Builds from source (requires .NET 9 SDK) or downloads a pre-built binary.

.PARAMETER User
  Install for the current user only (default: $env:LOCALAPPDATA\kiwi)

.PARAMETER System
  Install system-wide to $env:ProgramFiles\kiwi  (requires Administrator)

.PARAMETER Prefix
  Install to a custom directory path

.PARAMETER Url
  Download a pre-built binary from this URL instead of building from source

.PARAMETER Uninstall
  Remove Kiwi from the system

.PARAMETER Update
  Remove the previous install and reinstall the latest version

.EXAMPLE
  .\install.ps1
  .\install.ps1 -System
  .\install.ps1 -Prefix "C:\tools\kiwi"
  .\install.ps1 -Url "https://example.com/kiwi.exe"
  .\install.ps1 -Update
  .\install.ps1 -Uninstall

.NOTES
  One-liner install from the web (run in an elevated PowerShell prompt if using -System):
  irm https://raw.githubusercontent.com/fuseraft/kiwi/main/install.ps1 | iex
#>

[CmdletBinding(DefaultParameterSetName = 'User')]
param (
  [Parameter(ParameterSetName = 'User')]
  [switch]$User,

  [Parameter(ParameterSetName = 'System')]
  [switch]$System,

  [Parameter(ParameterSetName = 'Custom')]
  [string]$Prefix,

  [string]$Url,
  [switch]$Uninstall,
  [switch]$Update
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# -------------------------------------------------------------------------
# Constants
# -------------------------------------------------------------------------
$RepoUrl  = 'https://github.com/fuseraft/kiwi'
$RepoRaw  = 'https://raw.githubusercontent.com/fuseraft/kiwi/main'
$RuntimeId = 'win-x64'

# -------------------------------------------------------------------------
# Colors / Logging
# -------------------------------------------------------------------------
function Write-Info    { param([string]$Msg) Write-Host "  $([char]0x2022) $Msg" -ForegroundColor Cyan }
function Write-Success { param([string]$Msg) Write-Host "  $([char]0x2713) $Msg" -ForegroundColor Green }
function Write-Warn    { param([string]$Msg) Write-Host "  ! $Msg" -ForegroundColor Yellow }
function Write-Header  { param([string]$Msg) Write-Host "`n$Msg" -ForegroundColor White }
function Write-Fatal   { param([string]$Msg) Write-Host "  X ERROR: $Msg" -ForegroundColor Red; exit 1 }


# -------------------------------------------------------------------------
# Privilege Check
# -------------------------------------------------------------------------
function Test-IsAdmin {
  $id = [System.Security.Principal.WindowsIdentity]::GetCurrent()
  $principal = New-Object System.Security.Principal.WindowsPrincipal($id)
  return $principal.IsInRole([System.Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Request-Elevation {
  param([string[]]$ScriptArgs)
  if (-not (Test-IsAdmin)) {
    Write-Warn 'System install requires Administrator. Relaunching as Administrator...'
    $argList = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $PSCommandPath) + $ScriptArgs
    Start-Process powershell -Verb RunAs -ArgumentList $argList
    exit 0
  }
}

# -------------------------------------------------------------------------
# PATH helpers
# -------------------------------------------------------------------------
function Add-ToUserPath {
  param([string]$Dir)
  $current = [Environment]::GetEnvironmentVariable('PATH', 'User')
  if ($current -notlike "*$Dir*") {
    [Environment]::SetEnvironmentVariable('PATH', "$Dir;$current", 'User')
    Write-Info "Added to user PATH: $Dir"
  }
  # Also update the current session
  $env:PATH = "$Dir;$env:PATH"
}

function Add-ToSystemPath {
  param([string]$Dir)
  $current = [Environment]::GetEnvironmentVariable('PATH', 'Machine')
  if ($current -notlike "*$Dir*") {
    [Environment]::SetEnvironmentVariable('PATH', "$Dir;$current", 'Machine')
    Write-Info "Added to system PATH: $Dir"
  }
  $env:PATH = "$Dir;$env:PATH"
}

function Remove-FromPath {
  param([string]$Dir)
  foreach ($scope in @('User', 'Machine')) {
    try {
      $current = [Environment]::GetEnvironmentVariable('PATH', $scope)
      if ($current -like "*$Dir*") {
        $updated = ($current -split ';' | Where-Object { $_ -ne $Dir }) -join ';'
        [Environment]::SetEnvironmentVariable('PATH', $updated, $scope)
        Write-Info "Removed from $scope PATH: $Dir"
      }
    } catch { }
  }
}

# -------------------------------------------------------------------------
# Download helper
# -------------------------------------------------------------------------
function Get-File {
  param([string]$Uri, [string]$Dest)
  Write-Info "Downloading: $Uri"
  $wc = New-Object System.Net.WebClient
  $wc.DownloadFile($Uri, $Dest)
}

# -------------------------------------------------------------------------
# Resolve install prefix
# -------------------------------------------------------------------------
if ($Prefix) {
  $InstallPrefix = $Prefix
} elseif ($System) {
  $InstallPrefix = Join-Path $env:ProgramFiles 'kiwi'
} else {
  $InstallPrefix = Join-Path $env:LOCALAPPDATA 'kiwi'
}

$BinDir = Join-Path $InstallPrefix 'bin'
$LibDir = Join-Path $InstallPrefix 'lib'

# -------------------------------------------------------------------------
# Uninstall
# -------------------------------------------------------------------------
if ($Uninstall) {
  Write-Header 'Uninstalling Kiwi'
  if (-not (Test-Path $InstallPrefix)) {
    Write-Warn "No Kiwi installation found at $InstallPrefix"
    exit 0
  }
  Remove-Item -Recurse -Force $InstallPrefix
  Remove-FromPath $BinDir
  Write-Success "Kiwi uninstalled from $InstallPrefix"
  exit 0
}

# -------------------------------------------------------------------------
# Banner
# -------------------------------------------------------------------------
Write-Host @"

  '||       ||               ||  
   ||  ..  ...  ... ... ... ...  
   || .'    ||   ||  ||  |   ||  
   ||'|.    ||    ||| |||    ||  
  .||. ||. .||.    |   |    .||. 

"@ -ForegroundColor Green

Write-Header 'Kiwi Installer'
Write-Info "Platform  : $RuntimeId"
Write-Info "Prefix    : $InstallPrefix"

# -------------------------------------------------------------------------
# Privilege Check for System Install
# -------------------------------------------------------------------------
if ($System -and -not (Test-IsAdmin)) {
  $passArgs = @('-System')
  if ($Url)    { $passArgs += "-Url `"$Url`"" }
  if ($Update) { $passArgs += '-Update' }
  Request-Elevation $passArgs
}

# -------------------------------------------------------------------------
# Update: remove old binary + lib
# -------------------------------------------------------------------------
if ($Update -and (Test-Path $InstallPrefix)) {
  Write-Info 'Removing previous installation...'
  if (Test-Path $BinDir) { Remove-Item -Recurse -Force $BinDir }
  if (Test-Path $LibDir) { Remove-Item -Recurse -Force $LibDir }
  Write-Success 'Previous installation removed'
}

New-Item -ItemType Directory -Force -Path $BinDir | Out-Null
New-Item -ItemType Directory -Force -Path $LibDir | Out-Null


# -------------------------------------------------------------------------
# Build or Download
# -------------------------------------------------------------------------
if ($Url) {
  # -------------------------------------------------------------------------
  # Download pre-built binary
  # -------------------------------------------------------------------------
  Write-Header 'Downloading Kiwi binary'

  $BinaryDest = Join-Path $BinDir 'kiwi.exe'
  Get-File -Uri $Url -Dest $BinaryDest
  Write-Success 'Binary downloaded'

  # Clone repo just for stdlib and settings
  Write-Header 'Fetching standard library'
  $GitExe = Get-Command git -ErrorAction SilentlyContinue
  if (-not $GitExe) {
    Write-Fatal 'git is required to fetch the standard library. Install Git for Windows and retry.'
  }

  $StdlibTmp = Join-Path $env:TEMP "kiwi_stdlib_$(Get-Random)"
  git clone --depth=1 $RepoUrl $StdlibTmp 2>&1 | Out-Null
  Copy-Item -Recurse -Force (Join-Path $StdlibTmp 'lib\*') $LibDir
  Copy-Item -Force (Join-Path $StdlibTmp 'src\kiwi-settings.json') $BinDir
  Remove-Item -Recurse -Force $StdlibTmp
  Write-Success 'Standard library installed'

} else {
  # -------------------------------------------------------------------------
  # Build from source
  # -------------------------------------------------------------------------
  Write-Header 'Building Kiwi from source'

  # Check .NET SDK
  $DotnetExe = Get-Command dotnet -ErrorAction SilentlyContinue
  if (-not $DotnetExe) {
    Write-Fatal ".NET 9 SDK not found.`nInstall from: https://dotnet.microsoft.com/download/dotnet/9.0"
  }

  $DotnetVer = (dotnet --version 2>$null) -replace '\s',''
  $DotnetMajor = [int]($DotnetVer -split '\.')[0]
  if ($DotnetMajor -lt 9) {
    Write-Fatal "Kiwi requires .NET 9 or higher. Found: $DotnetVer"
  }
  Write-Info ".NET SDK $DotnetVer"

  # Locate or clone the repository
  $CleanupRepo = $false
  $ScriptDir   = if ($PSCommandPath) { Split-Path -Parent $PSCommandPath } else { (Get-Location).Path }

  if (Test-Path (Join-Path $ScriptDir 'src\kiwi.csproj')) {
    $RepoDir = $ScriptDir
    Write-Info "Using local repository at $RepoDir"
  } else {
    $GitExe = Get-Command git -ErrorAction SilentlyContinue
    if (-not $GitExe) {
      Write-Fatal 'git is required to clone the Kiwi repository. Install Git for Windows and retry.'
    }
    $RepoDir = Join-Path $env:TEMP "kiwi_src_$(Get-Random)"
    Write-Info 'Cloning Kiwi repository...'
    git clone --depth=1 $RepoUrl $RepoDir 2>&1 | Out-Null
    Write-Success 'Repository cloned'
    $CleanupRepo = $true
  }

  $BuildTmp = Join-Path $env:TEMP "kiwi_build_$(Get-Random)"
  New-Item -ItemType Directory -Force -Path $BuildTmp | Out-Null
  $ProjPath = Join-Path $RepoDir 'src\kiwi.csproj'

  Write-Info 'Compiling... (this may take a minute)'

  $AotArgs = @(
    'publish', $ProjPath,
    '-c', 'Release',
    '-r', $RuntimeId,
    '--self-contained', 'true',
    '-p:PublishAot=true',
    '-p:PublishTrimmed=true',
    '-p:PublishReadyToRun=true',
    '-p:StripSymbols=true',
    '-p:DebugType=none',
    '-p:IncludeNativeLibrariesForSelfExtract=true',
    '-p:UseSystemTextJsonSourceGeneration=true',
    '-p:JsonSerializerIsReflectionEnabled=false',
    '-o', $BuildTmp
  )

  $proc = Start-Process dotnet -ArgumentList $AotArgs -Wait -PassThru -NoNewWindow `
    -RedirectStandardOutput (Join-Path $env:TEMP 'kiwi_build.log') `
    -RedirectStandardError  (Join-Path $env:TEMP 'kiwi_build_err.log')

  if ($proc.ExitCode -ne 0) {
    Write-Warn 'AOT build failed — falling back to non-AOT build'

    $FallbackArgs = @(
      'publish', $ProjPath,
      '-c', 'Release',
      '-r', $RuntimeId,
      '--self-contained', 'true',
      '-p:PublishTrimmed=true',
      '-p:PublishReadyToRun=true',
      '-p:StripSymbols=true',
      '-p:DebugType=none',
      '-p:IncludeNativeLibrariesForSelfExtract=true',
      '-o', $BuildTmp
    )

    $proc2 = Start-Process dotnet -ArgumentList $FallbackArgs -Wait -PassThru -NoNewWindow `
      -RedirectStandardOutput (Join-Path $env:TEMP 'kiwi_build.log') `
      -RedirectStandardError  (Join-Path $env:TEMP 'kiwi_build_err.log')

    if ($proc2.ExitCode -ne 0) {
      Write-Fatal "Build failed. See $env:TEMP\kiwi_build_err.log for details."
    }
    Write-Success 'Non-AOT build succeeded'
  } else {
    Write-Success 'AOT build succeeded'
  }

  # Install binary
  Copy-Item -Force (Join-Path $BuildTmp 'kiwi.exe') $BinDir

  # Install settings
  $SettingsSrc = Join-Path $BuildTmp 'kiwi-settings.json'
  if (Test-Path $SettingsSrc) {
    Copy-Item -Force $SettingsSrc $BinDir
  }

  # Install native DLLs produced by build
  Get-ChildItem -Path $BuildTmp -Filter '*.dll' | ForEach-Object {
    Copy-Item -Force $_.FullName $BinDir
  }

  # Install stdlib
  Copy-Item -Recurse -Force (Join-Path $RepoDir 'lib\*') $LibDir
  Write-Success 'Standard library installed'

  # Cleanup
  Remove-Item -Recurse -Force $BuildTmp
  if ($CleanupRepo) { Remove-Item -Recurse -Force $RepoDir }
}

# -------------------------------------------------------------------------
# PATH Registration
# -------------------------------------------------------------------------
Write-Header 'Configuring PATH'

if ($System -or (Test-IsAdmin -and -not $Prefix)) {
  Add-ToSystemPath $BinDir
} else {
  Add-ToUserPath $BinDir
}

# -------------------------------------------------------------------------
# Done
# -------------------------------------------------------------------------
Write-Header 'Installation complete!'
Write-Host "  Binary  : $BinDir\kiwi.exe"
Write-Host "  Stdlib  : $LibDir"
Write-Host ''
Write-Host 'Open a new terminal window, then run ' -NoNewline
Write-Host 'kiwi --help' -ForegroundColor Green -NoNewline
Write-Host ' to get started.'
Write-Host ''
