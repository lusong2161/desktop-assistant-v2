param(
    [switch]$Release,
    [string]$Version = "1.0.0",
    [string]$OutputDir = "dist"
)

$ErrorActionPreference = "Stop"

# Configuration
$configuration = if ($Release) { "Release" } else { "Debug" }
$platform = "x64"
$projectRoot = Split-Path $PSScriptRoot -Parent

function Write-Status {
    param([string]$Message)
    Write-Host "=> $Message"
}

# Ensure clean output directory
Write-Status "Preparing output directory..."
$outputPath = Join-Path $projectRoot $OutputDir
if (Test-Path $outputPath) {
    Remove-Item -Path $outputPath -Recurse -Force
}
New-Item -ItemType Directory -Path $outputPath | Out-Null

# Build solution
Write-Status "Building solution..."
& "$projectRoot\build.ps1" -Release:$Release
if ($LASTEXITCODE -ne 0) { throw "Build failed" }

# Build installer
Write-Status "Building installer..."
Push-Location "$projectRoot\installer"
try {
    & ".\build_installer.ps1" -Release:$Release
    if ($LASTEXITCODE -ne 0) { throw "Installer build failed" }
    Move-Item "SmartAssistant-$configuration.msi" "$outputPath\SmartAssistant-$Version.msi"
} finally {
    Pop-Location
}

# Copy setup scripts
Write-Status "Copying setup scripts..."
Copy-Item "$projectRoot\setup\*.ps1" $outputPath

# Create portable version
Write-Status "Creating portable version..."
$portablePath = Join-Path $outputPath "SmartAssistant-Portable-$Version"
New-Item -ItemType Directory -Path $portablePath | Out-Null

# Copy application files
Copy-Item "$projectRoot\SmartAssistant.UI\bin\$configuration\*" $portablePath -Recurse
Copy-Item "$projectRoot\build\$configuration\*.dll" "$portablePath\lib"

# Copy dependencies
$depsPath = Join-Path $portablePath "lib"
New-Item -ItemType Directory -Path $depsPath -Force | Out-Null

# OpenSSL
Copy-Item "C:\Program Files\OpenSSL-Win64\bin\libssl*.dll" $depsPath
Copy-Item "C:\Program Files\OpenSSL-Win64\bin\libcrypto*.dll" $depsPath

# SQLite
Copy-Item "C:\Program Files\SQLite\sqlite3.dll" $depsPath

# Create portable launcher
$launcherScript = @"
@echo off
set PATH=%~dp0lib;%PATH%
start "" "%~dp0SmartAssistant.exe"
"@
Set-Content -Path "$portablePath\SmartAssistant.cmd" -Value $launcherScript -Encoding ASCII

# Create ZIP archive
Write-Status "Creating ZIP archive..."
Compress-Archive -Path $portablePath\* -DestinationPath "$outputPath\SmartAssistant-Portable-$Version.zip" -Force

# Generate checksums
Write-Status "Generating checksums..."
Push-Location $outputPath
try {
    Get-ChildItem -File | ForEach-Object {
        $hash = Get-FileHash -Path $_.Name -Algorithm SHA256
        "$($hash.Hash.ToLower())  $($_.Name)" | Add-Content "checksums.txt"
    }
} finally {
    Pop-Location
}

# Create version info file
$versionInfo = @"
Version: $Version
Build Configuration: $configuration
Build Date: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Platform: $platform

Package Contents:
- SmartAssistant-$Version.msi: Full installer with all dependencies
- SmartAssistant-Portable-$Version.zip: Portable version (no installation required)
- setup.ps1: Installation script
- verify_installation.ps1: Installation verification script
- checksums.txt: SHA256 checksums for all files

System Requirements:
- Windows 10 or later (64-bit)
- .NET Framework 4.8 or later
- Visual C++ Redistributable 2015-2022
- 4GB RAM minimum
- 500MB free disk space
"@
Set-Content -Path "$outputPath\version_info.txt" -Value $versionInfo

Write-Status "Package created successfully!"
Write-Status "Output directory: $outputPath"
Write-Status "Version: $Version"
