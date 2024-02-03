// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
      'repo',
      abbr: 'r',
      defaultsTo: 'flutter/engine',
      help: 'The repository to fetch pull requests from.',
    )
    ..addOption(
      'max',
      help: 'The maximum number of most recently updated pull requests to fetch.',
      defaultsTo: '100',
      valueHelp: '1 to 100',
    )
    ..addMultiOption(
      'include-label',
      help: 'Include pull requests with the given label.',
      valueHelp: 'label',
    )
    ..addMultiOption(
      'exclude-label',
      help: 'Exclude pull requests with the given label.',
      valueHelp: 'label',
    )
    ..addFlag(
      'verbose',
      abbr: 'v',
      help: 'Print verbose output.',
    )
    ..addFlag(
      'help',
      abbr: 'h',
      help: 'Print this usage information.',
    );

  final ArgResults results = parser.parse(args);
  final String repository = results['repo'] as String;
  final int max = int.parse(results['max'] as String);
  final List<String> includeLabels = List<String>.from(results['include-label'] as List<Object?>);
  final List<String> excludeLabels = List<String>.from(results['exclude-label'] as List<Object?>);
  final bool verbose = results['verbose'] as bool;
  final bool help = results['help'] as bool;

  if (help) {
    print('Usage: github_commit_audit [options]');
    print(parser.usage);
    return;
  }

  // Can have either include or exclude labels, but not both.
  if (includeLabels.isNotEmpty && excludeLabels.isNotEmpty) {
    print('Cannot have both include and exclude labels.');
    io.exitCode = 1;
    return;
  }

  bool shouldSkipBasedOnLabel(Iterable<String> labels) {
    if (includeLabels.isNotEmpty) {
      return !labels.any(includeLabels.contains);
    }
    if (excludeLabels.isNotEmpty) {
      return labels.any(excludeLabels.contains);
    }
    return false;
  }

  void vprint(Object? object) {
    if (verbose) {
      print(object);
    }
  }

  // Count how many times a check has run.
  final Map<String, int> checkCounts = <String, int>{};

  // Count how many times a check has explicitly failed.
  final Map<String, int> checkFailures = <String, int>{};

  // Create a client.
  final http.Client client = http.Client();
  try {
    final GithubClient github = GithubClient(client, token: token);

    // Fetch some pull requests.
    final List<PullRequest> pullRequests = await github.fetchPullRequests(repository, max: max).toList();

    // Debug: Print the names of the pull requests.
    int i = 1;
    for (final PullRequest pr in pullRequests) {
      if (shouldSkipBasedOnLabel(pr.labels)) {
        vprint('[Skipping] PR #${pr.number}: ${pr.title}');
        continue;
      }

      print('[${i++}/${pullRequests.length}] PR #${pr.number}: ${pr.title}');

      // Fetch the commits for the pull request.
      final List<PullRequestCommit> commits = await github.fetchCommits(repository, pr.number).toList();
      for (final PullRequestCommit commit in commits) {
        vprint('  - ${commit.sha}');

        // Fetch the statuses for the commit.
        final List<PullRequestCommitCheck> statuses = await github.fetchCommitStatus(repository, commit.sha).toList();
        for (final PullRequestCommitCheck status in statuses) {
          vprint('    - ${status.name}: ${status.conclusion?.name ?? 'null'}');
          checkCounts[status.name] = (checkCounts[status.name] ?? 0) + 1;
          if (status.conclusion == CheckRunConclusion.failure) {
            checkFailures[status.name] = (checkFailures[status.name] ?? 0) + 1;
          }
        }
      }
    }
  } finally {
    client.close();
  }

  // Print the check counts sorted by a ratio of highest failures.
  final List<_Summary> summaries = <_Summary>[];
  for (final String name in checkCounts.keys) {
    summaries.add(_Summary(
      name: name,
      runCount: checkCounts[name]!,
      failureCount: checkFailures[name] ?? 0,
    ));
  }

  summaries.sort((_Summary a, _Summary b) => b.failureRatio.compareTo(a.failureRatio));

  print('Summary:');
  for (final _Summary summary in summaries) {
    print('  - ${summary.name}: ${summary.failureCount}/${summary.runCount} (${summary.failureRatio.toStringAsFixed(2)})');
  }
}

final class _Summary {
  const _Summary({
    required this.name,
    required this.runCount,
    required this.failureCount,
  });

  final String name;
  final int runCount;
  final int failureCount;

  double get failureRatio => failureCount / runCount;
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
