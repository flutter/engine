// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';

import 'package:http/http.dart' as http;
import 'package:meta/meta.dart';

/// A minimal GitHub client that can fetch pull requests and commit statuses.
final class GithubClient {
  /// Creates a new GitHub client that uses the given [client] to make requests.
  const GithubClient(this._client, {required String token})
      : _token = 'token $token';

  final http.Client _client;
  final String _token;

  Future<Object> _fetchJson(Uri url) async {
    final http.Response response = await _client.get(url, headers: <String, String>{
      'Accept': 'application/vnd.github.v3+json',
      'Authorization': _token,
    });
    if (response.statusCode != 200) {
      throw StateError('Failed to fetch $url: ${response.statusCode} ${response.reasonPhrase}');
    }
    return json.decode(response.body) as Object;
  }

  /// Returns up to [max] pull requests for the given [repository].
  ///
  /// Pull requests are sorted by their last update time (most recent first).
  ///
  /// ## Optional parameters
  ///
  /// - [base]: The base branch to compare the pull requests against.
  ///   Defaults to 'main'.
  /// - [max]: The maximum number of pull requests to fetch.
  ///   Defaults to 100, which is also the maximum.
  Stream<PullRequest> fetchPullRequests(
    String repository, {
    String base = 'main',
    int max = 100,
    PullRequestState? state,
  }) async* {
    RangeError.checkValueInInterval(max, 1, 100, 'max');
    final Uri url = Uri.https(
      'api.github.com',
      '/repos/$repository/pulls',
      <String, String>{
        'base': base,
        'state': state?.name ?? 'all',
        'per_page': '$max',
      },
    );
    final List<Object> json = (await _fetchJson(url) as List<Object?>).cast<Object>();
    for (final Object pr in json) {
      yield PullRequest.fromJson(pr as Map<String, Object?>);
    }
  }

  /// Fetches every commit for the given pull request.
  Stream<PullRequestCommit> fetchCommits(
    String repository,
    int pullRequest,
  ) async* {
    final Uri url = Uri.https('api.github.com', '/repos/$repository/pulls/$pullRequest/commits');
    final List<Object> json = (await _fetchJson(url) as List<Object?>).cast<Object>();
    for (final Object commit in json) {
      yield PullRequestCommit.fromJson(commit as Map<String, Object?>);
    }
  }

  /// Fetches the name and status of each check for the given commit.
  Stream<PullRequestCommitCheck> fetchCommitStatus(
    String repository,
    String commit,
  ) async* {
    final Uri url = Uri.https('api.github.com', '/repos/$repository/commits/$commit/check-runs');
    final Map<String, Object> json = (await _fetchJson(url) as Map<String, Object?>).cast<String, Object>();
    final List<Object?> checks = json['check_runs']! as List<Object?>;
    for (final Object? check in checks) {
      final Map<String, Object?> checkMap = check! as Map<String, Object?>;
      yield PullRequestCommitCheck.fromJson(checkMap);
    }
  }
}

/// https://docs.github.com/en/rest/pulls/pulls?apiVersion=2022-11-28
@immutable
final class PullRequest {
  /// Creates a pull request with the given properties.
  const PullRequest({
    required this.url,
    required this.number,
    required this.state,
    required this.title,
    required this.labels,
    required this.isDraft,
    required this.createdAt,
    required this.updatedAt,
  });

  /// Parses a pull request from a JSON map.
  factory PullRequest.fromJson(Map<String, Object?> json) {
    return PullRequest(
      url: json['url']! as String,
      number: json['number']! as int,
      state: PullRequestState.fromJson(json['state']! as String),
      title: json['title']! as String,
      labels: (json['labels']! as List<Object?>).map<String>((Object? label) {
        return (label! as Map<String, Object?>)['name']! as String;
      }).toSet(),
      isDraft: json['draft']! as bool,
      createdAt: DateTime.parse(json['created_at']! as String),
      updatedAt: DateTime.parse(json['updated_at']! as String),
    );
  }

