#Requires -Version 5.1
<#
.SYNOPSIS
  Kiwi Language Installer for Windows

.DESCRIPTION
  Installs the Kiwi programming language on Windows.
  Downloads a prebuilt release. With -Source, or when there's no prebuilt release,
  builds from source with the .NET 10 SDK.

.PARAMETER User
  Install for the current user only (default: $env:LOCALAPPDATA\kiwi)

.PARAMETER System
  Install system-wide to $env:ProgramFiles\kiwi  (requires Administrator)

.PARAMETER Prefix
  Install to a custom directory path

.PARAMETER Version
  Install this release, such as 1.8.0, instead of the latest

.PARAMETER Source
  Build from source instead of downloading a release

.PARAMETER Url
  Download a binary from this URL, with the standard library from main

.PARAMETER UseExisting
  Use the binary already present in .\bin\ instead of building from source

.PARAMETER Uninstall
  Remove Kiwi from the system

.PARAMETER Update
  Remove the previous install and reinstall the latest version

.EXAMPLE
  .\install.ps1
  .\install.ps1 -System
  .\install.ps1 -Prefix "C:\tools\kiwi"
  .\install.ps1 -Version 1.8.0
  .\install.ps1 -Source
  .\install.ps1 -Url "https://example.com/kiwi.exe"
  .\install.ps1 -UseExisting
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

  [string]$Version,
  [switch]$Source,
  [string]$Url,
  [switch]$UseExisting,
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
# Where release archives are downloaded from. CI points this at a local copy.
$ReleasesUrl = if ($env:KIWI_RELEASES_URL) { $env:KIWI_RELEASES_URL } else { "$RepoUrl/releases" }

