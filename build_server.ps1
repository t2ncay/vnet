# ============================================================
# VNET SERVER - Build Script (Headless)
# VEKTRAOS v9.5 CYBERWARFARE ENGINE
# ============================================================

# Color functions
function Write-Success { Write-Host "✅ $($args[0])" -ForegroundColor Green }
function Write-Error { Write-Host "❌ $($args[0])" -ForegroundColor Red }
function Write-Warning { Write-Host "⚠️ $($args[0])" -ForegroundColor Yellow }
function Write-Info { Write-Host "ℹ️ $($args[0])" -ForegroundColor Cyan }
function Write-Header { Write-Host "`n╔═══════════════════════════════════════════════════════╗" -ForegroundColor Magenta; Write-Host "║ $($args[0])" -ForegroundColor Magenta; Write-Host "╚═══════════════════════════════════════════════════════╝`n" -ForegroundColor Magenta }

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

Clear-Host
Write-Header "VNET SERVER - BUILD SYSTEM v2.1 (HEADLESS)"

$projectDir = "C:\Users\Admin\Desktop\Tuncay\Game Projects\vnet"
if (-not (Test-Path $projectDir)) {
    Write-Error "Project directory not found: $projectDir"
    exit 1
}

Set-Location $projectDir
Write-Info "Project directory: $projectDir"

# Check g++
$gpp = Get-Command g++ -ErrorAction SilentlyContinue
if (-not $gpp) {
    Write-Error "g++ not found! Please install MinGW."
    exit 1
}
Write-Success "Compiler: $($gpp.Source)"

# ============================================================
# SOURCE FILE COLLECTION
# ============================================================

Write-Header "COLLECTING SOURCE FILES"

# Server sources - EXCLUDING client-only files
$serverSources = @(
    # Server specific
    "src/server/main.cpp",
    "src/server/server_core.cpp",
    # Shared - ONLY the ones that don't depend on raylib
    "src/shared/vnet_sites.cpp",
    "src/shared/utils.cpp",
    # Lib
    "src/lib/vnet_lib.cpp"
)

# Verify source files exist
$missingFiles = @()
foreach ($file in $serverSources) {
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

Write-Success "Found $($serverSources.Count) source files"

# ============================================================
# BUILD SERVER
# ============================================================

Write-Header "BUILDING VNET SERVER (HEADLESS)"

# Create build directory
if (Test-Path "build_server") {
    Write-Info "Cleaning build directory..."
    Remove-Item -Recurse -Force "build_server"
}
New-Item -ItemType Directory -Path "build_server" -Force | Out-Null
Write-Success "Build directory created"

$projectDirAbs = (Get-Location).Path

# NOTE: kept as arrays (not joined into one string) so they can be
# splatted straight into the g++ call below with `@includeDirs`.
$includeDirs = @(
    "-I$projectDirAbs",
    "-I$projectDirAbs/src/server",
    "-I$projectDirAbs/src/shared",
    "-I$projectDirAbs/src/lib"
)

# Defines for server
$defines = @("-DHEADLESS_SERVER", "-D_CRT_SECURE_NO_WARNINGS", "-DNO_RAYLIB")

Write-Host "`n"
$objectFiles = @()
$failedFiles = @()
$totalFiles = $serverSources.Count
$i = 0

foreach ($src in $serverSources) {
    $i++
    $objName = ($src -replace '[/\\]', '_') -replace '\.cpp$', '.o'
    $obj = "build_server\$objName"
    $filename = Split-Path $src -Leaf

    Write-ProgressBar -Activity "Compiling Server" -Current $i -Total $totalFiles -Status "$filename"

    $compileArgs = @("-std=c++17", "-O2", "-g") + $includeDirs + $defines + @("-c", $src, "-o", $obj)

    Write-Host ""
    Write-Host "  Compiling $filename..." -ForegroundColor Gray

    # FIXED: call g++ directly via the call operator with an argument
    # array instead of Invoke-Expression on a command string. Native
    # stderr wasn't reliably surfacing through the old string-based
    # invocation, which is why failures showed no diagnostic text.
    $output = & g++ @compileArgs 2>&1
    $exitCode = $LASTEXITCODE

    if ($exitCode -ne 0) {
        $failedFiles += $filename
        Write-Host "  ❌ $filename - FAILED" -ForegroundColor Red
        $output | ForEach-Object { Write-Host "    $_" -ForegroundColor Red }
    } else {
        $objectFiles += $obj
        Write-Host "  ✅ $filename - OK" -ForegroundColor Green
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
# LINKING SERVER
# ============================================================

Write-Header "LINKING SERVER (HEADLESS)"

Write-Info "Linking $($objectFiles.Count) object files..."

Write-Info "Linking..."
Write-Host ""
$linkArgs = @("-o", "vnet_server.exe") + $objectFiles + @("-lws2_32", "-lwinpthread", "-lm")
Write-Host "  g++ $($linkArgs -join ' ')" -ForegroundColor Gray
$linkOutput = & g++ @linkArgs 2>&1
$linkExit = $LASTEXITCODE

if ($linkExit -ne 0) {
    Write-Warning "First link attempt failed, trying alternative..."
    $linkOutput | ForEach-Object { Write-Host "  $_" -ForegroundColor Yellow }

    $linkArgs2 = @("-o", "vnet_server.exe") + $objectFiles + @("-static", "-lws2_32", "-lwinpthread", "-lm")
    Write-Host "  g++ $($linkArgs2 -join ' ')" -ForegroundColor Gray
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

Write-Info "Cleaning up object files..."
Remove-Item -Path "build_server\*.o" -ErrorAction SilentlyContinue
Write-Success "Cleanup complete"

# ============================================================
# SUMMARY
# ============================================================

Write-Header "BUILD SUMMARY"

if (Test-Path "vnet_server.exe") {
    $size = [math]::Round((Get-Item vnet_server.exe).Length / 1KB, 2)
    Write-Success "✅ Server executable: $projectDir\vnet_server.exe"
    Write-Info "📦 Output size: $size KB"
    Write-Info "🕐 Build time: $(Get-Date -Format 'HH:mm:ss')"
    Write-Success "🎉 Server build completed successfully!"
} else {
    Write-Error "❌ Build failed - executable not found!"
    exit 1
}

# ============================================================
# RUN SERVER
# ============================================================

Write-Header "RUNNING VNET SERVER"

Write-Host "┌─────────────────────────────────────────────────────────────────┐" -ForegroundColor Magenta
Write-Host "│  🚀 Press any key to launch the server, or close this window. │" -ForegroundColor Cyan
Write-Host "│  ⚡ Server runs headless on port 8000                         │" -ForegroundColor Yellow
Write-Host "│  ⚡ Press Ctrl+C in the terminal to stop                      │" -ForegroundColor Yellow
Write-Host "└─────────────────────────────────────────────────────────────────┘" -ForegroundColor Magenta
Write-Host ""

$key = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
if ($key) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "  VNET SERVER v9.5 - CYBERWARFARE ENGINE" -ForegroundColor Magenta
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "  Listening on port 8000" -ForegroundColor Green
    Write-Host "  Press Ctrl+C to stop" -ForegroundColor Yellow
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
    .\vnet_server.exe
}