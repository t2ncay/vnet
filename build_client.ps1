# ============================================================
# VNET CLIENT - Build Script
# VEKTRAOS v9.5 CYBERWARFARE ENGINE
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
Write-Header "VNET CLIENT - BUILD SYSTEM v2.1"

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
# SOURCE FILE COLLECTION
# ============================================================

Write-Header "COLLECTING SOURCE FILES"

$clientSources = @(
    # Client specific
    "src/client/main.cpp",
    "src/client/game.cpp",
    "src/client/render.cpp",
    "src/client/player.cpp",
    "src/client/vnet_client.cpp",
    "src/client/desktop.cpp",
    "src/client/music_player.cpp",
    "src/client/wallpaper.cpp",
    # Shared
    "src/shared/vnet.cpp",
    "src/shared/vnet_sites.cpp",
    "src/shared/utils.cpp",
    # Lib
    "src/lib/vnet_lib.cpp"
)

# Verify source files exist
$missingFiles = @()
foreach ($file in $clientSources) {
    if (-not (Test-Path $file)) {
        $missingFiles += $file
    }
}

if ($missingFiles.Count -gt 0) {
    Write-Error "Missing source files:"
    foreach ($f in $missingFiles) {
        Write-Host "  - $f" -ForegroundColor Red
    }
    exit 1
}

Write-Success "Found $($clientSources.Count) source files"

# ============================================================
# BUILD CLIENT
# ============================================================

Write-Header "BUILDING VNET CLIENT"

# Create build directory
if (Test-Path "build_client") {
    Write-Info "Cleaning build directory..."
    Remove-Item -Recurse -Force "build_client"
}
New-Item -ItemType Directory -Path "build_client" -Force | Out-Null
Write-Success "Build directory created"

# ============================================================
# INCLUDE PATHS AND DEFINES
# ============================================================
# NOTE: These are kept as ARRAYS (not a joined string) so they can be
# splatted straight into the g++ call below with `@includeDirs`.
# Passing arrays through the call operator (&) means PowerShell never
# has to re-parse a giant quoted string, so paths with spaces
# (like "Game Projects" in $projectDir) just work, no manual escaping.

$projectDirAbs = (Get-Location).Path
$raylibInclude = "$projectDirAbs/vendor/raylib/include"
$raylibLib = "$projectDirAbs/vendor/raylib/lib"

Write-Info "Raylib include: $raylibInclude"
Write-Info "Raylib lib: $raylibLib"

$includeDirs = @(
    "-I$projectDirAbs",
    "-I$projectDirAbs/src/client",
    "-I$projectDirAbs/src/shared",
    "-I$projectDirAbs/src/lib",
    "-I$raylibInclude"
)

$defines = @("-D_CRT_SECURE_NO_WARNINGS", "-D_USE_MATH_DEFINES")

# Compile each source file with progress bar
Write-Host "`n"
$objectFiles = @()
$failedFiles = @()
$totalFiles = $clientSources.Count
$i = 0

foreach ($src in $clientSources) {
    $i++
    $objName = ($src -replace '[/\\]', '_') -replace '\.cpp$', '.o'
    $obj = "build_client\$objName"
    $filename = Split-Path $src -Leaf

    Write-ProgressBar -Activity "Compiling Client" -Current $i -Total $totalFiles -Status "$filename"

    $compileArgs = @("-std=c++17", "-O2", "-g") + $includeDirs + $defines + @("-c", $src, "-o", $obj)

    # FIXED: call g++ directly via the call operator with an argument
    # array, instead of building one giant string and running it
    # through Invoke-Expression. This is what actually lets 2>&1
    # merge and capture the compiler's real stderr text reliably.
    $output = & g++ @compileArgs 2>&1
    $exitCode = $LASTEXITCODE

    if ($exitCode -ne 0) {
        $failedFiles += $filename
        Write-Host "`n  ❌ $filename - FAILED" -ForegroundColor Red
        # FIXED: print every diagnostic line g++ produced, not just
        # the first line that happens to contain the substring "error:"
        # (that missed linker-style messages and multi-line diagnostics).
        $output | ForEach-Object { Write-Host "     $_" -ForegroundColor Red }
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

Write-Success "All $totalFiles files compiled successfully!"

# ============================================================
# LINKING CLIENT
# ============================================================

Write-Header "LINKING CLIENT"

Write-Info "Linking $($objectFiles.Count) object files..."

Write-Info "Linking..."
$linkArgs = @("-o", "vnet_client.exe") + $objectFiles + @(
    "-L$raylibLib", "-lraylib", "-lopengl32", "-lgdi32",
    "-lwinmm", "-lshell32", "-lwinpthread", "-lws2_32", "-lm"
)
$linkOutput = & g++ @linkArgs 2>&1
$linkExit = $LASTEXITCODE

if ($linkExit -ne 0) {
    Write-Warning "First link attempt failed, trying alternative..."
    $linkOutput | ForEach-Object { Write-Host "  $_" -ForegroundColor Yellow }

    $linkArgs2 = @("-o", "vnet_client.exe") + $objectFiles + @(
        "$raylibLib/libraylib.a", "-lopengl32", "-lgdi32",
        "-lwinmm", "-lshell32", "-lwinpthread", "-lws2_32", "-lm"
    )
    $linkOutput = & g++ @linkArgs2 2>&1
    $linkExit = $LASTEXITCODE
}

if ($linkExit -ne 0) {
    Write-Error "Linking failed!"
    $linkOutput | ForEach-Object { Write-Host "  $_" -ForegroundColor Red }
    exit 1
}

Write-Success "Linking successful!"

# ============================================================
# POST-BUILD
# ============================================================

Write-Header "POST-BUILD"

# Clean up object files
Write-Info "Cleaning up object files..."
Remove-Item -Path "build_client\*.o" -ErrorAction SilentlyContinue
Write-Success "Cleanup complete"

# Copy raylib.dll if it exists
if (Test-Path "vendor/raylib/lib/raylib.dll") {
    Copy-Item "vendor/raylib/lib/raylib.dll" -Destination "." -Force
    Write-Success "Copied raylib.dll"
}

# ============================================================
# SUMMARY
# ============================================================

Write-Header "BUILD SUMMARY"

if (Test-Path "vnet_client.exe") {
    $size = [math]::Round((Get-Item vnet_client.exe).Length / 1KB, 2)
    Write-Success "✅ Client executable: $projectDir\vnet_client.exe"
    Write-Info "📦 Output size: $size KB"
    Write-Info "🕐 Build time: $(Get-Date -Format 'HH:mm:ss')"
    Write-Success "🎉 Client build completed successfully!"
} else {
    Write-Error "❌ Build failed - executable not found!"
    exit 1
}

# ============================================================
# RUN CLIENT
# ============================================================

Write-Header "RUNNING VNET CLIENT"

Write-Host "┌─────────────────────────────────────────────────────────────────┐" -ForegroundColor Magenta
Write-Host "│  🚀 Press any key to launch the client, or close this window. │" -ForegroundColor Cyan
Write-Host "└─────────────────────────────────────────────────────────────────┘" -ForegroundColor Magenta
Write-Host ""

$key = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
if ($key) {
    Write-Host "🎮 Launching VNET Client..." -ForegroundColor Green
    Write-Host ""
    .\vnet_client.exe
}