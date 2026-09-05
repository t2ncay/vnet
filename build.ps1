# ============================================================
# VNET 3D - Advanced Build Script
# ============================================================

# Color functions
function Write-Success { Write-Host "✅ $($args[0])" -ForegroundColor Green }
function Write-Error { Write-Host "❌ $($args[0])" -ForegroundColor Red }
function Write-Warning { Write-Host "⚠️ $($args[0])" -ForegroundColor Yellow }
function Write-Info { Write-Host "ℹ️ $($args[0])" -ForegroundColor Cyan }
function Write-Header { Write-Host "`n╔═══════════════════════════════════════════════════════╗" -ForegroundColor Magenta; Write-Host "║ $($args[0])" -ForegroundColor Magenta; Write-Host "╚═══════════════════════════════════════════════════════╝`n" -ForegroundColor Magenta }

# Progress bar function
function Write-ProgressBar {
    param(
        [string]$Activity,
        [int]$Current,
        [int]$Total,
        [string]$Status = ""
    )
    $percent = ($Current / $Total) * 100
    $barLength = 40
    $filled = [int](($percent / 100) * $barLength)
    $empty = $barLength - $filled
    $bar = "█" * $filled + "░" * $empty
    Write-Host -NoNewline "`r  $Activity [$bar] $([math]::Round($percent, 1))% $Status" -ForegroundColor Cyan
}

# ============================================================
# HEADER
# ============================================================

Clear-Host
Write-Header "VNET 3D - BUILD SYSTEM v2.0"

# ============================================================
# PROJECT SETUP
# ============================================================

$projectDir = "C:\Users\Admin\Desktop\Tuncay\Game Projects\vnet"
if (-not (Test-Path $projectDir)) {
    Write-Error "Project directory not found: $projectDir"
    exit 1
}

Set-Location $projectDir
Write-Info "Project directory: $projectDir"

# ============================================================
# CHECK DEPENDENCIES
# ============================================================

Write-Header "CHECKING DEPENDENCIES"

# Check g++
$gpp = Get-Command g++ -ErrorAction SilentlyContinue
if (-not $gpp) {
    Write-Error "g++ not found! Please install MinGW."
    Write-Warning "Download from: https://www.mingw-w64.org/"
    exit 1
}
Write-Success "Compiler: $($gpp.Source)"

# Check raylib
if (Test-Path "vendor/raylib/include/raylib.h") {
    Write-Success "Raylib header: vendor/raylib/include/raylib.h"
} else {
    Write-Error "Raylib header not found!"
    exit 1
}

if (Test-Path "vendor/raylib/lib/libraylib.a") {
    Write-Success "Raylib library: vendor/raylib/lib/libraylib.a"
} elseif (Test-Path "vendor/raylib/lib/raylib.lib") {
    Write-Success "Raylib library: vendor/raylib/lib/raylib.lib"
} else {
    Write-Error "Raylib library not found!"
    exit 1
}

# Check font
if (Test-Path "assets/VCR_OSD_MONO_1.001.ttf") {
    Write-Success "VCR font: assets/VCR_OSD_MONO_1.001.ttf"
} else {
    Write-Warning "VCR font not found at assets/VCR_OSD_MONO_1.001.ttf"
    Write-Info "Using default font"
}

# ============================================================
# BUILD
# ============================================================

Write-Header "BUILDING VNET 3D"

# Create build directory
if (Test-Path "build") {
    Write-Info "Cleaning build directory..."
    Remove-Item -Recurse -Force "build"
}
New-Item -ItemType Directory -Path "build" -Force | Out-Null
Write-Success "Build directory created"

# Get source files
$srcFiles = Get-ChildItem -Path "src" -Filter "*.cpp"
$srcCount = $srcFiles.Count
$objectFiles = @()
$failedFiles = @()

Write-Info "Found $srcCount source files"

# Compile each source file with progress bar
Write-Host "`n"
$i = 0
foreach ($src in $srcFiles) {
    $i++
    $obj = "build\$($src.BaseName).o"
    $filename = $src.Name
    
    Write-ProgressBar -Activity "Compiling" -Current $i -Total $srcCount -Status "$filename"
    
    g++ -std=c++17 -O2 -g `
        -I. -I./vendor/raylib/include `
        -D_WIN32 -DWIN32_LEAN_AND_MEAN -DNOGDI -DNOUSER `
        -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES `
        -c $src.FullName -o $obj
    
    if ($LASTEXITCODE -ne 0) {
        $failedFiles += $filename
        Write-Host "`n  ❌ $filename - FAILED" -ForegroundColor Red
    } else {
        $objectFiles += $obj
    }
}

