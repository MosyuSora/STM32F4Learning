param(
    [string]$DemoDir = $PSScriptRoot
)

$ErrorActionPreference = "Stop"
$demo = Join-Path $DemoDir "demo.c"
$expected = Join-Path $DemoDir "expected-output.txt"
$buildDir = Join-Path $DemoDir ".build"
$exe = Join-Path $buildDir "demo.exe"

if (!(Test-Path -LiteralPath $demo)) {
    $children = Get-ChildItem -LiteralPath $DemoDir -Directory | Where-Object {
        Test-Path -LiteralPath (Join-Path $_.FullName "run.ps1")
    } | Sort-Object Name

    if ($children.Count -eq 0) {
        throw "demo.c not found: $demo"
    }

    foreach ($child in $children) {
        Write-Host "=== $($child.Name) ==="
        & powershell -ExecutionPolicy Bypass -File (Join-Path $child.FullName "run.ps1")
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }
    }
    exit 0
}

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

$gcc = Get-Command gcc -ErrorAction SilentlyContinue
$clang = Get-Command clang -ErrorAction SilentlyContinue
$cl = Get-Command cl -ErrorAction SilentlyContinue

if ($gcc) {
    & $gcc.Source -std=c99 -Wall -Wextra -pedantic $demo -o $exe
    & $exe
    exit $LASTEXITCODE
}

if ($clang) {
    & $clang.Source -std=c99 -Wall -Wextra -pedantic $demo -o $exe
    & $exe
    exit $LASTEXITCODE
}

if ($cl) {
    Push-Location $buildDir
    try {
        & $cl.Source /nologo /W4 /TC $demo /Fe:demo.exe
        & $exe
        exit $LASTEXITCODE
    }
    finally {
        Pop-Location
    }
}

Write-Host "No C compiler found in PATH. Showing expected output instead."
if (Test-Path -LiteralPath $expected) {
    Get-Content -LiteralPath $expected
}
else {
    Write-Host "expected-output.txt not found."
}
