param(
    [string]$CompilerExe = ".\my_compiler.exe",
    [string]$Manifest = ".\tests\manifest.txt"
)

if (-not (Test-Path $CompilerExe)) {
    Write-Host "Compiler binary not found: $CompilerExe"
    Write-Host "Build first: make build"
    exit 1
}

if (-not (Test-Path $Manifest)) {
    Write-Host "Manifest not found: $Manifest"
    exit 1
}

$total = 0
$passed = 0

Get-Content $Manifest | ForEach-Object {
    $line = $_.Trim()
    if ($line.Length -eq 0) { return }
    if ($line.StartsWith("#")) { return }

    $parts = $line.Split("|")
    if ($parts.Count -ne 2) {
        Write-Host "Invalid manifest line: $line"
        exit 1
    }

    $file = $parts[0]
    $expected = $parts[1]
    $total++

    & $CompilerExe --test $file $expected
    if ($LASTEXITCODE -eq 0) {
        $passed++
        Write-Host "[PASS] $file"
    } else {
        Write-Host "[FAIL] $file (expected: $expected)"
    }
}

Write-Host ""
Write-Host "Passed: $passed / $total"
if ($passed -ne $total) {
    exit 1
}
exit 0