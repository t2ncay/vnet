# ============================================================
# VNET CLIENT - Fast Parallel Build Script v5.1
# VEKTRAOS v9.5 CYBERWARFARE ENGINE
# ============================================================

# ---- COLOR PALETTE ----
$COLOR_BLACK   = "Black"
$COLOR_PANEL   = "DarkGray"
$COLOR_BLOOD   = "Red"
$COLOR_CYAN    = "Cyan"
$COLOR_AMBER   = "Yellow"
$COLOR_TOXIC   = "Green"
$COLOR_GHOST   = "Gray"
$COLOR_PURPLE  = "Magenta"
$COLOR_WHITE   = "White"

# ---- EMOJI FALLBACK (using surrogate pairs or text replacements) ----
$CHECK  = "$([char]0x2714)"  # ✓
$CROSS  = "$([char]0x2718)"  # ✘
$WARN   = "$([char]0x26A0)"  # ⚠
$INFO   = "$([char]0x2139)"  # ℹ
$FOLDER = "📁"               # Use direct Unicode (works in PS7)
$GEAR   = "⚙"
$CLOCK  = "⏰"
$BOLT   = "⚡"
$SCALE  = "⚖️"
$ROCKET = "🚀"
$GAME   = "🎮"

# ---- COLOR FUNCTIONS ----
function Write-Success { Write-Host "  $CHECK $($args[0])" -ForegroundColor $COLOR_TOXIC }
function Write-Error { Write-Host "  $CROSS $($args[0])" -ForegroundColor $COLOR_BLOOD }
function Write-Warning { Write-Host "  $WARN $($args[0])" -ForegroundColor $COLOR_AMBER }
function Write-Info { Write-Host "  $INFO $($args[0])" -ForegroundColor $COLOR_CYAN }
function Write-Section { Write-Host "`n  ═══ $($args[0]) ═══" -ForegroundColor $COLOR_PURPLE }

# ---- HEADER WITH ASCII ART ----
function Write-Header {
    Clear-Host
    Write-Host ""
    Write-Host "  ╔═══════════════════════════════════════════════════════════════╗" -ForegroundColor $COLOR_PURPLE
    Write-Host "  ║                                                               ║" -ForegroundColor $COLOR_PURPLE
    Write-Host "  ║   ██╗   ██╗███████╗██╗  ██╗████████╗██████╗  █████╗          ║" -ForegroundColor $COLOR_CYAN
    Write-Host "  ║   ██║   ██║██╔════╝██║ ██╔╝╚══██╔══╝██╔══██╗██╔══██╗         ║" -ForegroundColor $COLOR_CYAN
    Write-Host "  ║   ██║   ██║█████╗  █████╔╝    ██║   ██████╔╝███████║         ║" -ForegroundColor $COLOR_CYAN
    Write-Host "  ║   ╚██╗ ██╔╝██╔══╝  ██╔═██╗    ██║   ██╔══██╗██╔══██║         ║" -ForegroundColor $COLOR_CYAN
    Write-Host "  ║    ╚████╔╝ ███████╗██║  ██╗   ██║   ██║  ██║██║  ██║         ║" -ForegroundColor $COLOR_CYAN
    Write-Host "  ║     ╚═══╝  ╚══════╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝         ║" -ForegroundColor $COLOR_CYAN
    Write-Host "  ║                                                               ║" -ForegroundColor $COLOR_PURPLE
    Write-Host "  ║        ██████╗ ██████╗ ██╗     ██████╗                         ║" -ForegroundColor $COLOR_TOXIC
    Write-Host "  ║       ██╔════╝██╔═══██╗██║     ╚════██╗                        ║" -ForegroundColor $COLOR_TOXIC
    Write-Host "  ║       ██║     ██║   ██║██║      █████╔╝                        ║" -ForegroundColor $COLOR_TOXIC
    Write-Host "  ║       ██║     ██║   ██║██║     ██╔═══╝                         ║" -ForegroundColor $COLOR_TOXIC
    Write-Host "  ║       ╚██████╗╚██████╔╝███████╗███████╗                        ║" -ForegroundColor $COLOR_TOXIC
    Write-Host "  ║        ╚═════╝ ╚═════╝ ╚══════╝╚══════╝                        ║" -ForegroundColor $COLOR_TOXIC
    Write-Host "  ║                                                               ║" -ForegroundColor $COLOR_PURPLE
    Write-Host "  ║         COLD SIGNAL // CYBER OPERATIONS v9.5                  ║" -ForegroundColor $COLOR_AMBER
    Write-Host "  ║         FAST PARALLEL BUILD SYSTEM v5.1                       ║" -ForegroundColor $COLOR_GHOST
    Write-Host "  ║                                                               ║" -ForegroundColor $COLOR_PURPLE
    Write-Host "  ╚═══════════════════════════════════════════════════════════════╝" -ForegroundColor $COLOR_PURPLE
    Write-Host ""
}

