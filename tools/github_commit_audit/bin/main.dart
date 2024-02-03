// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:io' as io;

import 'package:args/args.dart';
import 'package:github_commit_audit/src/github_client.dart';
import 'package:http/http.dart' as http;
import 'package:path/path.dart' as p;

void main(List<String> args) async {
  // Look for a personal access token.
  final String token = _getPersonalAccessToken();

  // Determine what repository to fetch pull requests from.
  final ArgParser parser = ArgParser()
    ..addOption(
      'repository',
      abbr: 'r',
      defaultsTo: 'flutter/engine',
      help: 'The repository to fetch pull requests from.',
    )
    ..addOption(
      'max',
      help: 'The maximum number of most recently updated pull requests to fetch.',
      defaultsTo: '100',
      valueHelp: '1 to 100',
    );

  final ArgResults results = parser.parse(args);
  final String repository = results['repository'] as String;
  final int max = int.parse(results['max'] as String);

  // Create a client.
  final http.Client client = http.Client();
  try {
    final GithubClient github = GithubClient(client, token: token);

    // Fetch some pull requests.
    final List<PullRequest> pullRequests = await github.fetchPullRequests(repository, max: max).toList();

    // Debug: Print the names of the pull requests.
    for (final PullRequest pr in pullRequests) {
      print('PR #${pr.number}: ${pr.title}');

      // Fetch the commits for the pull request.
      final List<PullRequestCommit> commits = await github.fetchCommits(repository, pr.number).toList();
      for (final PullRequestCommit commit in commits) {
        print('  - ${commit.sha}');

        // Fetch the statuses for the commit.
        final List<PullRequestCommitCheck> statuses = await github.fetchCommitStatus(repository, commit.sha).toList();
        for (final PullRequestCommitCheck status in statuses) {
          print('    - ${status.name}: ${status.conclusion?.name ?? 'null'}');
        }
      }
    }
  } finally {
    client.close();
  }
}


/// Returns the personal access token from the environment or a file.
///
/// The personal access token is used to authenticate with the GitHub API.
///
/// If no token is found an error is thrown.
String _getPersonalAccessToken() {
  String? token = io.Platform.environment['GITHUB_TOKEN'];
  if (token == null) {
    // Find a file called .github_token relative to this script.
    final String self = io.File.fromUri(io.Platform.script).absolute.parent.path;
    final io.File tokenFile = io.File(p.join(p.dirname(self), '.github_token'));
    if (tokenFile.existsSync()) {
      token = tokenFile.readAsStringSync().trim();
    }
  }
  if (token == null) {
    throw StateError(
        'No GitHub token found. Set a GITHUB_TOKEN environment variable or '
        'create a .github_token file in the same directory of the '
        'github_commit_audit package (i.e. next to the README).');
  }
  return token;
}
