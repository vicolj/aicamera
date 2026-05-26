# Cursor stop hook: auto stage, commit, and optionally push when the agent finishes.
# Requires GitHub auth if push is enabled and origin remote exists.

param(
    [switch]$Push = $true
)

$ErrorActionPreference = "Stop"

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

$status = git status --porcelain 2>$null
if (-not $status) {
    exit 0
}

git add -A

$timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
$message = "chore: auto commit after agent session ($timestamp)"
git commit -m $message 2>$null
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