Write-Host "`n"

if ($failedFiles.Count -gt 0) {
    Write-Error "Compilation failed on $($failedFiles.Count) file(s):"
    foreach ($f in $failedFiles) {
        Write-Host "  - $f" -ForegroundColor Red
    }
    exit 1
}

Write-Success "All $srcCount files compiled successfully!"

# ============================================================
# LINKING
# ============================================================

Write-Header "LINKING"

Write-Info "Linking $($objectFiles.Count) object files..."

# Try different link approaches
$linkSuccess = $false
$linkAttempts = @(
    "-lraylib -lopengl32 -lgdi32 -lwinmm -lshell32 -lwinpthread -lws2_32 -lm",
    "-static -lraylib -lopengl32 -lgdi32 -lwinmm -lshell32 -lwinpthread -lws2_32 -lm",
    "vendor/raylib/lib/libraylib.a -lopengl32 -lgdi32 -lwinmm -lshell32 -lwinpthread -lws2_32 -lm"
)

$attempt = 1
foreach ($linkFlags in $linkAttempts) {
    # FIXED: Use ${attempt} to delimit the variable name
    Write-Info "Link attempt ${attempt}: $linkFlags"
    
    $cmd = "g++ -o vnet_demo.exe $objectFiles -L./vendor/raylib/lib $linkFlags"
    Invoke-Expression $cmd 2>&1 | Out-Null
    
    if ($LASTEXITCODE -eq 0) {
        $linkSuccess = $true
        Write-Success "Linking successful with attempt ${attempt}"
        break
    }
    $attempt++
}

if (-not $linkSuccess) {
    Write-Error "All linking attempts failed!"
    Write-Warning "Trying direct link without object files..."
    
    # Try direct link
    g++ -o vnet_demo.exe $objectFiles -L./vendor/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -lshell32 -lwinpthread -lws2_32 -lm 2>&1
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Linking failed!"
        exit 1
    }
    Write-Success "Linking successful with fallback!"
}

# ============================================================
# POST-BUILD
# ============================================================

Write-Header "POST-BUILD"

# Clean up object files
Write-Info "Cleaning up object files..."
Remove-Item -Path "*.o" -ErrorAction SilentlyContinue
Write-Success "Cleanup complete"

# Copy raylib.dll if it exists
if (Test-Path "vendor/raylib/lib/raylib.dll") {
    Copy-Item "vendor/raylib/lib/raylib.dll" -Destination "." -Force
    Write-Success "Copied raylib.dll"
}

# Copy assets
if (Test-Path "assets") {
    if (-not (Test-Path "assets/VCR_OSD_MONO_1.001.ttf")) {
        Write-Warning "VCR font not found in assets folder"
    }
}

# ============================================================
# SUMMARY
# ============================================================

Write-Header "BUILD SUMMARY"

Write-Success "Executable: $projectDir\vnet_demo.exe"
Write-Info "Build time: $(Get-Date -Format 'HH:mm:ss')"
Write-Info "Output size: $([math]::Round((Get-Item vnet_demo.exe).Length / 1KB, 2)) KB"

# Check if executable exists
if (Test-Path "vnet_demo.exe") {
    Write-Success "Build completed successfully!"
} else {
    Write-Error "Build failed - executable not found!"
    exit 1
}

# ============================================================
# RUN
# ============================================================

Write-Header "RUNNING VNET 3D"

Write-Host "┌─────────────────────────────────────────────────────────────────┐" -ForegroundColor Magenta
Write-Host "│  🚀 Press any key to launch the game, or close this window.   │" -ForegroundColor Cyan
Write-Host "└─────────────────────────────────────────────────────────────────┘" -ForegroundColor Magenta
Write-Host ""

$key = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
if ($key) {
    Write-Host "🎮 Launching VNET 3D..." -ForegroundColor Green
    Write-Host ""
    .\vnet_demo.exe
}