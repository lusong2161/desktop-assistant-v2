param(
    [switch]$Release,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

# Configuration
$configuration = if ($Release) { "Release" } else { "Debug" }
$platform = "x64"
$msbuildPath = "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
$cmakePath = "cmake"
$vsDevCmd = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

# Clean build directories if requested
if ($Clean) {
    Write-Host "Cleaning build directories..."
    Remove-Item -Path "build" -Recurse -ErrorAction SilentlyContinue
    Remove-Item -Path "SmartAssistant.UI\bin" -Recurse -ErrorAction SilentlyContinue
    Remove-Item -Path "SmartAssistant.UI\obj" -Recurse -ErrorAction SilentlyContinue
}

# Create build directory for C++ project
New-Item -ItemType Directory -Path "build" -Force | Out-Null
Push-Location "build"

try {
    # Configure and build C++ project
    Write-Host "Configuring C++ project..."
    & $cmakePath -G "Visual Studio 17 2022" -A $platform ..
    if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed" }

    Write-Host "Building C++ project..."
    & $cmakePath --build . --config $configuration
    if ($LASTEXITCODE -ne 0) { throw "C++ build failed" }

    # Run C++ tests
    Write-Host "Running C++ tests..."
    & ctest -C $configuration --output-on-failure
    if ($LASTEXITCODE -ne 0) { throw "C++ tests failed" }
}
finally {
    Pop-Location
}

# Build C# project
Write-Host "Building C# project..."
& $msbuildPath "SmartAssistant.sln" /p:Configuration=$configuration /p:Platform=$platform
if ($LASTEXITCODE -ne 0) { throw "C# build failed" }

# Copy dependencies
Write-Host "Copying dependencies..."
$binPath = "SmartAssistant.UI\bin\$configuration"
New-Item -ItemType Directory -Path "$binPath\lib" -Force | Out-Null
Copy-Item "build\$configuration\SmartAssistant.Core.dll" "$binPath\lib"
Copy-Item "build\$configuration\*.dll" "$binPath\lib"

Write-Host "Build completed successfully!"