# ============================================================
# MAIN EXECUTION
# ============================================================

Write-Header

# ---- PROJECT SETUP ----
Write-Section "INITIALIZING BUILD ENVIRONMENT"

$projectDir = "C:\Users\Admin\Desktop\Tuncay\Game Projects\vnet"
if (-not (Test-Path $projectDir)) {
    Write-Error "Project directory not found: $projectDir"
    exit 1
}

Set-Location $projectDir
Write-Info "Project root: $projectDir"

# ---- DEPENDENCY CHECK ----
Write-Section "VERIFYING DEPENDENCIES"

$gpp = Get-Command g++ -ErrorAction SilentlyContinue
if (-not $gpp) {
    Write-Error "g++ not found! Please install MinGW."
    Write-Host "  Download: https://www.mingw-w64.org/" -ForegroundColor $COLOR_GHOST
    exit 1
}
Write-Success "Compiler: $($gpp.Source)"

if (-not (Test-Path "vendor/raylib/include/raylib.h")) {
    Write-Error "Raylib header not found!"
    exit 1
}
Write-Success "Raylib header: vendor/raylib/include/raylib.h"

if (Test-Path "vendor/raylib/lib/libraylib.a") {
    Write-Success "Raylib library: vendor/raylib/lib/libraylib.a"
} elseif (Test-Path "vendor/raylib/lib/raylib.lib") {
    Write-Success "Raylib library: vendor/raylib/lib/raylib.lib"
} else {
    Write-Warning "Raylib library not found, will use dynamic linking"
}

if (Test-Path "assets/fonts/JetBrainsMono-Bold.ttf") {
    Write-Success "Font: assets/fonts/JetBrainsMono-Bold.ttf"
} else {
    Write-Warning "JetBrains font not found, using default"
}

# ---- SOURCE DISCOVERY ----
Write-Section "SCANNING SOURCE FILES"

$allCppFiles = Get-ChildItem -Path "src" -Recurse -Filter "*.cpp" | 
               Where-Object { $_.FullName -notmatch "build" -and $_.FullName -notmatch "temp" } |
               ForEach-Object { $_.FullName }

$clientSources = @()
foreach ($file in $allCppFiles) {
    if ($file -notmatch "src\\server\\" -and 
        $file -notmatch "src\\tools\\" -and 
        $file -notmatch "src\\test\\") {
        $clientSources += $file
    }
}

Write-Success "Found $($clientSources.Count) source files"

# Show breakdown
$dirGroups = $clientSources | ForEach-Object { 
    $dir = Split-Path $_ -Parent
    $dir = $dir -replace ".*\\src\\", ""
    if ($dir -eq "") { "root" } else { $dir }
} | Group-Object | Sort-Object Count -Descending

Write-Host ""
Write-Host "  📁 Source breakdown:" -ForegroundColor $COLOR_CYAN
foreach ($group in $dirGroups) {
    $barLength = [math]::Min(30, $group.Count * 2)
    $bar = "█" * $barLength
    Write-Host "    $($group.Name.PadRight(30)) $bar $($group.Count) files" -ForegroundColor $COLOR_GHOST
}
Write-Host ""

# ---- BUILD DIRECTORY ----
if (Test-Path "build_client") {
    Write-Info "Cleaning previous build..."
    Remove-Item -Recurse -Force "build_client" -ErrorAction SilentlyContinue
}
New-Item -ItemType Directory -Path "build_client" -Force | Out-Null
Write-Success "Build directory: build_client/"

