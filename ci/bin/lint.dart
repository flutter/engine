/// Runs clang-tidy on files with changes.
///
/// usage:
/// dart lint.dart <path to compile_commands.json> <path to git repository> [clang-tidy checks]
///
/// User environment variable FLUTTER_LINT_ALL to run on all files.

import 'dart:async' show Completer;
import 'dart:convert' show jsonDecode, utf8, LineSplitter;
import 'dart:io'
    show
        File,
        Process,
        ProcessResult,
        ProcessException,
        exit,
        Directory,
        FileSystemEntity,
        Platform,
        stderr,
        stdout;

import 'package:args/args.dart';
import 'package:path/path.dart' as path;

Platform defaultPlatform = Platform();

/// Exception class for when a process fails to run, so we can catch
/// it and provide something more readable than a stack trace.
class ProcessRunnerException implements Exception {
  ProcessRunnerException(this.message, {this.result});

  final String message;
  final ProcessResult result;

  int get exitCode => result?.exitCode ?? -1;

  @override
  String toString() {
    String output = runtimeType.toString();
    output += ': $message';
    final String stderr = (result?.stderr ?? '') as String;
    if (stderr.isNotEmpty) {
      output += ':\n$stderr';
    }
    return output;
  }
}

class ProcessRunnerResult {
  const ProcessRunnerResult(this.exitCode, this.stdout, this.stderr, this.output);
  final int exitCode;
  final List<int> stdout;
  final List<int> stderr;
  final List<int> output;
}

/// A helper class for classes that want to run a process, optionally have the
/// stderr and stdout reported as the process runs, and capture the stdout
/// properly without dropping any.
class ProcessRunner {
  ProcessRunner({
    this.defaultWorkingDirectory,
  });

  /// Sets the default directory used when `workingDirectory` is not specified
  /// to [runProcess].
  final Directory defaultWorkingDirectory;

  /// The environment to run processes with.
  Map<String, String> environment = Map<String, String>.from(Platform.environment);

  /// Run the command and arguments in `commandLine` as a sub-process from
  /// `workingDirectory` if set, or the [defaultWorkingDirectory] if not. Uses
  /// [Directory.current] if [defaultWorkingDirectory] is not set.
  ///
  /// Set `failOk` if [runProcess] should not throw an exception when the
  /// command completes with a a non-zero exit code.
  Future<ProcessRunnerResult> runProcess(
    List<String> commandLine, {
    Directory workingDirectory,
    bool printOutput = false,
    bool failOk = false,
    Stream<List<int>> stdin,
  }) async {
    workingDirectory ??= defaultWorkingDirectory ?? Directory.current;
    if (printOutput) {
      stderr.write('Running "${commandLine.join(' ')}" in ${workingDirectory.path}.\n');
    }
    final List<int> stdoutOutput = <int>[];
    final List<int> stderrOutput = <int>[];
    final List<int> combinedOutput = <int>[];
    final Completer<void> stdoutComplete = Completer<void>();
    final Completer<void> stderrComplete = Completer<void>();
    final Completer<void> stdinComplete = Completer<void>();

    Process process;
    Future<int> allComplete() async {
      if (stdin != null) {
        await stdinComplete.future;
        await process?.stdin?.close();
      }
      await stderrComplete.future;
      await stdoutComplete.future;
      return process?.exitCode ?? Future<int>.value(0);
    }

    try {
      process = await Process.start(
        commandLine.first,
        commandLine.sublist(1),
        workingDirectory: workingDirectory.absolute.path,
        environment: environment,
        runInShell: false,
      );
      if (stdin != null) {
        stdin.listen((List<int> data) {
          process?.stdin?.add(data);
        }, onDone: () async => stdinComplete.complete());
      }
      process.stdout.listen(
        (List<int> event) {
          stdoutOutput.addAll(event);
          combinedOutput.addAll(event);
          if (printOutput) {
            stdout.add(event);
          }
        },
        onDone: () async => stdoutComplete.complete(),
      );
      process.stderr.listen(
        (List<int> event) {
          stderrOutput.addAll(event);
          combinedOutput.addAll(event);
          if (printOutput) {
            stderr.add(event);
          }
        },
        onDone: () async => stderrComplete.complete(),
      );
    } on ProcessException catch (e) {
      final String message = 'Running "${commandLine.join(' ')}" in ${workingDirectory.path} '
          'failed with:\n${e.toString()}';
      throw ProcessRunnerException(message);
    } on ArgumentError catch (e) {
      final String message = 'Running "${commandLine.join(' ')}" in ${workingDirectory.path} '
          'failed with:\n${e.toString()}';
      throw ProcessRunnerException(message);
    }

    final int exitCode = await allComplete();
    if (exitCode != 0 && !failOk) {
      final String message =
          'Running "${commandLine.join(' ')}" in ${workingDirectory.path} failed';
      throw ProcessRunnerException(
        message,
        result: ProcessResult(
            0, exitCode, null, 'exited with code $exitCode\n${utf8.decode(combinedOutput)}'),
      );
    }
    return ProcessRunnerResult(exitCode, stdoutOutput, stderrOutput, combinedOutput);
  }
}

