# ============================================================
# VNET CLIENT - Build Script v3.0 (Recursive Source Discovery)
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
Write-Header "VNET CLIENT - BUILD SYSTEM v3.0 (RECURSIVE SOURCE DISCOVERY)"

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
# RECURSIVE SOURCE FILE DISCOVERY - FIXED DUPLICATES
# ============================================================

Write-Header "RECURSIVE SOURCE DISCOVERY"

# Find ALL .cpp files recursively in src/
Write-Info "Scanning src/ directory recursively..."

# Get all .cpp files, excluding temporary/build files
$allCppFiles = Get-ChildItem -Path "src" -Recurse -Filter "*.cpp" | 
               Where-Object { $_.FullName -notmatch "build" -and $_.FullName -notmatch "temp" } |
               ForEach-Object { $_.FullName }

# Filter files - keep only those we want to compile
# Exclude: server files (they're separate)
$excludePatterns = @(
    "src\\server\\",
    "src\\tools\\",
    "src\\test\\"
)

$clientSources = @()
foreach ($file in $allCppFiles) {
    $shouldExclude = $false
    foreach ($pattern in $excludePatterns) {
        if ($file -match $pattern) {
            $shouldExclude = $true
            break
        }
    }
    if (-not $shouldExclude) {
        $clientSources += $file
    }
}

# ============================================================
# CRITICAL FIX: Remove duplicates!
# ============================================================
$clientSources = $clientSources | Select-Object -Unique

# Convert to relative paths for cleaner display
$relativeSources = @()
foreach ($file in $clientSources) {
    $relative = $file.Replace("$projectDir\", "")
    $relativeSources += $relative
}

# Display discovered files
Write-Success "Found $($clientSources.Count) source files:"
Write-Host ""
Write-Host "  ┌─────────────────────────────────────────────────────────────┐" -ForegroundColor Cyan
foreach ($src in $relativeSources | Sort-Object) {
    Write-Host "  │  $src" -ForegroundColor Gray
}
Write-Host "  └─────────────────────────────────────────────────────────────┘" -ForegroundColor Cyan
Write-Host ""

# Show breakdown by directory
$dirGroups = $relativeSources | ForEach-Object { 
    $dir = Split-Path $_ -Parent
    if ($dir -eq "") { "root" } else { $dir }
} | Group-Object | Sort-Object Count -Descending

Write-Host "  📁 Source file breakdown:"
foreach ($group in $dirGroups) {
    Write-Host "    ├─ $($group.Name) : $($group.Count) files" -ForegroundColor Green
}
Write-Host ""

# ============================================================
# VERIFY ESSENTIAL FILES - FIXED PATHS
# ============================================================

Write-Info "Verifying essential source files..."

$essentialFiles = @(
    "src\client\main.cpp",
    "src\client\desktop\desktop.cpp",
    "src\client\desktop\desktop.h",
    "src\shared\vnet.cpp",
    "src\shared\vex_parser.cpp",
    "src\shared\vnet_sites.cpp"
)

$missingEssential = $false
foreach ($file in $essentialFiles) {
    if (-not (Test-Path $file)) {
        Write-Error "Essential file missing: $file"
        $missingEssential = $true
    } else {
        Write-Success "Found: $file"
    }
}

if ($missingEssential) {
    Write-Error "Essential source files missing!"
    Write-Info "Make sure you've moved desktop.cpp/h to src/client/desktop/"
    exit 1
}

# ============================================================
# BUILD CLIENT
# ============================================================

Write-Header "BUILDING VNET CLIENT"

# Create build directory
if (Test-Path "build_client") {
    Write-Info "Cleaning build directory..."
    Remove-Item -Recurse -Force "build_client" -ErrorAction SilentlyContinue
}
New-Item -ItemType Directory -Path "build_client" -Force | Out-Null
Write-Success "Build directory created"

# ============================================================
# INCLUDE PATHS AND DEFINES
# ============================================================

$projectDirAbs = (Get-Location).Path
$raylibInclude = "$projectDirAbs/vendor/raylib/include"
$raylibLib = "$projectDirAbs/vendor/raylib/lib"

Write-Info "Raylib include: $raylibInclude"
Write-Info "Raylib lib: $raylibLib"

$includeDirs = @(
    "-I$projectDirAbs",
    "-I$projectDirAbs/src/client",
    "-I$projectDirAbs/src/client/desktop",
    "-I$projectDirAbs/src/client/desktop/apps",
    "-I$projectDirAbs/src/client/desktop/apps/vdec",
    "-I$projectDirAbs/src/client/desktop/settings",
    "-I$projectDirAbs/src/shared",
    "-I$projectDirAbs/src/lib",
    "-I$raylibInclude"
)

$defines = @(
    "-D_CRT_SECURE_NO_WARNINGS",
    "-D_USE_MATH_DEFINES",
    "-D_WIN32_WINNT=0x0600"
)

# ============================================================
# COMPILE ALL SOURCE FILES
# ============================================================

Write-Host "`n"
Write-Host "  ╔═══════════════════════════════════════════════════════════════╗" -ForegroundColor Yellow
Write-Host "  ║  🔨 COMPILING $($clientSources.Count) SOURCE FILES  ║" -ForegroundColor Yellow
Write-Host "  ╚═══════════════════════════════════════════════════════════════╝" -ForegroundColor Yellow
Write-Host ""

$objectFiles = @()
$failedFiles = @()
$totalFiles = $clientSources.Count
$i = 0

foreach ($src in $clientSources) {
    $i++
    $relativePath = $src.Replace("$projectDir\", "")
    $safeName = $relativePath -replace '[/\\:\. ]', '_'
    $obj = "build_client\$safeName.o"
    $filename = Split-Path $relativePath -Leaf
    $directory = Split-Path $relativePath -Parent

    Write-ProgressBar -Activity "Compiling Client" -Current $i -Total $totalFiles -Status "$filename ($directory)"

    $compileArgs = @(
        "-std=c++17",
        "-O3",
        "-g",
        "-Wall",
        "-Wextra"
    ) + $includeDirs + $defines + @(
        "-c",
        $src,
        "-o",
        $obj
    )

    $output = & g++ @compileArgs 2>&1
    $exitCode = $LASTEXITCODE

    if ($exitCode -ne 0) {
        $failedFiles += $filename
        Write-Host "`n  ❌ $filename - FAILED" -ForegroundColor Red
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

# Try standard linking
$linkArgs = @(
    "-o", "vnet_client.exe"
) + $objectFiles + @(
    "-L$raylibLib",
    "-lraylib",
    "-lopengl32",
    "-lgdi32",
    "-lwinmm",
    "-lshell32",
    "-lwinpthread",
    "-lws2_32",
    "-lm"
)

Write-Info "Linking..."
$linkOutput = & g++ @linkArgs 2>&1
$linkExit = $LASTEXITCODE

# If standard linking fails, try direct library path
if ($linkExit -ne 0) {
    Write-Warning "First link attempt failed, trying alternative library linking..."
    $linkOutput | ForEach-Object { Write-Host "  $_" -ForegroundColor Yellow }

    $linkArgs2 = @(
        "-o", "vnet_client.exe"
    ) + $objectFiles + @(
        "$raylibLib/libraylib.a",
        "-lopengl32",
        "-lgdi32",
        "-lwinmm",
        "-lshell32",
        "-lwinpthread",
        "-lws2_32",
        "-lm"
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
    Write-Info "📁 Source files compiled: $($clientSources.Count)"
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