# ---- INCLUDE PATHS ----
$projectDirAbs = (Get-Location).Path
$raylibInclude = "$projectDirAbs/vendor/raylib/include"
$raylibLib = "$projectDirAbs/vendor/raylib/lib"

$includeDirs = @(
    "-I$projectDirAbs",
    "-I$projectDirAbs/src/client",
    "-I$projectDirAbs/src/client/desktop",
    "-I$projectDirAbs/src/client/desktop/apps",
    "-I$projectDirAbs/src/client/desktop/apps/vdec",
    "-I$projectDirAbs/src/client/desktop/settings",
    "-I$projectDirAbs/src/client/connection",
    "-I$projectDirAbs/src/client/raid",
    "-I$projectDirAbs/src/shared",
    "-I$projectDirAbs/src/lib",
    "-I$raylibInclude"
)

$defines = @(
    "-D_CRT_SECURE_NO_WARNINGS",
    "-D_USE_MATH_DEFINES",
    "-D_WIN32_WINNT=0x0600"
)

$compileBase = @(
    "-std=c++17",
    "-O3",
    "-g",
    "-Wall",
    "-Wextra"
) + $includeDirs + $defines

# ---- PARALLEL COMPILATION ----
Write-Section "PARALLEL COMPILATION"

$maxJobs = [Environment]::ProcessorCount
if ($maxJobs -gt 8) { $maxJobs = 8 }
Write-Info "Using $maxJobs parallel jobs ($([Environment]::ProcessorCount) cores available)"

$objectFiles = @()
$failedFiles = @()
$totalFiles = $clientSources.Count
$jobs = @()
$completed = 0
$startTime = Get-Date

Write-Host ""
Write-Host "  ╔═══════════════════════════════════════════════════════════════════╗" -ForegroundColor $COLOR_AMBER
Write-Host "  ║  🔨 COMPILING $totalFiles SOURCE FILES ($maxJobs parallel)  ║" -ForegroundColor $COLOR_AMBER
Write-Host "  ╚═══════════════════════════════════════════════════════════════════╝" -ForegroundColor $COLOR_AMBER
Write-Host ""