class WorkerJob {
  WorkerJob(
    this.name,
    this.args, {
    this.workingDirectory,
    this.printOutput = false,
  });

  /// The name of the job.
  final String name;

  /// The arguments for the process, including the command name as args[0].
  final List<String> args;

  /// The working directory that the command should be executed in.
  final Directory workingDirectory;

  /// Whether or not this command should print it's stdout when it runs.
  final bool printOutput;

  @override
  String toString() {
    return args.join(' ');
  }
}

/// A pool of worker processes that will keep [numWorkers] busy until all of the
/// (presumably single-threaded) processes are finished.
class ProcessPool {
  ProcessPool({int numWorkers}) : numWorkers = numWorkers ?? Platform.numberOfProcessors;

  ProcessRunner processRunner = ProcessRunner();
  int numWorkers;
  List<WorkerJob> pendingJobs = <WorkerJob>[];
  List<WorkerJob> failedJobs = <WorkerJob>[];
  Map<WorkerJob, Future<List<int>>> inProgressJobs = <WorkerJob, Future<List<int>>>{};
  Map<WorkerJob, ProcessRunnerResult> completedJobs = <WorkerJob, ProcessRunnerResult>{};
  Completer<Map<WorkerJob, ProcessRunnerResult>> completer =
      Completer<Map<WorkerJob, ProcessRunnerResult>>();

  void _printReport() {
    final int totalJobs = completedJobs.length + inProgressJobs.length + pendingJobs.length;
    final String percent =
        totalJobs == 0 ? '100' : ((100 * completedJobs.length) ~/ totalJobs).toString().padLeft(3);
    final String completed = completedJobs.length.toString().padLeft(3);
    final String total = totalJobs.toString().padRight(3);
    final String inProgress = inProgressJobs.length.toString().padLeft(2);
    final String pending = pendingJobs.length.toString().padLeft(3);
    stdout.write('Jobs: $percent% done, $completed/$total completed, $inProgress in '
        'progress, $pending pending.  \r');
  }

  Future<List<int>> _scheduleJob(WorkerJob job) async {
    final Completer<List<int>> jobDone = Completer<List<int>>();
    final List<int> output = <int>[];
    try {
      completedJobs[job] = await processRunner.runProcess(
        job.args,
        workingDirectory: job.workingDirectory,
        printOutput: job.printOutput,
      );
    } catch (e) {
      failedJobs.add(job);
      if (e is ProcessRunnerException) {
        print(e.toString());
        print('${utf8.decode(output)}');
      } else {
        print('\nJob $job failed: $e');
      }
    } finally {
      inProgressJobs.remove(job);
      if (pendingJobs.isNotEmpty) {
        final WorkerJob newJob = pendingJobs.removeAt(0);
        inProgressJobs[newJob] = _scheduleJob(newJob);
      } else {
        if (inProgressJobs.isEmpty) {
          completer.complete(completedJobs);
        }
      }
      jobDone.complete(output);
      _printReport();
    }
    return jobDone.future;
  }

