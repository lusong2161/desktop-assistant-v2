param(
    [switch]$Release
)

$ErrorActionPreference = "Stop"

# Configuration
$configuration = if ($Release) { "Release" } else { "Debug" }
$platform = "x64"
$wixPath = "${env:WIX}bin"
$projectPath = Split-Path $PSScriptRoot -Parent

# Build the main solution first
& "$projectPath\build.ps1" -Configuration $configuration
if ($LASTEXITCODE -ne 0) { throw "Build failed" }

# Set WiX variables
$env:SmartAssistant_UI_TargetPath = "$projectPath\SmartAssistant.UI\bin\$configuration"
$env:SmartAssistant_Core_TargetPath = "$projectPath\build\$configuration"

# Build installer
Write-Host "Building installer..."
& "$wixPath\candle.exe" -arch x64 Product.wxs
if ($LASTEXITCODE -ne 0) { throw "WiX candle failed" }

& "$wixPath\light.exe" -ext WixUIExtension Product.wixobj -out "SmartAssistant-$configuration.msi"
if ($LASTEXITCODE -ne 0) { throw "WiX light failed" }

Write-Host "Installer build completed successfully!"