foreach ($src in $clientSources) {
    $relativePath = $src.Replace("$projectDirAbs\", "")
    $safeName = $relativePath -replace '[/\\:\. ]', '_'
    $obj = "build_client\$safeName.o"
    $filename = Split-Path $relativePath -Leaf
    
    $scriptBlock = {
        param($srcFile, $objFile, $compileArgs, $filename)
        $output = & g++ @compileArgs -c $srcFile -o $objFile 2>&1
        $exitCode = $LASTEXITCODE
        return @{
            success = ($exitCode -eq 0)
            filename = $filename
            output = $output
            objFile = $objFile
        }
    }
    
    $jobArgs = @($src, $obj, $compileBase, $filename)
    $jobs += Start-Job -ScriptBlock $scriptBlock -ArgumentList $jobArgs
}

while ($jobs.Count -gt 0) {
    $completedJobs = $jobs | Where-Object { $_.State -eq 'Completed' -or $_.State -eq 'Failed' }
    foreach ($job in $completedJobs) {
        $result = Receive-Job -Job $job
        $jobs = $jobs | Where-Object { $_ -ne $job }
        Remove-Job -Job $job
        
        if ($result.success) {
            $objectFiles += $result.objFile
            Write-Host "  $CHECK $($result.filename)" -ForegroundColor $COLOR_TOXIC
        } else {
            $failedFiles += $result.filename
            Write-Host "  $CROSS $($result.filename) - FAILED" -ForegroundColor $COLOR_BLOOD
            if ($result.output) {
                $result.output | ForEach-Object { Write-Host "     $_" -ForegroundColor $COLOR_GHOST }
            }
        }
        $completed++
        
        $percent = [math]::Round(($completed / $totalFiles) * 100, 1)
        $elapsed = [math]::Round(((Get-Date) - $startTime).TotalSeconds, 1)
        Write-Host -NoNewline "`r  Progress: $percent% ($completed/$totalFiles) | Elapsed: ${elapsed}s" -ForegroundColor $COLOR_CYAN
    }
    Start-Sleep -Milliseconds 100
}

Write-Host "`n"

if ($failedFiles.Count -gt 0) {
    Write-Error "Compilation failed on $($failedFiles.Count) file(s):"
    foreach ($f in $failedFiles) { Write-Host "  - $f" -ForegroundColor $COLOR_BLOOD }
    exit 1
}

$compileTime = [math]::Round(((Get-Date) - $startTime).TotalSeconds, 1)
Write-Success "All $totalFiles files compiled successfully in ${compileTime}s"

# ---- LINKING ----
Write-Section "LINKING EXECUTABLE"

Write-Info "Linking $($objectFiles.Count) object files..."

$linkStart = Get-Date

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

$linkOutput = & g++ @linkArgs 2>&1
$linkExit = $LASTEXITCODE

if ($linkExit -ne 0) {
    Write-Warning "First link attempt failed, trying direct library..."
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
    $linkOutput | ForEach-Object { Write-Host "  $_" -ForegroundColor $COLOR_BLOOD }
    exit 1
}

$linkTime = [math]::Round(((Get-Date) - $linkStart).TotalSeconds, 1)
Write-Success "Linking successful in ${linkTime}s"

# ---- POST-BUILD ----
Write-Section "POST-BUILD"

Remove-Item -Path "build_client\*.o" -ErrorAction SilentlyContinue
Write-Info "Cleaned object files"

if (Test-Path "vendor/raylib/lib/raylib.dll") {
    Copy-Item "vendor/raylib/lib/raylib.dll" -Destination "." -Force
    Write-Success "Copied raylib.dll"
}

# ---- SUMMARY ----
Write-Section "BUILD SUMMARY"

$totalTime = [math]::Round(((Get-Date) - $startTime).TotalSeconds, 1)

if (Test-Path "vnet_client.exe") {
    $size = [math]::Round((Get-Item vnet_client.exe).Length / 1KB, 2)
    
    Write-Host ""
    Write-Host "  ╔═══════════════════════════════════════════════════════════════════╗" -ForegroundColor $COLOR_TOXIC
    Write-Host "  ║                         BUILD SUCCESSFUL!                        ║" -ForegroundColor $COLOR_TOXIC
    Write-Host "  ╠═══════════════════════════════════════════════════════════════════╣" -ForegroundColor $COLOR_TOXIC
    Write-Host "  ║  $FOLDER Executable : vnet_client.exe                     ║" -ForegroundColor $COLOR_WHITE
    Write-Host "  ║  $GEAR Size        : $size KB                              ║" -ForegroundColor $COLOR_WHITE
    Write-Host "  ║  $INFO Files       : $totalFiles sources                   ║" -ForegroundColor $COLOR_WHITE
    Write-Host "  ║  $BOLT Parallel    : $maxJobs jobs                         ║" -ForegroundColor $COLOR_WHITE
    Write-Host "  ║  $CLOCK Time       : ${totalTime}s total                   ║" -ForegroundColor $COLOR_WHITE
    Write-Host "  ║  $SCALE Status     : $CHECK READY TO RUN         ║" -ForegroundColor $COLOR_TOXIC
    Write-Host "  ╚═══════════════════════════════════════════════════════════════════╝" -ForegroundColor $COLOR_TOXIC
    Write-Host ""
} else {
    Write-Error "Build failed - executable not found!"
    exit 1
}

# ---- RUN PROMPT ----
Write-Section "READY TO LAUNCH"

Write-Host ""
Write-Host "  ┌─────────────────────────────────────────────────────────────────────┐" -ForegroundColor $COLOR_PURPLE
Write-Host "  │  $ROCKET Press any key to launch the client, or close this window.  │" -ForegroundColor $COLOR_CYAN
Write-Host "  └─────────────────────────────────────────────────────────────────────┘" -ForegroundColor $COLOR_PURPLE
Write-Host ""

$key = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
if ($key) {
    Write-Host ""
    Write-Host "  $GAME Launching VNET Client..." -ForegroundColor $COLOR_TOXIC
    Write-Host ""
    .\vnet_client.exe
}