  Future<Map<WorkerJob, ProcessRunnerResult>> startWorkers(List<WorkerJob> jobs) async {
    assert(inProgressJobs.isEmpty);
    assert(failedJobs.isEmpty);
    assert(completedJobs.isEmpty);
    if (jobs.isEmpty) {
      return <WorkerJob, ProcessRunnerResult>{};
    }
    pendingJobs = jobs;
    for (int i = 0; i < numWorkers; ++i) {
      if (pendingJobs.isEmpty) {
        break;
      }
      final WorkerJob job = pendingJobs.removeAt(0);
      inProgressJobs[job] = _scheduleJob(job);
    }
    return completer.future.then((Map<WorkerJob, ProcessRunnerResult> result) {
      stdout.flush();
      return result;
    });
  }
}

String _linterOutputHeader = '''
┌──────────────────────────┐
│ Engine Clang Tidy Linter │
└──────────────────────────┘
The following errors have been reported by the Engine Clang Tidy Linter.  For
more information on addressing these issues please see:
https://github.com/flutter/flutter/wiki/Engine-Clang-Tidy-Linter
''';

class Command {
  Directory directory = Directory('');
  String command = '';
  File file = File('');
}

Command parseCommand(Map<String, dynamic> map) {
  final Directory dir = Directory(map['directory'] as String).absolute;
  return Command()
    ..directory = dir
    ..command = map['command'] as String
    ..file = File(path.normalize(path.join(dir.path, map['file'] as String)));
}

String calcTidyArgs(Command command) {
  String result = command.command;
  result = result.replaceAll(RegExp(r'\S*clang/bin/clang'), '');
  result = result.replaceAll(RegExp(r'-MF \S*'), '');
  return result;
}

String calcTidyPath(Command command) {
  final RegExp regex = RegExp(r'\S*clang/bin/clang');
  return regex
          .stringMatch(command.command)
          ?.replaceAll('clang/bin/clang', 'clang/bin/clang-tidy') ??
      '';
}

bool isNonEmptyString(String str) => str.isNotEmpty;

bool containsAny(File file, Iterable<File> queries) {
  return queries.where((File query) => path.equals(query.path, file.path)).isNotEmpty;
}

/// Returns a list of all files with current changes or differ from `master`.
Future<List<File>> getListOfChangedFiles(Directory repoPath) async {
  final ProcessRunner processRunner = ProcessRunner(defaultWorkingDirectory: repoPath);
  String branch = 'upstream/master';
  final ProcessResult fetchResult = Process.runSync('git', <String>['fetch', 'upstream', 'master']);
  if (fetchResult.exitCode != 0) {
    branch = 'origin/master';
    Process.runSync('git', <String>['fetch', 'origin', 'master']);
  }
  final Set<String> result = <String>{};
  final ProcessRunnerResult diffResult = await processRunner.runProcess(
      <String>['git', 'diff', '--name-only', '--diff-filter=ACMRT', if (branch.isNotEmpty) branch]);

  result.addAll(utf8.decode(diffResult.stdout).split('\n').where(isNonEmptyString));
  return result.map<File>((String filePath) => File(path.join(repoPath.path, filePath))).toList();
}

Future<List<File>> dirContents(Directory dir) {
  final List<File> files = <File>[];
  final Completer<List<File>> completer = Completer<List<File>>();
  final Stream<FileSystemEntity> lister = dir.list(recursive: true);
  lister.listen((FileSystemEntity file) => file is File ? files.add(file) : null,
      onError: (Object e) => completer.completeError(e), onDone: () => completer.complete(files));
  return completer.future;
}

File buildFileAsRepoFile(String buildFile, Directory repoPath) {
  // Removes the "../../flutter" from the build files to make it relative to the flutter
  // dir.
  final String relativeBuildFile = path.joinAll(path.split(buildFile).sublist(3));
  final File result = File(path.join(repoPath.absolute.path, relativeBuildFile));
  print('Build file: $buildFile => ${result.path}');
  return result;
}

Future<bool> shouldIgnoreFile(File file) async {
  if (path.split(file.path).contains('third_party')) {
    return true;
  } else {
    final RegExp exp = RegExp(r'//.*FLUTTER_NOLINT');
    await for (String line
        in file.openRead().transform(utf8.decoder).transform(const LineSplitter())) {
      if (exp.hasMatch(line)) {
        return true;
      } else if (line.isNotEmpty && line[0] != '\n' && line[0] != '/') {
        // Quick out once we find a line that isn't empty or a comment.  The
        // FLUTTER_NOLINT must show up before the first real code.
        return false;
      }
    }
    return false;
  }
}

