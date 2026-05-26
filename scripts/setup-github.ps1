param(
    [string]$RepoName = "edger-rec",
    [switch]$Private,
    [switch]$SkipCreateRepo
)

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot\..

$gh = "C:\Program Files\GitHub CLI\gh.exe"
if (-not (Test-Path $gh)) {
    $gh = (Get-Command gh -ErrorAction SilentlyContinue).Source
}
if (-not $gh) {
    Write-Host "GitHub CLI (gh) not found. Install: winget install GitHub.cli"
    exit 1
}

Write-Host "Step 1/4: GitHub login (browser will open if not logged in)..."
& $gh auth login -h github.com -p https -w
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Step 2/4: Git credential manager login..."
git credential-manager github login
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if (-not (Test-Path ".git")) {
    Write-Host "Step 3/4: Initializing git repository..."
    git init
    git branch -M main
} else {
    Write-Host "Step 3/4: Git repository already initialized."
}

if (-not $SkipCreateRepo) {
    Write-Host "Step 4/4: Creating GitHub repo and setting origin..."
    $visibility = if ($Private) { "--private" } else { "--public" }
    & $gh repo create $RepoName $visibility --source=. --remote=origin --push
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Repo may already exist. Trying to set remote manually..."
        $user = (& $gh api user -q .login)
        git remote remove origin 2>$null
        git remote add origin "https://github.com/$user/$RepoName.git"
        git push -u origin main
    }
} else {
    Write-Host "Step 4/4: Skipped repo creation. Add remote manually:"
    Write-Host "  git remote add origin https://github.com/<USER>/$RepoName.git"
    Write-Host "  git push -u origin main"
}

Write-Host "Done. GitHub is linked and ready."
