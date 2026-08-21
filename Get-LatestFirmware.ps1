#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Download the latest firmware artifact built by the "Build Voyager firmware"
    GitHub Actions workflow, unzip it, and store it in this project.

.DESCRIPTION
    Queries the GitHub REST API for the most recent artifact produced by the
    workflow, downloads its .zip, extracts it into a folder in the repo, and
    (by default) prunes older extracts so only the newest firmware remains.

    Downloading Actions artifacts ALWAYS requires authentication, even for
    public repos. Provide a token via -Token, or set $env:GITHUB_TOKEN /
    $env:GH_TOKEN. A classic PAT needs the "repo" scope (or "actions:read" for
    a fine-grained token) on jackhen124/keyboard.

.PARAMETER Repo
    owner/name of the GitHub repository. Defaults to jackhen124/keyboard.

.PARAMETER ArtifactName
    Name of the artifact to fetch. Defaults to voyager-jack-firmware.

.PARAMETER Destination
    Folder (relative to the repo root) to extract the firmware into.
    Defaults to voyager-flash/firmware.

.PARAMETER Token
    GitHub token. Falls back to $env:GITHUB_TOKEN then $env:GH_TOKEN.

.PARAMETER KeepZip
    Keep the downloaded .zip alongside the extracted files (default: removed).

.EXAMPLE
    $env:GITHUB_TOKEN = "ghp_xxx"
    ./Get-LatestFirmware.ps1

.EXAMPLE
    ./Get-LatestFirmware.ps1 -Token ghp_xxx -Destination build
#>

[CmdletBinding()]
param(
    [string]$Repo = 'jackhen124/keyboard',
    [string]$ArtifactName = 'voyager-jack-firmware',
    [string]$Destination = 'voyager-flash/firmware',
    [string]$Token,
    [switch]$KeepZip
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

# --- Resolve token -----------------------------------------------------------
if (-not $Token) { $Token = $env:GITHUB_TOKEN }
if (-not $Token) { $Token = $env:GH_TOKEN }
if (-not $Token) {
    throw "No GitHub token found. Pass -Token, or set `$env:GITHUB_TOKEN / `$env:GH_TOKEN. " +
          "A classic PAT needs the 'repo' scope (fine-grained: 'actions:read')."
}

$headers = @{
    Authorization          = "Bearer $Token"
    Accept                 = 'application/vnd.github+json'
    'X-GitHub-Api-Version' = '2022-11-28'
    'User-Agent'           = 'keyboard-firmware-fetch'
}

# --- Resolve destination relative to the repo root (this script's folder) ----
$repoRoot = $PSScriptRoot
if (-not $repoRoot) { $repoRoot = (Get-Location).Path }
$destPath = if ([System.IO.Path]::IsPathRooted($Destination)) {
    $Destination
} else {
    Join-Path $repoRoot $Destination
}

# --- Find the latest matching artifact ---------------------------------------
Write-Host "Looking up latest '$ArtifactName' artifact in $Repo ..." -ForegroundColor Cyan

$listUri = "https://api.github.com/repos/$Repo/actions/artifacts?per_page=100&name=$ArtifactName"
$response = Invoke-RestMethod -Uri $listUri -Headers $headers

$artifact = $response.artifacts |
    Where-Object { $_.name -eq $ArtifactName -and -not $_.expired } |
    Sort-Object { [datetime]$_.created_at } -Descending |
    Select-Object -First 1

if (-not $artifact) {
    throw "No non-expired artifact named '$ArtifactName' found in $Repo. " +
          "Has the 'Build Voyager firmware' workflow succeeded recently?"
}

$created = ([datetime]$artifact.created_at).ToString('yyyy-MM-dd HH:mm:ss')
Write-Host "Found artifact #$($artifact.id) (run $($artifact.workflow_run.id)), created $created UTC." -ForegroundColor Green

# --- Download the artifact zip -----------------------------------------------
# The download endpoint returns a 302 to a signed storage URL. We grab the
# redirect target ourselves and fetch it WITHOUT the GitHub auth header, since
# storage rejects the Authorization header.
New-Item -ItemType Directory -Force -Path $destPath | Out-Null
$zipPath = Join-Path $destPath "$ArtifactName.zip"

Write-Host "Downloading artifact zip ..." -ForegroundColor Cyan
$dlUri = "https://api.github.com/repos/$Repo/actions/artifacts/$($artifact.id)/zip"
$redirect = Invoke-WebRequest -Uri $dlUri -Headers $headers -MaximumRedirection 0 -SkipHttpErrorCheck
$location = $redirect.Headers.Location
if ($location -is [array]) { $location = $location[0] }
if (-not $location) {
    throw "Expected a redirect to a download URL but got HTTP $($redirect.StatusCode)."
}
Invoke-WebRequest -Uri $location -OutFile $zipPath

# --- Extract, replacing any previous contents --------------------------------
Write-Host "Extracting to $destPath ..." -ForegroundColor Cyan
Get-ChildItem -Path $destPath -Exclude "$ArtifactName.zip" | Remove-Item -Recurse -Force
Expand-Archive -Path $zipPath -DestinationPath $destPath -Force

if (-not $KeepZip) { Remove-Item $zipPath -Force }

# --- Report ------------------------------------------------------------------
$files = Get-ChildItem -Path $destPath -Recurse -File | Where-Object { $_.Name -ne "$ArtifactName.zip" }
Write-Host ""
Write-Host "Done. Firmware in $destPath :" -ForegroundColor Green
$files | ForEach-Object {
    Write-Host ("  {0,-40} {1,10:N0} bytes" -f $_.Name, $_.Length)
}