void _usage(ArgParser parser) {
  stderr.writeln('lint.dart [--help] [--lint-all] [--verbose] [--diff-branch]');
  stderr.writeln(parser.usage);
  exit(0);
}

bool verbose = false;

void main(List<String> arguments) async {
  final ArgParser parser = ArgParser();
  parser.addFlag('help', help: 'Print help.');
  parser.addFlag('lint-all',
      help: 'lint all of the sources, regardless of FLUTTER_NOLINT.', defaultsTo: false);
  parser.addFlag('verbose', help: 'Print verbose output.', defaultsTo: verbose);
  parser.addOption('repo', help: 'Use the given path as the repo path');
  parser.addOption('compile-commands',
      help: 'Use the given path as the source of compile_commands.json. This '
          'file is created by running tools/gn');
  parser.addOption('checks',
      help: 'Perform the given checks on the code. Defaults to the empty '
          'string, indicating all checks should be performed.',
      defaultsTo: '');
  final ArgResults options = parser.parse(arguments);

  verbose = options['verbose'] as bool;

  if (options['help'] as bool) {
    _usage(parser);
  }

  final File buildCommandsPath = File(options['compile-commands'] as String);
  if (!buildCommandsPath.existsSync()) {
    stderr.writeln("Build commands path ${buildCommandsPath.absolute.path} doesn't exist.");
    _usage(parser);
  }

  final Directory repoPath = Directory(options['repo'] as String);
  if (!repoPath.existsSync()) {
    stderr.writeln("Repo path ${repoPath.absolute.path} doesn't exist.");
    _usage(parser);
  }

  final String checksArg = options.wasParsed('checks') ? options['checks'] as String : '';
  final String checks = checksArg.isNotEmpty ? '--checks=$checksArg' : '--config=';
  final bool lintAll =
      Platform.environment['FLUTTER_LINT_ALL'] != null || options['lint-all'] as bool;
  final List<File> changedFiles =
      lintAll ? await dirContents(repoPath) : await getListOfChangedFiles(repoPath);

  if (verbose) {
    print('Checking lint in repo at $repoPath.');
    if (checksArg.isNotEmpty) {
      print('Checking for specific checks: $checks.');
    }
    if (lintAll) {
      print('Checking all ${changedFiles.length} files the repo dir.');
    } else {
      print('Dectected ${changedFiles.length} files that have changed');
    }
  }

  final List<dynamic> buildCommandMaps =
      jsonDecode(await buildCommandsPath.readAsString()) as List<dynamic>;
  final List<Command> buildCommands = buildCommandMaps
      .map<Command>((dynamic x) => parseCommand(x as Map<String, dynamic>))
      .toList();
  final Command firstCommand = buildCommands[0];
  final String tidyPath = calcTidyPath(firstCommand);
  assert(tidyPath.isNotEmpty);
  final List<Command> changedFileBuildCommands =
      buildCommands.where((Command x) => containsAny(x.file, changedFiles)).toList();

  if (verbose) {
    print('Found ${changedFileBuildCommands.length} files that have build '
        'commands associated with them and can be lint checked.');
  }

  print(_linterOutputHeader);
  int exitCode = 0;
  final List<WorkerJob> jobs = <WorkerJob>[];
  for (Command command in changedFileBuildCommands) {
    if (!(await shouldIgnoreFile(command.file))) {
      final String tidyArgs = calcTidyArgs(command);
      final List<String> args = <String>[command.file.path, checks, '--'];
      args.addAll(tidyArgs?.split(' ') ?? <String>[]);
      print('🔶 linting ${command.file}');
      jobs.add(WorkerJob(command.file.path, <String>[tidyPath, ...args],
          workingDirectory: command.directory));
    } else {
      print('🔷 ignoring ${command.file}');
    }
  }
  final ProcessPool pool = ProcessPool();
  final Map<WorkerJob, ProcessRunnerResult> results = await pool.startWorkers(jobs);
  print('\n');
  for (final WorkerJob job in results.keys) {
    if (results[job].stdout.isEmpty) {
      continue;
    }
    print('❌ Failures for ${job.name}:');
    print(utf8.decode(results[job].stdout));
    exitCode = 1;
  }
  if (exitCode == 0) {
    print('No lint problems found.');
  }
  exit(exitCode);
}
