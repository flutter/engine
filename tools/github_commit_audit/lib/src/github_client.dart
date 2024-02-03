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
      },
    );
    final List<Object> json = (_fetchJson(url) as List<Object>).cast<Object>();
    for (final Object pr in json) {
      yield PullRequest.fromJson(pr as Map<String, Object?>);
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
      labels: (json['labels']! as List<Object?>).cast<String>().toSet(),
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
