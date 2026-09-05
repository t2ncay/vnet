# ============================================================
# VNET - Build All Script (Client + Server)
# VEKTRAOS v9.5 CYBERWARFARE ENGINE
# ============================================================

# Color functions
function Write-Success { Write-Host "✅ $($args[0])" -ForegroundColor Green }
function Write-Error { Write-Host "❌ $($args[0])" -ForegroundColor Red }
function Write-Warning { Write-Host "⚠️ $($args[0])" -ForegroundColor Yellow }
function Write-Info { Write-Host "ℹ️ $($args[0])" -ForegroundColor Cyan }
function Write-Header { Write-Host "`n╔═══════════════════════════════════════════════════════╗" -ForegroundColor Magenta; Write-Host "║ $($args[0])" -ForegroundColor Magenta; Write-Host "╚═══════════════════════════════════════════════════════╝`n" -ForegroundColor Magenta }

# ============================================================
# HEADER
# ============================================================

Clear-Host
Write-Header "VNET - BUILD ALL v2.0 (CLIENT + SERVER)"

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
    exit 1
}
Write-Success "Compiler: $($gpp.Source)"

# ============================================================
# SOURCE FILE COLLECTION
# ============================================================

Write-Header "COLLECTING SOURCE FILES"

$clientSources = @(
    "src/client/main.cpp",
    "src/client/game.cpp",
    "src/client/render.cpp",
    "src/client/player.cpp",
    "src/client/vnet_client.cpp",
    "src/shared/vnet.cpp",
    "src/shared/vnet_sites.cpp",
    "src/shared/utils.cpp",
    "src/lib/vnet_lib.cpp"
)

$serverSources = @(
    "src/server/main.cpp",
    "src/server/server_core.cpp",
    "src/shared/vnet.cpp",
    "src/shared/vnet_sites.cpp",
    "src/shared/utils.cpp",
    "src/lib/vnet_lib.cpp"
)

# Verify source files exist
$missingFiles = @()
foreach ($file in $clientSources + $serverSources) {
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

Write-Success "Client source: $($clientSources.Count) files"
Write-Success "Server source: $($serverSources.Count) files"

# ============================================================
# BUILD CLIENT
# ============================================================

Write-Header "BUILDING VNET CLIENT"

if (Test-Path "build_client") {
    Remove-Item -Recurse -Force "build_client"
}
New-Item -ItemType Directory -Path "build_client" -Force | Out-Null

$clientObjs = @()
$failed = @()
$total = $clientSources.Count
$i = 0

foreach ($src in $clientSources) {
    $i++
    $objName = ($src -replace '[/\\]', '_') -replace '\.cpp$', '.o'
    $obj = "build_client\$objName"
    $filename = Split-Path $src -Leaf
    
    Write-ProgressBar -Activity "Client" -Current $i -Total $total -Status "$filename"
    
    $includes = "-I. -I./src/client -I./src/shared -I./src/lib -I./vendor/raylib/include"
    $defines = "-D_WIN32 -DWIN32_LEAN_AND_MEAN -DNOGDI -DNOUSER -D_CRT_SECURE_NO_WARNINGS"
    
    g++ -std=c++17 -O2 -g $includes $defines -c $src -o $obj
    
    if ($LASTEXITCODE -ne 0) {
        $failed += $filename
        Write-Host "`n  ❌ $filename - FAILED" -ForegroundColor Red
    } else {
        $clientObjs += $obj
    }
}

Write-Host "`n"

if ($failed.Count -gt 0) {
    Write-Error "Client compilation failed!"
    exit 1
}

Write-Success "Client compiled successfully!"

# Link client
Write-Info "Linking client..."
$objList = $clientObjs -join " "
g++ -o vnet_client.exe $objList -L./vendor/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -lshell32 -lwinpthread -lws2_32 -lm

if ($LASTEXITCODE -ne 0) {
    Write-Error "Client linking failed!"
    exit 1
}

Write-Success "Client linked successfully!"

# ============================================================
# BUILD SERVER
# ============================================================

Write-Header "BUILDING VNET SERVER (HEADLESS)"

if (Test-Path "build_server") {
    Remove-Item -Recurse -Force "build_server"
}
New-Item -ItemType Directory -Path "build_server" -Force | Out-Null

$serverObjs = @()
$failed = @()
$total = $serverSources.Count
$i = 0

foreach ($src in $serverSources) {
    $i++
    $objName = ($src -replace '[/\\]', '_') -replace '\.cpp$', '.o'
    $obj = "build_server\$objName"
    $filename = Split-Path $src -Leaf
    
    Write-ProgressBar -Activity "Server" -Current $i -Total $total -Status "$filename"
    
    $includes = "-I. -I./src/server -I./src/shared -I./src/lib"
    $defines = "-D_WIN32 -DWIN32_LEAN_AND_MEAN -DHEADLESS_SERVER -D_CRT_SECURE_NO_WARNINGS"
    
    g++ -std=c++17 -O2 -g $includes $defines -c $src -o $obj
    
    if ($LASTEXITCODE -ne 0) {
        $failed += $filename
        Write-Host "`n  ❌ $filename - FAILED" -ForegroundColor Red
    } else {
        $serverObjs += $obj
    }
}

Write-Host "`n"

if ($failed.Count -gt 0) {
    Write-Error "Server compilation failed!"
    exit 1
}

Write-Success "Server compiled successfully!"

# Link server
Write-Info "Linking server..."
$objList = $serverObjs -join " "
g++ -o vnet_server.exe $objList -lws2_32 -lwinpthread -lm

if ($LASTEXITCODE -ne 0) {
    Write-Error "Server linking failed!"
    exit 1
}

Write-Success "Server linked successfully!"

# ============================================================
# POST-BUILD
# ============================================================

Write-Header "POST-BUILD"

# Clean up
Remove-Item -Path "build_client\*.o" -ErrorAction SilentlyContinue
Remove-Item -Path "build_server\*.o" -ErrorAction SilentlyContinue

# Copy raylib.dll
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
    Write-Success "✅ Client: vnet_client.exe ($size KB)"
} else {
    Write-Error "❌ Client executable not found!"
}

