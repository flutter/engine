// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:io' as io;

import 'package:http/http.dart' as http;
import 'package:path/path.dart' as p;

void main(List<String> args) async {
  // Look for a personal access token.
  final String token = _getPersonalAccessToken();

  // Print the last 10 pull requests for the flutter/flutter repository.
  final List<int> pullRequests = await _fetchPullRequests(
    token: token,
    repository: 'flutter/engine',
    max: 100,
  );

  // Create map of every check name to the number of times it has run and failed.
  final Map<String, int> checkCounts = <String, int>{};
  final Map<String, int> failedCheckCounts = <String, int>{};

  // For each PR, access the checks API to get the status of the commit.
  for (final int pullRequest in pullRequests) {
    final List<String> commits = await _fetchCommits(
      token: token,
      repository: 'flutter/engine',
      pullRequest: pullRequest,
    );
    for (final String commit in commits) {
      final List<(String name, String? status)> statuses = await _fetchCommitStatus(
        token: token,
        repository: 'flutter/engine',
        commit: commit,
      );
      print('PR $pullRequest commit $commit');
      for (final (String name, String? status) in statuses) {
        print('  $name: $status');
      }

      // Count the number of times each check has run and failed.
      for (final (String name, String? status) in statuses) {
        checkCounts[name] = (checkCounts[name] ?? 0) + 1;
        if (status == 'failure') {
          failedCheckCounts[name] = (failedCheckCounts[name] ?? 0) + 1;
        }
      }
    }
  }

  // Print the results, for each check, print the number of times it has run and failed and the failure rate.
  for (final String name in checkCounts.keys.toList()..sort()) {
    final int count = checkCounts[name]!;
    final int failedCount = failedCheckCounts[name] ?? 0;
    final double failureRate = count == 0 ? 0.0 : failedCount / count;
    print('$name: $failedCount/$count (${failureRate.toStringAsFixed(2)})');
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

/// Fetches the last N pull requests for the given repository.
Future<List<int>> _fetchPullRequests({
  required String token,
  required String repository,
  int max = 10,
}) async {
  final http.Client client = http.Client();
  try {
    final Uri url = Uri.https('api.github.com', '/repos/$repository/pulls', <String, String>{
      'per_page': '$max',
    });
    final http.Response response = await client.get(
      url,
      headers: <String, String>{
        'Authorization': 'token $token',
        'Accept': 'application/vnd.github.v3+json',
      },
    );
    if (response.statusCode != 200) {
      throw StateError('Failed to fetch pull requests: ${response.body}');
    }
    final List<Object?> json = jsonDecode(response.body) as List<Object?>;
    // Filter out PRs that have a label of 'platform-web'.
    return json.map<int>((Object? pr) {
      final Map<String, Object?> prMap = pr! as Map<String, Object?>;
      final List<Object?> labels = prMap['labels']! as List<Object?>;
      if (!labels.any((Object? label) => (label! as Map<String, Object?>)['name'] == 'platform-web')) {
        return -1;
      }
      return prMap['number']! as int;
    }).where((int pr) => pr != -1).toList();
    // return json.map<int>((Object? pr) {
    //   return (pr! as Map<String, Object?>)['number']! as int;
    // }).toList();
  } finally {
    client.close();
  }
}

/// Fetches every commit for the given pull request.
Future<List<String>> _fetchCommits({
  required String token,
  required String repository,
  required int pullRequest,
}) async {
  final http.Client client = http.Client();
  try {
    final Uri url = Uri.https('api.github.com', '/repos/$repository/pulls/$pullRequest/commits');
    final http.Response response = await client.get(
      url,
      headers: <String, String>{
        'Authorization': 'token $token',
        'Accept': 'application/vnd.github.v3+json',
      },
    );
    if (response.statusCode != 200) {
      throw StateError('Failed to fetch commits: ${response.body}');
    }
    final List<Object?> json = jsonDecode(response.body) as List<Object?>;
    return json.map<String>((Object? commit) {
      return (commit! as Map<String, Object?>)['sha']! as String;
    }).toList();
  } finally {
    client.close();
  }
}

/// Fetches the name and status of each check for the given commit.
Future<List<(String, String?)>> _fetchCommitStatus({
  required String token,
  required String repository,
  required String commit,
}) async {
  final http.Client client = http.Client();
  try {
    final Uri url = Uri.https('api.github.com', '/repos/$repository/commits/$commit/check-runs');
    final http.Response response = await client.get(
      url,
      headers: <String, String>{
        'Authorization': 'token $token',
        'Accept': 'application/vnd.github.v3+json',
      },
    );
    if (response.statusCode != 200) {
      throw StateError('Failed to fetch commit status: ${response.body}');
    }
    final Map<String, Object?> json = jsonDecode(response.body) as Map<String, Object?>;
    final List<Object?> checks = json['check_runs']! as List<Object?>;
    return checks.map((Object? check) {
      final Map<String, Object?> checkMap = check! as Map<String, Object?>;
      return (checkMap['name']! as String, checkMap['conclusion'] as String?);
    }).toList();
  } finally {
    client.close();
  }
}
