#Requires -Version 5.1
<#
.SYNOPSIS
  Build script for the Kiwi Language (Windows)

.DESCRIPTION
  Compiles Kiwi from source using the .NET 9 SDK and outputs the binary to .\bin\.
  Attempts an AOT build first; falls back to a non-AOT build if AOT fails.

.PARAMETER RuntimeId
  Target runtime identifier. Default: win-x64
  Supported: win-x64, linux-x64, linux-arm64, osx-x64, osx-arm64

.PARAMETER UseExisting
  Skip the build if .\bin\kiwi.exe already exists.

.EXAMPLE
  .\build.ps1
  .\build.ps1 -RuntimeId win-x64
  .\build.ps1 -UseExisting
#>

param (
  [ValidateSet('win-x64','linux-x64','linux-arm64','osx-x64','osx-arm64')]
  [string]$RuntimeId = 'win-x64',

  [switch]$UseExisting
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$SolutionPath = Join-Path $PSScriptRoot 'src\kiwi.csproj'
$OutputDir    = Join-Path $PSScriptRoot 'bin'

# Allow overriding RID via environment variable
if ($env:KIWI_RUNTIME_ID) { $RuntimeId = $env:KIWI_RUNTIME_ID }

# -------------------------------------------------------------------------
# --UseExisting: skip build if the binary is already present
# -------------------------------------------------------------------------
if ($UseExisting) {
  $ExistingBin = Join-Path $OutputDir 'kiwi.exe'
  if (Test-Path $ExistingBin) {
    Write-Host "Using existing binary at $ExistingBin (skipping build)."
    exit 0
  }
  Write-Host "No existing binary found at $ExistingBin — building from source."
}

# -------------------------------------------------------------------------
# .NET SDK check
# -------------------------------------------------------------------------
if (-not (Get-Command dotnet -ErrorAction SilentlyContinue)) {
  Write-Host "The 'dotnet' CLI could not be found. Please install the .NET 9 SDK and try again."
  exit 1
}

$DotnetVer   = (dotnet --version 2>$null) -replace '\s', ''
$DotnetMajor = [int]($DotnetVer -split '\.')[0]
if ($DotnetMajor -lt 9) {
  Write-Host "Kiwi requires .NET 9 or higher. Found: $DotnetVer"
  exit 1
}

# -------------------------------------------------------------------------
# Build
# -------------------------------------------------------------------------
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

Write-Host "Building Kiwi for $RuntimeId..."

$AotArgs = @(
  'publish', $SolutionPath,
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
  '-o', $OutputDir
)

$LogOut = Join-Path $env:TEMP 'kiwi_build.log'
$LogErr = Join-Path $env:TEMP 'kiwi_build_err.log'

$proc = Start-Process dotnet -ArgumentList $AotArgs -Wait -PassThru -NoNewWindow `
  -RedirectStandardOutput $LogOut `
  -RedirectStandardError  $LogErr

if ($proc.ExitCode -ne 0) {
  Write-Host 'AOT build failed. Output:'
  Get-Content $LogOut, $LogErr -ErrorAction SilentlyContinue | Write-Host
  Write-Host 'Falling back to non-AOT build...'

  $FallbackArgs = @(
    'publish', $SolutionPath,
    '-c', 'Release',
    '-r', $RuntimeId,
    '--self-contained', 'true',
    '-p:PublishTrimmed=true',
    '-p:PublishReadyToRun=true',
    '-p:StripSymbols=true',
    '-p:DebugType=none',
    '-p:IncludeNativeLibrariesForSelfExtract=true',
    '-o', $OutputDir
  )

  $proc2 = Start-Process dotnet -ArgumentList $FallbackArgs -Wait -PassThru -NoNewWindow `
    -RedirectStandardOutput $LogOut `
    -RedirectStandardError  $LogErr

  if ($proc2.ExitCode -ne 0) {
    Write-Host 'Fallback build failed. Output:'
    Get-Content $LogOut, $LogErr -ErrorAction SilentlyContinue | Write-Host
    exit 1
  }

  Write-Host 'Fallback build succeeded (non-AOT). Consider fixing AOT issues for optimal performance.'
}

# -------------------------------------------------------------------------
# Cleanup intermediate directories
# -------------------------------------------------------------------------
foreach ($dir in @('src\bin', 'src\obj', 'obj')) {
  $full = Join-Path $PSScriptRoot $dir
  if (Test-Path $full) { Remove-Item -Recurse -Force $full }
}

# Sync to existing user/system install if one is present
$SyncCandidates = @(
  (Join-Path $env:LOCALAPPDATA 'kiwi\bin'),
  (Join-Path $env:ProgramFiles  'kiwi\bin')
)

foreach ($InstallBinDir in $SyncCandidates) {
  $InstalledExe = Join-Path $InstallBinDir 'kiwi.exe'
  if (Test-Path $InstalledExe) {
    $InstallLibDir = Join-Path (Split-Path -Parent $InstallBinDir) 'lib'
    Copy-Item -Force (Join-Path $OutputDir 'kiwi.exe') $InstallBinDir
    $SettingsSrc = Join-Path $OutputDir 'kiwi-settings.json'
    if (Test-Path $SettingsSrc) { Copy-Item -Force $SettingsSrc $InstallBinDir }
    Get-ChildItem -Path $OutputDir -Filter '*.dll' | ForEach-Object {
      Copy-Item -Force $_.FullName $InstallBinDir
    }
    Copy-Item -Recurse -Force (Join-Path $PSScriptRoot 'lib\*') $InstallLibDir
    Write-Host "Synced to existing install at $InstallBinDir"
  }
}

Write-Host "Build succeeded! Try running '$OutputDir\kiwi.exe -h' and happy coding!"
