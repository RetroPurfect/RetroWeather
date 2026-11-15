# Publish Retro Weather for free

This pack is prepared for **a public GitHub repository + GitHub Releases + GitHub Pages**.
No paid domain, hosting plan, code-signing certificate or weather API key is required
for the documented noncommercial setup. GitHub's service limits/terms still apply.
Nothing has been published or connected to an account by creating this pack.

## What is included

- `docs/`: complete static website; open `docs/index.html` for a local preview.
- `release/RetroWeather_Setup_v1.2.1.exe`: unsigned Windows installer.
- `release/RetroWeather_v1.2.1_Source.zip`: original player source/build assets.
- `release/RetroWeather_v1.2.1_Network_Source.zip`: exact helper dependency source,
  patches and upstream recipe. Publish with the installer, not only on request.
- `release/SHA256SUMS.txt`: checksums of the three release downloads.
- Application source/build files in this root; dependency notices at the top level.

The installer is deliberately labeled v1.2.1 because the accepted v1.2 baseline
has been preserved unchanged. Test this patch on Windows before announcing it
as a tested stable release. The included publishing script creates a GitHub
**prerelease** to reflect that status; remove the prerelease flag only after testing.
The website identifies the native retest still needed.

## Free publishing on your Windows build PC

1. Create a free account at https://github.com/ if needed.
2. Install free Git (https://git-scm.com/) and GitHub CLI (https://cli.github.com/).
3. In a terminal run `gh auth login` and finish its browser sign-in yourself.
4. Extract the entire Public Release Pack. In PowerShell, in its root, run:

   ```powershell
   powershell -NoProfile -ExecutionPolicy Bypass -File .\Publish-GitHub.ps1 -RepositoryName RetroWeather
   ```

   This one-session execution-policy choice runs the supplied script; inspect it
   first. No machine-wide execution-policy change is required. The script asks
   for a typed `PUBLISH` before making the repository public. It creates a NEW
   public repository on the signed-in account; it refuses an existing repository.
   It commits source and website, uploads the four release assets, enables Pages,
   and prints the public URLs. It does not alter the accepted v1.2 pack.
5. Allow GitHub Pages time to deploy, then verify all download/source links.
6. Install and run the patch, exercise CRT Mode and Help links, and run its
   benchmark. Once accepted, use the release page to clear the prerelease flag.

If your account has no Git identity, configure your preferred Git name and
email first. A GitHub-provided no-reply email can keep your personal email out
of commits. The script has no signing key and never disables SmartScreen.

The website detects owner/repository from its `OWNER.github.io/REPOSITORY/`
address; no placeholder GitHub username is published. For a custom domain,
set `window.RETRO_REPOSITORY = 'owner/repository'` before loading `site.js`.
A custom domain is optional; GitHub's default address is sufficient.

## Manual route / recover an interrupted publish

Do not rerun the new-repository script over an existing repository. Complete
remaining steps manually if publication stopped after the repository was created:

1. Put this source tree into a new PUBLIC repository, branch `main`. Respect the
   included `.gitignore`; keep release ZIPs/installer out of the Git history.
2. Create and push tag `v1.2.1` for the source commit.
3. Create a GitHub prerelease with that tag. Upload the installer, Source ZIP,
   Network Source ZIP and SHA256SUMS.txt from `release/`, preserving filenames.
4. In Settings > Pages, choose Deploy from a branch, `main`, folder `/docs`.
5. Enable Issues for feedback. The website derives its repository/issues URLs.
6. Verify all four assets download publicly without authentication and hashes match.

GitHub Pages: https://docs.github.com/en/pages/getting-started-with-github-pages/creating-a-github-pages-site
Release downloads: https://docs.github.com/en/repositories/releasing-projects-on-github/managing-releases-in-a-repository

## License / source checklist

Read LICENSE.txt, DATA_LICENSES.txt, THIRD_PARTY_NOTICES.txt and NETWORK_SOURCE.txt.
Keep all existing copyright/license texts. The player's MIT license is separate
from the helper's component licenses and Open-Meteo's free API restrictions.
Do not add a noncommercial restriction to the MIT source license; document the
API terms separately. Include complete matching helper source alongside binaries.
No claim of legal certification or unrestricted third-party API access is made.

No API secrets, user settings/cache, benchmark recordings or acceptance logs are
included in the public Source ZIP. The retained v1.2 baseline is a separate backup.
