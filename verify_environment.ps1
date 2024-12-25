# PowerShell script to verify development environment setup

Write-Host "=== Verifying Windows Development Environment Setup ===" -ForegroundColor Green

# Check PowerShell version
$PSVersion = $PSVersionTable.PSVersion
Write-Host "PowerShell Version: $PSVersion"

# Check Visual Studio Installation
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
    if ($vsPath) {
        Write-Host "✓ Visual Studio is installed at: $vsPath" -ForegroundColor Green
        
        # Check for required workloads
        $workloads = & $vsWhere -latest -property workloads
        if ($workloads -match "Microsoft.VisualStudio.Workload.ManagedDesktop") {
            Write-Host "✓ .NET Desktop Development workload is installed" -ForegroundColor Green
        } else {
            Write-Host "✗ .NET Desktop Development workload is missing" -ForegroundColor Red
        }
        
        if ($workloads -match "Microsoft.VisualStudio.Workload.NativeDesktop") {
            Write-Host "✓ Desktop Development with C++ workload is installed" -ForegroundColor Green
        } else {
            Write-Host "✗ Desktop Development with C++ workload is missing" -ForegroundColor Red
        }
    } else {
        Write-Host "✗ Visual Studio installation not found" -ForegroundColor Red
    }
} else {
    Write-Host "✗ vswhere.exe not found. Visual Studio might not be installed." -ForegroundColor Red
}

# Check .NET SDK
try {
    $dotnetVersion = dotnet --version
    Write-Host "✓ .NET SDK Version: $dotnetVersion" -ForegroundColor Green
    
    # Check if Windows Desktop SDK is installed
    $dotnetSdks = dotnet --list-sdks
    if ($dotnetSdks -match "Microsoft.WindowsDesktop.App") {
        Write-Host "✓ Windows Desktop SDK is installed" -ForegroundColor Green
    } else {
        Write-Host "✗ Windows Desktop SDK is missing" -ForegroundColor Red
    }
} catch {
    Write-Host "✗ .NET SDK not found" -ForegroundColor Red
}

# Check Windows SDK
$windowsSdkPath = "HKLM:\SOFTWARE\WOW6432Node\Microsoft\Microsoft SDKs\Windows\v10.0"
if (Test-Path $windowsSdkPath) {
    $sdkVersion = (Get-ItemProperty $windowsSdkPath).ProductVersion
    Write-Host "✓ Windows SDK Version: $sdkVersion" -ForegroundColor Green
} else {
    Write-Host "✗ Windows SDK not found" -ForegroundColor Red
}

# Check Build Tools
$msbuildPath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
if (Test-Path $msbuildPath) {
    Write-Host "✓ MSBuild tools found" -ForegroundColor Green
} else {
    Write-Host "✗ MSBuild tools not found" -ForegroundColor Red
}

# Verify Solution Build
Write-Host "`nAttempting to build solution..." -ForegroundColor Yellow
try {
    dotnet build SmartAssistant.sln --configuration Debug
    Write-Host "✓ Solution builds successfully" -ForegroundColor Green
} catch {
    Write-Host "✗ Solution build failed: $_" -ForegroundColor Red
}

Write-Host "`nEnvironment verification complete. Please address any items marked with ✗" -ForegroundColor Yellow
