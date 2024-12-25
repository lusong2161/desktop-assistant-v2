param(
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

function Test-Dependency {
    param(
        [string]$Name,
        [scriptblock]$Test,
        [string]$ErrorMessage
    )

    Write-Host -NoNewline "Checking $Name... "
    try {
        if (& $Test) {
            Write-Host "OK" -ForegroundColor Green
            return $true
        } else {
            Write-Host "Failed" -ForegroundColor Red
            if ($ErrorMessage) {
                Write-Host "  $ErrorMessage" -ForegroundColor Yellow
            }
            return $false
        }
    } catch {
        Write-Host "Error" -ForegroundColor Red
        if ($Verbose) {
            Write-Host "  $_" -ForegroundColor Red
        }
        return $false
    }
}

function Test-Installation {
    $allPassed = $true

    # Check .NET Framework
    $allPassed = $allPassed -and (Test-Dependency ".NET Framework 4.8" {
        $release = (Get-ItemProperty "HKLM:\SOFTWARE\Microsoft\NET Framework Setup\NDP\v4\Full").Release
        return $release -ge 528040
    } "Please install .NET Framework 4.8 or later")

    # Check Visual C++ Redistributable
    $allPassed = $allPassed -and (Test-Dependency "Visual C++ Redistributable" {
        Get-WmiObject -Class Win32_Product | 
        Where-Object { $_.Name -like "*Microsoft Visual C++ 2015-2022*" } | 
        Select-Object -First 1
    } "Please install Visual C++ Redistributable 2015-2022")

    # Check OpenSSL
    $allPassed = $allPassed -and (Test-Dependency "OpenSSL" {
        Test-Path "C:\Program Files\OpenSSL-Win64\bin\libssl-1_1-x64.dll"
    } "Please install OpenSSL 1.1 or later")

    # Check SQLite
    $allPassed = $allPassed -and (Test-Dependency "SQLite" {
        Test-Path "C:\Program Files\SQLite\sqlite3.dll"
    } "Please install SQLite")

    # Check application installation
    $allPassed = $allPassed -and (Test-Dependency "Smart Assistant" {
        Test-Path "C:\Program Files\SmartAssistant\SmartAssistant.exe"
    } "Application not found. Please run setup.ps1")

    # Check firewall rules
    $allPassed = $allPassed -and (Test-Dependency "Firewall Rules" {
        Get-NetFirewallRule -Name "SmartAssistant" -ErrorAction SilentlyContinue
    } "Firewall rules not configured. Please run setup.ps1")

    # Final result
    if ($allPassed) {
        Write-Host "`nAll checks passed!" -ForegroundColor Green
        return 0
    } else {
        Write-Host "`nSome checks failed. Please fix the issues and try again." -ForegroundColor Red
        return 1
    }
}

exit (Test-Installation)