# Windows PowerShell 5.1 may not offer TLS 1.2, which GitHub requires.
[Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [Net.SecurityProtocolType]::Tls12

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

if ($Version) {
  $Version = $Version.TrimStart('v')
  if ($Version -notmatch '^\d+\.\d+\.\d+$') {
    Write-Fatal "-Version takes a release number such as 1.8.0, not '$Version'."
  }
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
  if ($Url)     { $passArgs += "-Url `"$Url`"" }
  if ($Version) { $passArgs += "-Version $Version" }
  if ($Source)  { $passArgs += '-Source' }
  if ($Update)  { $passArgs += '-Update' }
  Request-Elevation $passArgs
}

# Each way of getting Kiwi fills $Stage with bin\ and lib\, which are installed only once they're
# complete, so a failed download or build leaves an existing install as it was.
$Stage = Join-Path $env:TEMP "kiwi_stage_$(Get-Random)"
$StageBin = Join-Path $Stage 'bin'
$StageLib = Join-Path $Stage 'lib'
New-Item -ItemType Directory -Force -Path $StageBin | Out-Null
New-Item -ItemType Directory -Force -Path $StageLib | Out-Null
$Cleanup = @($Stage)

$ScriptDir = if ($PSCommandPath) { Split-Path -Parent $PSCommandPath } else { (Get-Location).Path }

function Remove-Temp {
  foreach ($dir in $script:Cleanup) {
    if (Test-Path $dir) { Remove-Item -Recurse -Force $dir -ErrorAction SilentlyContinue }
  }
}

function Stop-Install {
  param([string]$Msg)
  Remove-Temp
  Write-Fatal $Msg
}

# -------------------------------------------------------------------------
# Download a release. Returns $false when the archive isn't available.
# -------------------------------------------------------------------------
function Get-Release {
  $asset = "kiwi-$RuntimeId.zip"
  $base = if ($Version) { "$ReleasesUrl/download/v$Version" } else { "$ReleasesUrl/latest/download" }

  $tmp = Join-Path $env:TEMP "kiwi_download_$(Get-Random)"
  New-Item -ItemType Directory -Force -Path $tmp | Out-Null
  $script:Cleanup += $tmp
  $archive = Join-Path $tmp $asset
  $sums = Join-Path $tmp 'SHA256SUMS'

  try { Get-File -Uri "$base/$asset" -Dest $archive } catch { return $false }
  try { Get-File -Uri "$base/SHA256SUMS" -Dest $sums } catch {
    Stop-Install "Couldn't download $base/SHA256SUMS to verify the archive."
  }

  $expected = $null
  foreach ($line in Get-Content $sums) {
    $fields = $line -split '\s+', 2
    if ($fields.Count -eq 2 -and $fields[1].TrimStart('*') -eq $asset) { $expected = $fields[0] }
  }
  if (-not $expected) { Stop-Install "SHA256SUMS has no checksum for $asset." }
  $actual = (Get-FileHash -Algorithm SHA256 -Path $archive).Hash.ToLowerInvariant()
  if ($actual -ne $expected.ToLowerInvariant()) {
    Stop-Install "The download doesn't match its checksum (expected $expected, got $actual). Nothing was installed."
  }
  Write-Success 'Checksum verified'

  Expand-Archive -Path $archive -DestinationPath $tmp -Force
  $root = Join-Path $tmp 'kiwi'
  if (-not (Test-Path (Join-Path $root 'bin\kiwi.exe')) -or -not (Test-Path (Join-Path $root 'lib'))) {
    Stop-Install "$asset doesn't contain kiwi\bin\kiwi.exe and kiwi\lib."
  }
  Copy-Item -Recurse -Force (Join-Path $root 'bin\*') $StageBin
  Copy-Item -Recurse -Force (Join-Path $root 'lib\*') $StageLib
  Write-Success 'Archive downloaded'
  return $true
}

# -------------------------------------------------------------------------
# Build from source
# -------------------------------------------------------------------------
function Build-FromSource {
  Write-Header 'Building Kiwi from source'

  # Check .NET SDK
  $DotnetExe = Get-Command dotnet -ErrorAction SilentlyContinue
  if (-not $DotnetExe) {
    Stop-Install ".NET 10 SDK not found.`nInstall from: https://dotnet.microsoft.com/download/dotnet/10.0"
  }

  $DotnetVer = (dotnet --version 2>$null) -replace '\s',''
  $DotnetMajor = [int]($DotnetVer -split '\.')[0]
  if ($DotnetMajor -lt 10) {
    Stop-Install "Kiwi requires .NET 10 or higher. Found: $DotnetVer"
  }
  Write-Info ".NET SDK $DotnetVer"

  # Locate or clone the repository
  if ((Test-Path (Join-Path $ScriptDir 'src\kiwi.csproj')) -and -not $Version) {
    $RepoDir = $ScriptDir
    Write-Info "Using local repository at $RepoDir"
  } else {
    $GitExe = Get-Command git -ErrorAction SilentlyContinue
    if (-not $GitExe) {
      Stop-Install 'git is required to clone the Kiwi repository. Install Git for Windows and retry.'
    }
    $RepoDir = Join-Path $env:TEMP "kiwi_src_$(Get-Random)"
    $script:Cleanup += $RepoDir
    $cloneArgs = @('clone', '--depth=1')
    if ($Version) {
      $cloneArgs += @('--branch', "v$Version")
      Write-Info "Cloning Kiwi repository at v$Version..."
    } else {
      Write-Info 'Cloning Kiwi repository...'
    }
    # Windows PowerShell 5.1 turns git's progress output on stderr into errors.
    $ErrorActionPreference = 'Continue'
    git @cloneArgs $RepoUrl $RepoDir 2>&1 | Out-Null
    $ErrorActionPreference = 'Stop'
    if ($LASTEXITCODE -ne 0) { Stop-Install "Couldn't clone $RepoUrl." }
    Write-Success 'Repository cloned'
  }

  $BuildTmp = Join-Path $env:TEMP "kiwi_build_$(Get-Random)"
  New-Item -ItemType Directory -Force -Path $BuildTmp | Out-Null
  $script:Cleanup += $BuildTmp
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
    Write-Warn 'AOT build failed - falling back to non-AOT build'

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
      Stop-Install "Build failed. See $env:TEMP\kiwi_build_err.log for details."
    }
    Write-Success 'Non-AOT build succeeded'
  } else {
    Write-Success 'AOT build succeeded'
  }

  Copy-Item -Force (Join-Path $BuildTmp 'kiwi.exe') $StageBin
  $SettingsSrc = Join-Path $BuildTmp 'kiwi-settings.json'
  if (Test-Path $SettingsSrc) { Copy-Item -Force $SettingsSrc $StageBin }
  # Native DLLs produced by the build
  Get-ChildItem -Path $BuildTmp -Filter '*.dll' | ForEach-Object {
    Copy-Item -Force $_.FullName $StageBin
  }
  Copy-Item -Recurse -Force (Join-Path $RepoDir 'lib\*') $StageLib
}

