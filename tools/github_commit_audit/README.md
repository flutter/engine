# `github_commit_audit`

An _experimental_ (read: not used on CI) tool to audit commits in a GitHub
repository.

Specifically, it reports how many commits have failed checks. This tool can be
used to "audit" if the reported flakiness of a repository seems to be
undercounted.

This is a "for fun" tool. PRs welcome.

## Usage

This tool uses a [personal access token](https://docs.github.com/en/github/authenticating-to-github/creating-a-personal-access-token).

Either:

- Set a `GITHUB_TOKEN` environment variable
- Create a file called `.github_token` in the package root (i.e. next to this
  README) with the token.

```shell
# Assuming you're in the root of the repository.
dart ./tools/github_commit_audit/bin/main.dart
```

... will give output similar to:

```txt
Summary:
  - Linux Framework Smoke Tests: 56/126 (0.44)
  - Linux linux_license: 52/126 (0.41)
  - Linux linux_host_engine: 41/126 (0.33)
  - Linux Fuchsia FEMU: 41/126 (0.33)
  - Linux Web Framework tests: 23/71 (0.32)
  - Linux linux_web_engine: 39/126 (0.31)
  - Linux linux_fuchsia: 37/122 (0.30)
  - Linux linux_android_emulator_tests: 20/66 (0.30)
  - Linux linux_unopt: 38/126 (0.30)
  - Linux mac_unopt: 37/126 (0.29)
  - Mac mac_host_engine: 36/126 (0.29)
  - Windows windows_unopt: 31/126 (0.25)
  - Linux linux_android_debug_engine: 27/126 (0.21)
  - Linux linux_clang_tidy: 21/99 (0.21)
  - Windows windows_host_engine: 25/126 (0.20)
  - Mac mac_ios_engine: 20/126 (0.16)
  - Mac mac_clang_tidy: 15/103 (0.15)
  - Linux linux_android_aot_engine: 16/126 (0.13)
  - Windows windows_android_aot_engine: 15/126 (0.12)
  - Windows windows_arm_host_engine: 13/126 (0.10)
  - Linux linux_host_desktop_engine: 11/126 (0.09)
  - Linux mac_android_aot_engine: 10/126 (0.08)
  - Linux linux_arm_host_engine: 9/126 (0.07)
  - triage: 0/127 (0.00)
  - ci.yaml validation: 0/118 (0.00)
  - cla/google: 0/117 (0.00)
  - Vulnerability scanning: 0/109 (0.00)
  - Extract Dependencies: 0/109 (0.00)
```

## Options

- `--help`: Show help message.
- `--repo`: The repository to audit. Defaults to `flutter/engine`.
- `--include-label`: Skip PRs that don't have this label.
- `--exclude-label`: Skip PRs that have this label.

## Examples

```bash
dart ./tools/github_commit_audit/bin/main.dart \
  --repo flutter/engine \
  --exclude-label "platform-web"
```
