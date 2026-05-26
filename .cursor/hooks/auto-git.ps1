# Cursor stop hook: auto stage and commit when the agent finishes.
# Push is disabled by default; pass -Push to enable.

param(
    [switch]$Push = $false
)

$ErrorActionPreference = "SilentlyContinue"

$inputJson = [Console]::In.ReadToEnd()
if (-not $inputJson) {
    $inputJson = "{}"
}

try {
    $payload = $inputJson | ConvertFrom-Json
} catch {
    $payload = $null
}

$repoRoot = (Get-Location).Path
if ($payload -and $payload.workspace_roots -and $payload.workspace_roots.Count -gt 0) {
    $repoRoot = $payload.workspace_roots[0]
}

Set-Location $repoRoot

if (-not (Test-Path ".git")) {
    exit 0
}

$changes = git status --porcelain 2>$null
if (-not $changes) {
    exit 0
}

git add -A

$files = @()
foreach ($line in ($changes -split "`n")) {
    if (-not $line) { continue }
    $path = $line.Substring(3).Trim()
    if ($path -match ' -> ') {
        $path = ($path -split ' -> ')[-1]
    }
    $files += $path
}

$timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
$summary = ($files | Select-Object -First 8) -join ", "
if ($files.Count -gt 8) {
    $summary += " (+$($files.Count - 8) more)"
}

$message = @"
chore: auto commit ($timestamp)

$summary
"@

git commit -m $message.Trim()
if ($LASTEXITCODE -ne 0) {
    exit 0
}

if ($Push) {
    $remote = git remote get-url origin 2>$null
    if ($LASTEXITCODE -eq 0 -and $remote) {
        git push -u origin HEAD 2>$null
    }
}

exit 0
