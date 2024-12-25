# Smart Assistant Package

This package contains the Smart Assistant application for Windows, a floating desktop assistant built with C# and C++.

## Package Contents

- `SmartAssistant-{version}.msi`: Full installer with all dependencies
- `SmartAssistant-Portable-{version}.zip`: Portable version (no installation required)
- `setup.ps1`: Installation script
- `verify_installation.ps1`: Installation verification script
- `checksums.txt`: SHA256 checksums for all files
- `version_info.txt`: Detailed version and build information

## Installation Options

### 1. Using the MSI Installer (Recommended)

1. Double-click `SmartAssistant-{version}.msi`
2. Follow the installation wizard
3. Launch Smart Assistant from the Start Menu

### 2. Using the Setup Script

1. Open PowerShell as Administrator
2. Navigate to the package directory
3. Run: `.\setup.ps1`
4. Verify installation: `.\verify_installation.ps1`

### 3. Portable Version

1. Extract `SmartAssistant-Portable-{version}.zip`
2. Run `SmartAssistant.cmd`

## System Requirements

- Windows 10 or later (64-bit)
- .NET Framework 4.8 or later
- Visual C++ Redistributable 2015-2022
- 4GB RAM minimum
- 500MB free disk space

## Verification

To verify package integrity:
1. Open PowerShell
2. Run: `Get-FileHash <filename> -Algorithm SHA256`
3. Compare with checksums.txt

## Support

For support or bug reports, please contact support@smartassistant.com

## License

This software is licensed under the terms provided in the installer package.
