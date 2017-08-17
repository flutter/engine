Frontend Server
==============

Frontend server is simple wrapper around Dart Frontend. It is a Dart application
that compiles Dart source into Dart Kernel binary(.dill-file).

Frontend server runs in two modes:
 - immediate mode, where Dart source file name is provided as command line
 argument;
 - interactive mode, where communication is happening over stdin/stdout.

 Interactive mode instructions
 ---
- ```compile <input.dart>```
 Compiles <input.dart> Dart source file with Dart Frontend, replies with ```result``` response.
- ```recompile <boundary-key>```
```<path/to/updated/file1.dart>```
```<path/to/updated/file2.dart>```
```...```
```<boundary-key>```
 Incrementally recompiles Dart program previously compiled in current session, taking into account changes in the listed files. Replies with ```result``` response.
- ```accept```
 Accepts results of incremental compilation, so that next recompilation request won't recompile previously reported changed files.
- ```reject```
 Rejects results of incremental compilation, so that next recompilation request will recompile previously reported changed files.
- ```quit```
 Stops the server.

Response from the server
---
- ```result <boundary-key>```
```<compiler output>```
```<boundary-key> [<output.dill>]```
Response from the compiler is bracketed by ```<boundary-key>``` tags. If compiler was able to produce a Dart Kernel file, the name of this file ```<output.dill>``` is provided too. If compiler encountered unrecoverable errors, there will be no output file name provided.

