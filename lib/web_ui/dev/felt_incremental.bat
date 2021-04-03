:: felt_windows: a command-line utility for Windows for building and testing
:: Flutter web engine.
:: FELT stands for Flutter Engine Local Tester.

@ECHO OFF
SETLOCAL

FOR /F "tokens=1-2 delims=:" %%a in ('where gclient') DO SET GCLIENT_PATH=%%b
IF %GCLIENT_PATH%==[] (ECHO "ERROR: gclient is not in your PATH")

FOR /F "tokens=1-2 delims=:" %%a in ('where ninja') DO SET NINJA_PATH=%%b
IF %NINJA_PATH%==[] (ECHO "ERROR: ninja is not in your PATH")

:: Starting from this script's path, walk up to engine source directory.
SET SCRIPT_DIR=%~dp0
FOR %%a IN ("%SCRIPT_DIR:~0,-1%") DO SET TMP=%%~dpa
FOR %%a IN ("%TMP:~0,-1%") DO SET TMP=%%~dpa
FOR %%a IN ("%TMP:~0,-1%") DO SET TMP=%%~dpa
FOR %%a IN ("%TMP:~0,-1%") DO SET ENGINE_SRC_DIR=%%~dpa

SET ENGINE_SRC_DIR=%ENGINE_SRC_DIR:~0,-1%
SET OUT_DIR=%ENGINE_SRC_DIR%\out
SET HOST_DEBUG_UNOPT_DIR=%OUT_DIR%\host_debug_unopt
SET DART_SDK_DIR=%HOST_DEBUG_UNOPT_DIR%\dart-sdk
SET DART_BIN=%DART_SDK_DIR%\bin\dart
SET PUB_BIN=%DART_SDK_DIR%\bin\pub
SET FLUTTER_DIR=%ENGINE_SRC_DIR%\flutter
SET WEB_UI_DIR=%FLUTTER_DIR%\lib\web_ui
SET DEV_DIR=%WEB_UI_DIR%\dev
SET FELT_PATH=%DEV_DIR%\felt.dart
SET GN=%FLUTTER_DIR%\tools\gn
SET DART_TOOL_DIR=%WEB_UI_DIR%\.dart_tool
SET STAMP_PATH=%DART_TOOL_DIR%\felt.snapshot.stamp
SET SNAPSHOT_PATH=%DART_TOOL_DIR%\felt.snapshot

:: Set revision from using git in Flutter directory.
CD %FLUTTER_DIR%
FOR /F "tokens=1 delims=:" %%a in ('git rev-parse HEAD') DO SET REVISION=%%a

:: Uncomment for debugging the values.
:: ECHO DEV_DIR = %DEV_DIR%
:: ECHO WEB_UI_DIR = %WEB_UI_DIR%
:: ECHO FLUTTER_DIR = %FLUTTER_DIR%
:: ECHO ENGINE_SRC_DIR = %ENGINE_SRC_DIR%
:: ECHO OUT_DIR = %OUT_DIR%
:: ECHO HOST_DEBUG_UNOPT_DIR = %HOST_DEBUG_UNOPT_DIR%
:: ECHO DART_SDK_DIR = %DART_SDK_DIR%
:: ECHO PUB_BIN = %PUB_BIN%
:: ECHO FELT_PATH = %FELT_PATH%
:: ECHO GN = %GN%
:: ECHO DART_TOOL_DIR = %DART_TOOL_DIR%
:: ECHO STAMP_PATH = %STAMP_PATH%
:: ECHO SNAPSHOT_PATH = %SNAPSHOT_PATH%
:: ECHO REVISION = %REVISION%
:: ECHO DART_BIN = %DART_BIN%

cd %WEB_UI_DIR%
IF NOT EXIST "%SNAPSHOT_PATH%" (
  ECHO Precompiling felt snapshot
  CALL %PUB_BIN% get
  %DART_BIN% --snapshot="%SNAPSHOT_PATH%" --packages="%WEB_UI_DIR%\.packages" %FELT_PATH%
)

IF %1==test (
  %DART_SDK_DIR%\bin\dart --packages="%WEB_UI_DIR%\.packages" "%SNAPSHOT_PATH%" %* --browser=chrome
) ELSE (
  %DART_SDK_DIR%\bin\dart --packages="%WEB_UI_DIR%\.packages" "%SNAPSHOT_PATH%" %*
)

EXIT /B %ERRORLEVEL%
