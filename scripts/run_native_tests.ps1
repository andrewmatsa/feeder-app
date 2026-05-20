# Run PlatformIO native C++ unit tests using MSVC (cl.exe).
# Usage: .\scripts\run_native_tests.ps1
#
# If GCC/MinGW is installed, use: pio test -e native
# This script is a Windows fallback using the already-installed MSVC Build Tools.

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$testDir = Join-Path $root "test"
$buildDir = Join-Path $root ".pio\native_build"

# Find cl.exe
$cl = Get-ChildItem "C:\Program Files (x86)\Microsoft Visual Studio" -Recurse -Filter "cl.exe" -ErrorAction SilentlyContinue |
      Where-Object { $_.FullName -match "Hostx64\\x64" } |
      Select-Object -First 1

if (-not $cl) {
    Write-Error "cl.exe not found. Install MSVC Build Tools or MinGW and add to PATH."
    exit 1
}

Write-Host "Compiler: $($cl.FullName)" -ForegroundColor Cyan

# Find Unity library source (downloaded by PlatformIO)
$unityDir = Get-ChildItem "$root\.pio" -Recurse -Filter "unity.c" -ErrorAction SilentlyContinue |
            Select-Object -First 1 | ForEach-Object { $_.DirectoryName }

if (-not $unityDir) {
    Write-Host "Unity not found -- run 'pio test -e native' once to download it, then retry." -ForegroundColor Yellow
    exit 1
}

New-Item -ItemType Directory -Path $buildDir -Force | Out-Null

# cl.exe is at: ...VC\Tools\MSVC\<ver>\bin\Hostx64\x64\cl.exe
# vcvars64.bat is at: ...VC\Auxiliary\Build\vcvars64.bat
# Search upward from cl.exe directory to find it
$vcvars = Get-ChildItem (Split-Path $cl.FullName -Parent) -Recurse -Filter "vcvars64.bat" -ErrorAction SilentlyContinue |
          Select-Object -First 1 -ExpandProperty FullName
if (-not $vcvars) {
    # Walk up the tree looking for VC\Auxiliary\Build\vcvars64.bat
    $dir = Split-Path $cl.FullName -Parent
    while ($dir -and $dir -ne (Split-Path $dir -Parent)) {
        $candidate = Join-Path $dir "Auxiliary\Build\vcvars64.bat"
        if (Test-Path $candidate) { $vcvars = $candidate; break }
        $dir = Split-Path $dir -Parent
    }
}
if (-not $vcvars) {
    Write-Error "vcvars64.bat not found near cl.exe."
    exit 1
}

$testSuites = @("test_servo", "test_battery")
$passed = 0
$failed = 0

foreach ($suite in $testSuites) {
    $src = Join-Path $testDir "$suite\$suite.cpp"
    if (-not (Test-Path $src)) { continue }

    $exe = Join-Path $buildDir "$suite.exe"

    Write-Host ""
    Write-Host "Building $suite..." -ForegroundColor Cyan

    # Write a temporary batch file so we avoid PowerShell 5.1 && quoting issues
    $bat = Join-Path $buildDir "build_$suite.bat"
    $clExe = $cl.FullName
    $batContent = "@echo off`r`ncall `"$vcvars`" 2>nul`r`n`"$clExe`" /EHsc /std:c++17 /DNATIVE_TEST /I`"$testDir`" /I`"$unityDir`" `"$src`" `"$unityDir\unity.c`" /Fe:`"$exe`" /nologo`r`n"
    [System.IO.File]::WriteAllText($bat, $batContent, [System.Text.Encoding]::ASCII)

    $result = cmd /c $bat 2>&1
    Remove-Item $bat -ErrorAction SilentlyContinue

    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build FAILED for ${suite}:" -ForegroundColor Red
        Write-Host $result
        $failed++
        continue
    }

    Write-Host "Running $suite..." -ForegroundColor Cyan
    & $exe
    if ($LASTEXITCODE -eq 0) {
        $passed++
    } else {
        $failed++
    }
}

Write-Host ""
Write-Host ("=" * 40)
if ($failed -eq 0) {
    Write-Host "All $passed suite(s) passed!" -ForegroundColor Green
} else {
    Write-Host "$passed passed, $failed failed." -ForegroundColor Red
}
exit $failed
