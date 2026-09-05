@echo off
setlocal enabledelayedexpansion

echo ==========================================
echo   VNET 3D - Build & Run Script
echo ==========================================
echo.

set PROJECT_DIR=C:\Users\Admin\Desktop\Tuncay\Game Projects\vnet
set BUILD_DIR=%PROJECT_DIR%\build

echo 📁 Project Directory: %PROJECT_DIR%
echo.

:: Check if build directory exists
if exist "%BUILD_DIR%" (
    echo 📦 Build directory exists - cleaning...
    rmdir /s /q "%BUILD_DIR%"
    echo ✅ Clean complete
) else (
    echo 📦 No build directory found - creating...
)

echo.
echo 📦 Creating build directory...
mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

echo.
echo 🔧 Running CMake configuration...
cmake .. -G "Visual Studio 18 2026" -A x64

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ❌ CMake configuration failed!
    pause
    exit /b 1
)

echo.
echo 🔨 Building Release configuration...
cmake --build . --config Release --parallel

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ❌ Build failed!
    pause
    exit /b 1
)

echo.
echo ✅ Build successful!

:: Check if executable exists
if exist "%BUILD_DIR%\bin\Release\vnet_demo.exe" (
    echo.
    echo 🚀 Running VNET 3D...
    echo.
    "%BUILD_DIR%\bin\Release\vnet_demo.exe"
) else if exist "%BUILD_DIR%\Release\vnet_demo.exe" (
    echo.
    echo 🚀 Running VNET 3D...
    echo.
    "%BUILD_DIR%\Release\vnet_demo.exe"
) else (
    echo.
    echo ❌ Could not find vnet_demo.exe
    echo.
    echo Looking in:
    echo   %BUILD_DIR%\bin\Release\vnet_demo.exe
    echo   %BUILD_DIR%\Release\vnet_demo.exe
    echo.
    pause
    exit /b 1
)

echo.
echo ✅ Game exited successfully!
pause