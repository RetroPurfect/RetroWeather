param([ValidatePattern('^[A-Za-z0-9_.-]+$')][string]$RepositoryName = 'RetroWeather')
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Set-Location $PSScriptRoot
function Run-Native([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program failed (exit $LASTEXITCODE). Stop here; follow PUBLISHING.md to resume manually." }
}
foreach ($command in @('git','gh')) {
    if (-not (Get-Command $command -ErrorAction SilentlyContinue)) { throw "Install free $command first. See PUBLISHING.md." }
}
Run-Native 'gh' @('auth','status')
$account = (& gh api user --jq '.login').Trim()
if ($LASTEXITCODE -ne 0 -or $account -notmatch '^[A-Za-z0-9-]+$') { throw 'Could not resolve the signed-in GitHub account.' }
$repository = "$account/$RepositoryName"
& gh repo view $repository --json name *> $null
if ($LASTEXITCODE -eq 0) { throw "$repository already exists. No changes made. Use the manual/resume steps in PUBLISHING.md." }
if (Test-Path '.git') { throw 'This folder already has Git state. Use a fresh extracted copy or the manual steps.' }
$assets = @('release/RetroWeather_Setup_v1.2.1.exe','release/RetroWeather_v1.2.1_Source.zip','release/RetroWeather_v1.2.1_Network_Source.zip','release/SHA256SUMS.txt')
foreach ($asset in $assets) { if (-not (Test-Path -LiteralPath $asset -PathType Leaf)) { throw "Missing release asset: $asset" } }
$gitName = & git config user.name
$gitEmail = & git config user.email
if (-not $gitName -or -not $gitEmail) { throw 'Set your Git user.name and user.email (a GitHub no-reply email is fine) before publishing.' }
# Verify downloads against the prepared manifest before any public operation.
foreach ($line in Get-Content -LiteralPath 'release/SHA256SUMS.txt') {
    if ($line -match '^([a-f0-9]{64})  ([A-Za-z0-9_.-]+)$') {
        $expected = $Matches[1]; $fileName = $Matches[2]
        $actual = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path 'release' $fileName)).Hash.ToLowerInvariant()
        if ($actual -ne $expected) { throw "Checksum mismatch: $fileName" }
    } else { throw 'Unexpected checksum manifest format.' }
}
Write-Host "This will publish the source and website to NEW PUBLIC repository $repository, and upload the installer plus matching sources as prerelease v1.2.1."
if ((Read-Host 'Type PUBLISH to continue') -cne 'PUBLISH') { Write-Host 'Cancelled. Nothing published.'; exit 0 }
Run-Native 'git' @('init','-b','main')
Run-Native 'git' @('add','.')
Run-Native 'git' @('commit','-m','Prepare Retro Weather v1.2.1 public release')
Run-Native 'gh' @('repo','create',$repository,'--public','--source','.','--remote','origin','--push','--description','Free, open-source native Windows weather player and screensaver')
Run-Native 'git' @('tag','v1.2.1')
Run-Native 'git' @('push','origin','v1.2.1')
$releaseArguments = @('release','create','v1.2.1') + $assets + @('--repo',$repository,'--verify-tag','--draft','--prerelease','--title','Retro Weather v1.2.1 - public release candidate','--notes-file','RELEASE_NOTES.txt')
Run-Native 'gh' $releaseArguments
Run-Native 'gh' @('release','edit','v1.2.1','--repo',$repository,'--draft=false')
Run-Native 'gh' @('repo','edit',$repository,'--enable-issues=true')
Run-Native 'gh' @('api','--method','POST',"repos/$repository/pages",'-f','source[branch]=main','-f','source[path]=/docs')
Write-Host "Source: https://github.com/$repository"
Write-Host "Downloads: https://github.com/$repository/releases/tag/v1.2.1"
Write-Host "Website: https://$account.github.io/$RepositoryName/ (allow time for deployment)"