  /// The pull request URL.
  final String url;

  /// The pull request number.
  final int number;

  /// The pull request state.
  final PullRequestState state;

  /// The pull request title.
  final String title;

  /// Labels associated with the pull request.
  final Set<String> labels;

  /// Whether the pull request is a draft.
  final bool isDraft;

  /// When the pull request was created.
  final DateTime createdAt;

  /// When the pull request was last updated.
  final DateTime updatedAt;
}

/// The state of a pull request.
enum PullRequestState {
  /// The pull request is open (including draft pull requests).
  open,

  /// The pull request is closed (including merged or declined pull requests).
  closed;

  /// Parses a pull request state from a JSON string.
  factory PullRequestState.fromJson(String value) {
    switch (value) {
      case 'open':
        return PullRequestState.open;
      case 'closed':
        return PullRequestState.closed;
      default:
        throw ArgumentError.value(value, 'value', 'Invalid pull request state');
    }
  }
}

/// https://docs.github.com/en/rest/pulls/pulls?apiVersion=2022-11-28#list-commits-on-a-pull-request
@immutable
final class PullRequestCommit {
  /// Creates a pull request commit with the given properties.
  const PullRequestCommit({
    required this.url,
    required this.sha,
    required this.message,
  });

  /// Parses a pull request commit from a JSON map.
  factory PullRequestCommit.fromJson(Map<String, Object?> json) {
    return PullRequestCommit(
      url: (json['commit']! as Map<String, Object?>)['url']! as String,
      sha: json['sha']! as String,
      message: (json['commit']! as Map<String, Object?>)['message']! as String,
    );
  }

  /// The URL of the commit.
  final String url;

  /// The SHA of the commit.
  final String sha;

  /// The commit message.
  final String message;
}

/// Possible conclusions for a check run.
enum CheckRunConclusion {
  /// The check run succeeded.
  success,

  /// The check run failed.
  failure,

  /// The check run was neutral.
  neutral,

  /// The check run was cancelled.
  cancelled,

  /// The check run timed out.
  timedOut,

  /// The check run has an action required.
  actionRequired;

  /// Parses a check run conclusion from a JSON string.
  ///
  /// Returns `null` if the value is not a recognized conclusion (still valid).
  static CheckRunConclusion? fromJson(String? value) {
    switch (value) {
      case 'success':
        return CheckRunConclusion.success;
      case 'failure':
        return CheckRunConclusion.failure;
      case 'neutral':
        return CheckRunConclusion.neutral;
      case 'cancelled':
        return CheckRunConclusion.cancelled;
      case 'timed_out':
        return CheckRunConclusion.timedOut;
      case 'action_required':
        return CheckRunConclusion.actionRequired;
      default:
        return null;
    }
  }
}

/// https://docs.github.com/en/rest/checks/runs?apiVersion=2022-11-28#list-check-runs-for-a-git-reference
final class PullRequestCommitCheck {
  /// Creates a pull request commit check with the given properties.
  const PullRequestCommitCheck({
    required this.url,
    required this.name,
    required this.conclusion,
    required this.startedAt,
    required this.completedAt,
  });

  /// Parses a pull request commit check from a JSON map.
  factory PullRequestCommitCheck.fromJson(Map<String, Object?> json) {
    return PullRequestCommitCheck(
      url: json['html_url']! as String,
      name: json['name']! as String,
      conclusion: CheckRunConclusion.fromJson(json['conclusion'] as String?),
      startedAt: json['started_at'] == null
          ? null
          : DateTime.parse(json['started_at']! as String),
      completedAt: json['completed_at'] == null
          ? null
          : DateTime.parse(json['completed_at']! as String),
    );
  }

  /// The URL of the check.
  final String url;

  /// The name of the check.
  final String name;

  /// The conclusion of the check, if available.
  final CheckRunConclusion? conclusion;

  /// When the check was started.
  final DateTime? startedAt;

  /// When the check was completed.
  final DateTime? completedAt;
}
