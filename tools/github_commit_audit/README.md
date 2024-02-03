# `github_commit_audit`

An _experimental_ (read: not used on CI) tool to audit commits in a GitHub
repository.

Specifically, it reports how many commits have failed checks. This tool can be
used to "audit" if the reported flakiness of a repository seems to be
undercounted.

## Usage

This tool uses a [personal access token](https://docs.github.com/en/github/authenticating-to-github/creating-a-personal-access-token).

Either:

- Set a `GITHUB_TOKEN` environment variable
- Create a file called `.github_token` in the package root (i.e. next to this
  README) with the token.

```bash
# Assuming you're in the root of the repository.
dart ./tools/github_commit_audit/bin/main.dart
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
