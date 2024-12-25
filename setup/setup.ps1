param(
    [switch]$Silent
)

$ErrorActionPreference = "Stop"

# Configuration
$dotnetVersion = "4.8"
$vcRedistVersion = "14.34.31938"
$openSSLVersion = "1.1.1"
$sqliteVersion = "3.40.1"

function Write-Status {
    param([string]$Message)
    if (!$Silent) {
        Write-Host $Message
    }
}

function Test-AdminPrivileges {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Install-DotNetFramework {
    Write-Status "Checking .NET Framework..."
    $release = (Get-ItemProperty "HKLM:\SOFTWARE\Microsoft\NET Framework Setup\NDP\v4\Full").Release
    if (!$release -or $release -lt 528040) {
        Write-Status "Installing .NET Framework $dotnetVersion..."
        $url = "https://go.microsoft.com/fwlink/?LinkId=2085155"
        $installer = "$env:TEMP\ndp48-web.exe"
        Invoke-WebRequest -Uri $url -OutFile $installer
        Start-Process -FilePath $installer -ArgumentList "/q /norestart" -Wait
        Remove-Item $installer
    }
}

function Install-VCRedist {
    Write-Status "Checking Visual C++ Redistributable..."
    $installed = Get-WmiObject -Class Win32_Product | 
                 Where-Object { $_.Name -like "*Microsoft Visual C++ 2015-2022*" }
    if (!$installed) {
        Write-Status "Installing Visual C++ Redistributable..."
        $url = "https://aka.ms/vs/17/release/vc_redist.x64.exe"
        $installer = "$env:TEMP\vc_redist.x64.exe"
        Invoke-WebRequest -Uri $url -OutFile $installer
        Start-Process -FilePath $installer -ArgumentList "/install /quiet /norestart" -Wait
        Remove-Item $installer
    }
}

function Install-OpenSSL {
    Write-Status "Checking OpenSSL..."
    if (!(Test-Path "C:\Program Files\OpenSSL-Win64")) {
        Write-Status "Installing OpenSSL..."
        $url = "https://slproweb.com/download/Win64OpenSSL-$openSSLVersion.exe"
        $installer = "$env:TEMP\Win64OpenSSL.exe"
        Invoke-WebRequest -Uri $url -OutFile $installer
        Start-Process -FilePath $installer -ArgumentList "/SILENT /VERYSILENT /SP- /SUPPRESSMSGBOXES" -Wait
        Remove-Item $installer
    }
}

function Install-SQLite {
    Write-Status "Checking SQLite..."
    if (!(Test-Path "C:\Program Files\SQLite")) {
        Write-Status "Installing SQLite..."
        $url = "https://www.sqlite.org/2023/sqlite-dll-win64-x64-$sqliteVersion.zip"
        $zip = "$env:TEMP\sqlite.zip"
        Invoke-WebRequest -Uri $url -OutFile $zip
        Expand-Archive -Path $zip -DestinationPath "C:\Program Files\SQLite" -Force
        Remove-Item $zip

        # Add to PATH
        $path = [Environment]::GetEnvironmentVariable("Path", "Machine")
        if (!$path.Contains("SQLite")) {
            [Environment]::SetEnvironmentVariable(
                "Path", 
                "$path;C:\Program Files\SQLite", 
                "Machine"
            )
        }
    }
}

function Install-Application {
    Write-Status "Installing Smart Assistant..."
    $installerPath = Join-Path $PSScriptRoot "SmartAssistant-Release.msi"
    if (Test-Path $installerPath) {
        Start-Process -FilePath "msiexec.exe" `
                     -ArgumentList "/i `"$installerPath`" /qn" `
                     -Wait
    } else {
        throw "Installer not found: $installerPath"
    }
}

function Configure-Firewall {
    Write-Status "Configuring firewall rules..."
    $ruleName = "SmartAssistant"
    $exePath = "C:\Program Files\SmartAssistant\SmartAssistant.exe"

    # Remove existing rules
    Remove-NetFirewallRule -Name $ruleName -ErrorAction SilentlyContinue

    # Add new rules
    New-NetFirewallRule -Name $ruleName `
                       -DisplayName "Smart Assistant" `
                       -Direction Inbound `
                       -Program $exePath `
                       -Action Allow
}

# Main installation process
try {
    if (!(Test-AdminPrivileges)) {
        throw "This script requires administrator privileges."
    }


    Write-Status "Starting Smart Assistant installation..."

    # Install prerequisites
    Install-DotNetFramework
    Install-VCRedist
    Install-OpenSSL
    Install-SQLite

    # Install application
    Install-Application

    # Configure firewall
    Configure-Firewall

    Write-Status "Installation completed successfully!"
} catch {
    Write-Error "Installation failed: $_"
    exit 1
}