if (Test-Path "vnet_server.exe") {
    $size = [math]::Round((Get-Item vnet_server.exe).Length / 1KB, 2)
    Write-Success "✅ Server: vnet_server.exe ($size KB)"
} else {
    Write-Error "❌ Server executable not found!"
}

Write-Info "🕐 Build time: $(Get-Date -Format 'HH:mm:ss')"

# ============================================================
# RUN OPTION
# ============================================================

Write-Host "`n┌─────────────────────────────────────────────────────────────────┐" -ForegroundColor Magenta
Write-Host "│  🚀 Select an option:                                          │" -ForegroundColor Cyan
Write-Host "│  1. Run Client                                                │" -ForegroundColor Green
Write-Host "│  2. Run Server (headless)                                     │" -ForegroundColor Green
Write-Host "│  3. Run Both (server then client)                             │" -ForegroundColor Green
Write-Host "│  4. Exit                                                     │" -ForegroundColor Gray
Write-Host "└─────────────────────────────────────────────────────────────────┘" -ForegroundColor Magenta
Write-Host ""

$choice = Read-Host "Enter choice (1-4)"

switch ($choice) {
    "1" {
        Write-Host "🎮 Launching Client..." -ForegroundColor Green
        .\vnet_client.exe
    }
    "2" {
        Write-Host "🎮 Launching Server..." -ForegroundColor Green
        Write-Host "========================================" -ForegroundColor Cyan
        Write-Host "  VNET SERVER v9.5 - CYBERWARFARE ENGINE" -ForegroundColor Magenta
        Write-Host "========================================" -ForegroundColor Cyan
        Write-Host "  Listening on port 8000" -ForegroundColor Green
        Write-Host "  Press Ctrl+C to stop" -ForegroundColor Yellow
        Write-Host "========================================" -ForegroundColor Cyan
        .\vnet_server.exe
    }
    "3" {
        Write-Host "🎮 Launching Server first..." -ForegroundColor Green
        Start-Process -FilePath ".\vnet_server.exe" -WindowStyle Minimized
        Start-Sleep -Seconds 2
        Write-Host "🎮 Launching Client..." -ForegroundColor Green
        .\vnet_client.exe
    }
    default {
        Write-Host "Exiting..." -ForegroundColor Gray
    }
}