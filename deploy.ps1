# deploy.ps1 - Package the GTK application for Release

$ReleaseDir = "release"
$ExecutableName = "DeepSharkAttack.exe"
$MSYS2_BIN = "C:\msys64\mingw64\bin"
$MSYS2_USR = "C:\msys64\usr\bin"

# Ensure MSYS2 paths are in PATH for this session
foreach ($path in @($MSYS2_BIN, $MSYS2_USR)) {
    if (Test-Path $path) {
        if ($env:PATH -notlike "*$path*") {
            $env:PATH = "$path;$env:PATH"
        }
    }
}

Write-Host "Starting Deployment Process..." -ForegroundColor Cyan

# 1. Clean and Setup Release Directory
if (Test-Path $ReleaseDir) {
    Write-Host "Cleaning existing release directory..."
    Remove-Item -Recurse -Force $ReleaseDir
}
New-Item -ItemType Directory -Path $ReleaseDir | Out-Null
New-Item -ItemType Directory -Path "$ReleaseDir/bin" | Out-Null

# 2. Build the application in Release mode
Write-Host "Building application in Release mode..." -ForegroundColor Yellow

# Get GTK flags
$CFLAGS = & pkg-config --cflags gtk4
$LIBS = & pkg-config --libs gtk4

# We use the existing build.ps1 to build the main.c
# Note: We add -O3 for optimization and -mwindows to hide the console in release
$ExtraSrcs = "src/modele/poisson.c src/ui/screen_bassin.c src/ui/bassin_simulation.c src/ui/bassin_sidebar.c src/ui/bassin_xml.c src/ui/bassin_dialogs.c src/ui/bassin_menu.c src/ui/screen_stubs.c src/sound.c"
$ReleaseFlags = "-O3 -s -mwindows"

powershell -NoProfile -ExecutionPolicy Bypass -Command ".\build.ps1 -Target 'src/main.c' -OutFile '$ReleaseDir/$ExecutableName' -ExtraSrcs '$ExtraSrcs' -CFlags '$CFLAGS $ReleaseFlags' -Libs '$LIBS'"

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

# 3. Copy Assets
Write-Host "Copying assets..." -ForegroundColor Yellow
Copy-Item -Recurse "resources" "$ReleaseDir/resources"
Copy-Item -Recurse "data" "$ReleaseDir/data"
Copy-Item "ui_exemple.txt" "$ReleaseDir/ui_exemple.txt"
if (Test-Path "interface.txt") { Copy-Item "interface.txt" "$ReleaseDir/interface.txt" }

# 4. Copy DLL Dependencies
Write-Host "Gathering DLL dependencies (this may take a moment)..." -ForegroundColor Yellow

if (-not (Test-Path "$ReleaseDir/$ExecutableName")) {
    Write-Host "Error: Executable not found at $ReleaseDir/$ExecutableName" -ForegroundColor Red
    exit 1
}

$dllsToCopy = New-Object System.Collections.Generic.HashSet[string]
$processedFiles = New-Object System.Collections.Generic.HashSet[string]

function Get-Dependencies {
    param([string]$file)
    if ($processedFiles.Contains($file)) { return }
    $processedFiles.Add($file) | Out-Null

    $lddOutput = & ldd "$file"
    foreach ($line in $lddOutput) {
        if ($line -match "=>\s+(/mingw64/bin/\S+\.dll)") {
            $dep = $matches[1]
            $winDep = $dep -replace "^/mingw64", $MSYS2_BIN.Replace("\bin", "")
            $winDep = $winDep -replace "/", "\"
            
            if (Test-Path $winDep) {
                if ($dllsToCopy.Add($winDep)) {
                    Write-Host "Found dependency: $([System.IO.Path]::GetFileName($winDep))" -ForegroundColor Gray
                    Get-Dependencies $winDep
                }
            }
        }
    }
}

Get-Dependencies "$ReleaseDir/$ExecutableName"

# 4b. Handle GStreamer Plugins
Write-Host "Copying GStreamer plugins..." -ForegroundColor Yellow
$GstPluginDir = "$ReleaseDir/lib/gstreamer-1.0"
New-Item -ItemType Directory -Path $GstPluginDir -Force | Out-Null

$plugins = @(
    "libgstcoreelements.dll", "libgstplayback.dll", "libgsttypefindfunctions.dll",
    "libgstaudioconvert.dll", "libgstaudioresample.dll", "libgstvideoconvertscale.dll",
    "libgstvolume.dll", "libgstisomp4.dll", "libgstwavparse.dll", "libgstvideotestsrc.dll",
    "libgstaudiotestsrc.dll", "libgstautodetect.dll", "libgstvideofilter.dll",
    "libgstgio.dll", "libgstlibav.dll", "libgstpango.dll", "libgstogg.dll", "libgstvorbis.dll",
    "libgstopus.dll", "libgsttheora.dll", "libgstpng.dll", "libgstjpeg.dll"
)

foreach ($p in $plugins) {
    $pPath = "C:\msys64\mingw64\lib\gstreamer-1.0\$p"
    if (Test-Path $pPath) {
        Copy-Item $pPath $GstPluginDir
        Write-Host "Copied plugin: $p" -ForegroundColor Gray
        # Get dependencies of the plugin too
        Get-Dependencies $pPath
    }
}

Write-Host "Copying $($dllsToCopy.Count) DLLs to release folder..." -ForegroundColor Cyan
foreach ($dll in $dllsToCopy) {
    Copy-Item $dll "$ReleaseDir/"
}

# 5. Create the professional installer using Inno Setup
Write-Host "`nChecking for Inno Setup..." -ForegroundColor Yellow

$InnoPaths = @(
    "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
    "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe"
)

$InnoPath = $null
foreach ($path in $InnoPaths) {
    if (Test-Path $path) {
        $InnoPath = $path
        break
    }
}

if ($InnoPath) {
    Write-Host "Found Inno Setup at: $InnoPath" -ForegroundColor Gray
    
    # Ensure output directory exists
    if (-not (Test-Path "installer_output")) {
        New-Item -ItemType Directory -Path "installer_output" | Out-Null
    }

    Write-Host "Running Inno Setup to create installer..." -ForegroundColor Cyan
    & $InnoPath "installer.iss"
    if ($LASTEXITCODE -eq 0) {
        Write-Host "`nSUCCESS: Installer created in 'installer_output/' folder!" -ForegroundColor Green
    } else {
        Write-Host "`nWARNING: Inno Setup failed to create the installer." -ForegroundColor Red
    }
} else {
    Write-Host "Inno Setup 6 not found in standard locations." -ForegroundColor Gray
    Write-Host "Please ensure Inno Setup is installed and run 'ISCC.exe installer.iss' manually." -ForegroundColor Gray
}

Write-Host "`nSuccessfully packaged application in '$ReleaseDir/' folder!" -ForegroundColor Green
Write-Host "You can now zip the '$ReleaseDir' folder or use the setup in 'installer_output/'."
Write-Host "Run '$ExecutableName' to start the app."
