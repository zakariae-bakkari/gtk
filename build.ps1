param (
    [string]$Target,
    [string]$OutFile,
    [string]$ExtraSrcs = "",
    [string]$CFlags = "",
    [string]$Libs = ""
)

Write-Host "Debug ExtraSrcs: '$ExtraSrcs'"

# Enable UTF-8 encoding
$OutputEncoding = [System.Text.Encoding]::UTF8

# Set MSYS2 Path if not already in PATH to make sure gcc is accessible
$msysPath = "C:\msys64\mingw64\bin"
if (Test-Path $msysPath) {
    if ($env:PATH -notlike "*$msysPath*") {
        $env:PATH = "$msysPath;$env:PATH"
    }
}

# Clear any old compile.log
if (Test-Path "compile.log") {
    Remove-Item "compile.log" -Force
}

# Resolve target path
$targetPath = Resolve-Path $Target

# Helper to normalize file paths to flat object filenames inside obj/
function Get-ObjPath([string]$cFile) {
    if (-not (Test-Path $cFile)) {
        return ""
    }
    $resolved = (Resolve-Path $cFile).Path
    
    # Get relative path manually for PowerShell 5.1 compatibility
    $currentDir = (Get-Location).Path
    $relPath = $resolved
    if ($resolved.StartsWith($currentDir, [System.StringComparison]::OrdinalIgnoreCase)) {
        $relPath = $resolved.Substring($currentDir.Length).TrimStart('\').TrimStart('/')
    }
    
    # Create a flat name by replacing slashes and colons with underscores
    $safeName = $relPath -replace '[\\/:]', '_'
    $safeName = [System.IO.Path]::ChangeExtension($safeName, ".o")
    return Join-Path "obj_v2" $safeName
}

# Find all widget source files
$widgetSources = @()
if (Test-Path "widgets/sources") {
    $widgetSources = Get-ChildItem -Path "widgets/sources" -Filter "*.c" | ForEach-Object { $_.FullName }
}

# Find all extra source files
$extraSources = @()
if ($ExtraSrcs) {
    # Split by spaces, trimming empty tokens
    $parts = $ExtraSrcs -split "\s+" | Where-Object { $_.Trim() }
    foreach ($part in $parts) {
        if ($part) {
            $resolvedPart = Resolve-Path $part -ErrorAction SilentlyContinue
            if ($resolvedPart) {
                $extraSources += $resolvedPart.Path
            } else {
                $extraSources += $part
            }
        }
    }
}

# Build unique list of all source files
$allSources = @()
if ($targetPath) {
    $allSources += $targetPath.Path
}
foreach ($src in $extraSources) {
    if ($src -and $allSources -notcontains $src) {
        $allSources += $src
    }
}
foreach ($src in $widgetSources) {
    if ($src -and $allSources -notcontains $src) {
        $allSources += $src
    }
}

# Ensure obj/ directory exists
if (-not (Test-Path "obj_v2")) {
    New-Item -ItemType Directory -Path "obj_v2" -Force | Out-Null
}

# Find latest modification time of any .h header file to trigger rebuilds on header change
$latestHeaderTime = [DateTime]::MinValue
$headers = @()
if (Test-Path "widgets/headers") {
    $headers += Get-ChildItem -Path "widgets/headers" -Filter "*.h" -Recurse
}
if (Test-Path "src") {
    $headers += Get-ChildItem -Path "src" -Filter "*.h" -Recurse
}
foreach ($h in $headers) {
    if ($h.LastWriteTime -gt $latestHeaderTime) {
        $latestHeaderTime = $h.LastWriteTime
    }
}

# Determine which files need compilation
$compileQueue = @()
$objFiles = @()

foreach ($src in $allSources) {
    $obj = Get-ObjPath $src
    if (-not $obj -or $obj -eq "obj_v2" -or $obj -eq "obj_v2\" -or $obj -eq "obj_v2/") { continue }
    $objFiles += $obj
    
    $needsCompile = $false
    if (-not (Test-Path $obj)) {
        $needsCompile = $true
    } else {
        $srcTime = (Get-Item $src).LastWriteTime
        $objTime = (Get-Item $obj).LastWriteTime
        
        if ($srcTime -gt $objTime) {
            $needsCompile = $true
        } elseif ($latestHeaderTime -gt $objTime) {
            $needsCompile = $true
        }
    }
    
    if ($needsCompile) {
        $compileQueue += [PSCustomObject]@{
            Source = $src
            Object = $obj
        }
    }
}
Write-Host "Debug allSources: $allSources"
Write-Host "Debug objFiles: $objFiles"


# Check if target executable needs to be linked
$needsLink = $false
if (-not (Test-Path $OutFile)) {
    $needsLink = $true
} else {
    $exeTime = (Get-Item $OutFile).LastWriteTime
    foreach ($obj in $objFiles) {
        if (-not (Test-Path $obj)) {
            $needsLink = $true
            break
        }
        $objTime = (Get-Item $obj).LastWriteTime
        if ($objTime -gt $exeTime) {
            $needsLink = $true
            break
        }
    }
}

# Setup progress UI variables
$Esc = [char]27
$barWidth = 30
$totalFiles = $compileQueue.Count

# Write compile.log header
"=== Build Log started at $(Get-Date) ===" | Out-File "compile.log" -Encoding utf8

$success = $true

if ($totalFiles -gt 0) {
    # Parse CFlags
    $cFlagsList = @("-Isrc", "-Iwidgets/headers", "-Wno-deprecated-declarations", "-pipe")
    if ($CFlags) {
        $splitFlags = $CFlags -split '\s+' | Where-Object { $_.Trim() }
        $cFlagsList += $splitFlags
    }
    
    # Compile each file in the queue
    for ($i = 0; $i -lt $totalFiles; $i++) {
        # Progress from 0% to 90%
        $progress = [math]::Round(($i / $totalFiles) * 90)
        
        # Draw progress bar
        $numBars = [math]::Round(($progress * $barWidth) / 100)
        $numSpaces = $barWidth - $numBars
        $barStr = "=" * $numBars
        if ($numBars -gt 0 -and $numSpaces -gt 0) {
            $barStr = ($barStr.Substring(0, $numBars - 1)) + ">"
        }
        $spacesStr = " " * $numSpaces
        
        Write-Host -NoNewline "${Esc}[1G${Esc}[2K${Esc}[94mBuilding: [$barStr$spacesStr] $progress%${Esc}[0m"
        
        $item = $compileQueue[$i]
        
        # Setup process for GCC compiler
        $processStartInfo = New-Object System.Diagnostics.ProcessStartInfo
        $processStartInfo.FileName = "gcc"
        
        $argsList = @("-c", $item.Source, "-o", $item.Object) + $cFlagsList
        $processStartInfo.Arguments = $argsList -join " "
        
        $processStartInfo.RedirectStandardError = $true
        $processStartInfo.RedirectStandardOutput = $false
        $processStartInfo.UseShellExecute = $false
        $processStartInfo.CreateNoWindow = $true
        
        $process = [System.Diagnostics.Process]::Start($processStartInfo)
        $stderr = $process.StandardError.ReadToEnd()
        $process.WaitForExit()
        
        if ($process.ExitCode -ne 0) {
            $success = $false
            
            # Log compile failure details
            "`n[ERROR] Failed to compile: $($item.Source)" | Out-File "compile.log" -Encoding utf8 -Append
            if ($stderr) { $stderr | Out-File "compile.log" -Encoding utf8 -Append }
            break
        }
    }
}

# Link stage
if ($success -and $needsLink) {
    $numBars = [math]::Round((90 * $barWidth) / 100)
    $numSpaces = $barWidth - $numBars
    $barStr = "=" * $numBars
    if ($numSpaces -gt 0) { $barStr = $barStr + ">" }
    $spacesStr = " " * ($numSpaces - 1)
    Write-Host -NoNewline "${Esc}[1G${Esc}[2K${Esc}[94mBuilding: [$barStr$spacesStr] 90% (Linking)${Esc}[0m"
    
    # Parse Libs
    $libsList = @()
    if ($Libs) {
        $libsList += $Libs -split '\s+' | Where-Object { $_.Trim() }
    }
    $libsList += @("-lwinmm", "-lm")
    
    $processStartInfo = New-Object System.Diagnostics.ProcessStartInfo
    $processStartInfo.FileName = "gcc"
    
    $argsList = @("-o", $OutFile) + $objFiles + $libsList
    $processStartInfo.Arguments = $argsList -join " "
    
    $processStartInfo.RedirectStandardError = $true
    $processStartInfo.RedirectStandardOutput = $false
    $processStartInfo.UseShellExecute = $false
    $processStartInfo.CreateNoWindow = $true
    
    $process = [System.Diagnostics.Process]::Start($processStartInfo)
    $stderr = $process.StandardError.ReadToEnd()
    $process.WaitForExit()
    
    if ($process.ExitCode -ne 0) {
        $success = $false
        "`n[ERROR] Failed to link: $OutFile" | Out-File "compile.log" -Encoding utf8 -Append
        if ($stderr) { $stderr | Out-File "compile.log" -Append }
    }
}

if ($success) {
    # 100% Complete
    $barStr = "=" * $barWidth
    Write-Host -NoNewline "${Esc}[1G${Esc}[2K${Esc}[92mBuilding: [$barStr] 100% [OK]${Esc}[0m"
    Write-Host ""
    exit 0
} else {
    # Build Failed
    $barStr = "=" * $barWidth
    Write-Host -NoNewline "${Esc}[1G${Esc}[2K${Esc}[91mBuilding: [$barStr] Failed! [ERROR]${Esc}[0m"
    Write-Host ""
    exit 1
}