# -------------------------------------------------------------------------
# Download, Build or Copy
# -------------------------------------------------------------------------
if ($UseExisting) {
  Write-Header 'Using existing binary'

  $LocalBin = Join-Path $ScriptDir 'bin\kiwi.exe'
  if (-not (Test-Path $LocalBin)) {
    Stop-Install "No existing binary found at $LocalBin. Build first with .\build.ps1 or omit -UseExisting."
  }

  Copy-Item -Force $LocalBin $StageBin
  $LocalSettings = Join-Path $ScriptDir 'bin\kiwi-settings.json'
  if (Test-Path $LocalSettings) { Copy-Item -Force $LocalSettings $StageBin }
  # Native DLLs produced alongside the binary
  Get-ChildItem -Path (Join-Path $ScriptDir 'bin') -Filter '*.dll' | ForEach-Object {
    Copy-Item -Force $_.FullName $StageBin
  }
  Copy-Item -Recurse -Force (Join-Path $ScriptDir 'lib\*') $StageLib
  Write-Success "Binary copied from $LocalBin"

} elseif ($Url) {
  Write-Header 'Downloading Kiwi binary'
  Get-File -Uri $Url -Dest (Join-Path $StageBin 'kiwi.exe')
  Write-Success 'Binary downloaded'

  # Clone repo just for stdlib and settings
  Write-Header 'Fetching standard library'
  $GitExe = Get-Command git -ErrorAction SilentlyContinue
  if (-not $GitExe) {
    Stop-Install 'git is required to fetch the standard library. Install Git for Windows and retry.'
  }

  $StdlibTmp = Join-Path $env:TEMP "kiwi_stdlib_$(Get-Random)"
  $Cleanup += $StdlibTmp
  $ErrorActionPreference = 'Continue'
  git clone --depth=1 $RepoUrl $StdlibTmp 2>&1 | Out-Null
  $ErrorActionPreference = 'Stop'
  if ($LASTEXITCODE -ne 0) { Stop-Install "Couldn't clone $RepoUrl." }
  Copy-Item -Recurse -Force (Join-Path $StdlibTmp 'lib\*') $StageLib
  Copy-Item -Force (Join-Path $StdlibTmp 'src\kiwi-settings.json') $StageBin

} elseif ($Source) {
  Build-FromSource

} else {
  Write-Header $(if ($Version) { "Downloading Kiwi $Version" } else { 'Downloading Kiwi' })
  if (-not (Get-Release)) {
    if ($Version) {
      Stop-Install "Kiwi $Version has no prebuilt release for $RuntimeId. Pass -Source to build it."
    }
    if (Get-Command dotnet -ErrorAction SilentlyContinue) {
      Write-Warn "The latest release has no prebuilt binary for $RuntimeId. Building from source instead."
      Build-FromSource
    } else {
      Stop-Install "Couldn't download a prebuilt Kiwi for $RuntimeId, and the .NET 10 SDK isn't installed to build it from source."
    }
  }
}

# -------------------------------------------------------------------------
# Install
# -------------------------------------------------------------------------
# A binary that can't start on this system isn't installed.
$RunOutput = $null
$RunOk = $false
try {
  $RunOutput = & (Join-Path $StageBin 'kiwi.exe') -v 2>&1
  $RunOk = $LASTEXITCODE -eq 0 -and "$RunOutput".StartsWith('kiwi ')
} catch { $RunOutput = $_ }
if (-not $RunOk) {
  Write-Host "$RunOutput"
  Stop-Install "Kiwi can't run on this system, so nothing was installed."
}

# Update: remove old binary + lib
if ($Update -and (Test-Path $InstallPrefix)) {
  Write-Info 'Removing previous installation...'
  if (Test-Path $BinDir) { Remove-Item -Recurse -Force $BinDir }
  if (Test-Path $LibDir) { Remove-Item -Recurse -Force $LibDir }
  Write-Success 'Previous installation removed'
}

New-Item -ItemType Directory -Force -Path $BinDir | Out-Null
New-Item -ItemType Directory -Force -Path $LibDir | Out-Null
Copy-Item -Recurse -Force (Join-Path $StageBin '*') $BinDir
Copy-Item -Recurse -Force (Join-Path $StageLib '*') $LibDir
Remove-Temp
Write-Success "Installed $RunOutput to $InstallPrefix"

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
