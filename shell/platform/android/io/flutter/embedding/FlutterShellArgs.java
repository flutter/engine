package io.flutter.embedding;

import android.content.Intent;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

@SuppressWarnings({"WeakerAccess", "unused"})
public class FlutterShellArgs {
    public static final String ARG_KEY_TRACE_STARTUP = "trace-startup";
    public static final String ARG_TRACE_STARTUP = "--trace-startup";
    public static final String ARG_KEY_START_PAUSED = "start-paused";
    public static final String ARG_START_PAUSED = "--start-paused";
    public static final String ARG_KEY_USE_TEST_FONTS = "use-test-fonts";
    public static final String ARG_USE_TEST_FONTS = "--use-test-fonts";
    public static final String ARG_KEY_ENABLE_DART_PROFILING = "enable-dart-profiling";
    public static final String ARG_ENABLE_DART_PROFILING = "--enable-dart-profiling";
    public static final String ARG_KEY_ENABLE_SOFTWARE_RENDERING = "enable-software-rendering";
    public static final String ARG_ENABLE_SOFTWARE_RENDERING = "--enable-software-rendering";
    public static final String ARG_KEY_SKIA_DETERMINISTIC_RENDERING = "skia-deterministic-rendering";
    public static final String ARG_SKIA_DETERMINISTIC_RENDERING = "--skia-deterministic-rendering";
    public static final String ARG_KEY_TRACE_SKIA = "trace-skia";
    public static final String ARG_TRACE_SKIA = "--trace-skia";
    public static final String ARG_KEY_VERBOSE_LOGGING = "verbose-logging";
    public static final String ARG_VERBOSE_LOGGING = "--verbose-logging";

    public static FlutterShellArgs fromIntent(Intent intent) {
        // Before adding more entries to this list, consider that arbitrary
        // Android applications can generate intents with extra data and that
        // there are many security-sensitive args in the binary.
        ArrayList<String> args = new ArrayList<>();

        if (intent.getBooleanExtra(ARG_KEY_TRACE_STARTUP, false)) {
            args.add(ARG_TRACE_STARTUP);
        }
        if (intent.getBooleanExtra(ARG_KEY_START_PAUSED, false)) {
            args.add(ARG_START_PAUSED);
        }
        if (intent.getBooleanExtra(ARG_KEY_USE_TEST_FONTS, false)) {
            args.add(ARG_USE_TEST_FONTS);
        }
        if (intent.getBooleanExtra(ARG_KEY_ENABLE_DART_PROFILING, false)) {
            args.add(ARG_ENABLE_DART_PROFILING);
        }
        if (intent.getBooleanExtra(ARG_KEY_ENABLE_SOFTWARE_RENDERING, false)) {
            args.add(ARG_ENABLE_SOFTWARE_RENDERING);
        }
        if (intent.getBooleanExtra(ARG_KEY_SKIA_DETERMINISTIC_RENDERING, false)) {
            args.add(ARG_SKIA_DETERMINISTIC_RENDERING);
        }
        if (intent.getBooleanExtra(ARG_KEY_TRACE_SKIA, false)) {
            args.add(ARG_TRACE_SKIA);
        }
        if (intent.getBooleanExtra(ARG_KEY_VERBOSE_LOGGING, false)) {
            args.add(ARG_VERBOSE_LOGGING);
        }

        return new FlutterShellArgs(args);
    }

    private List<String> args;

    FlutterShellArgs(String[] args) {
        this.args = Arrays.asList(args);
    }

    FlutterShellArgs(List<String> args) {
        this.args = args;
    }

    public void add(String arg) {
        args.add(arg);
    }

    public String[] toArray() {
        String[] argsArray = new String[args.size()];
        return args.toArray(argsArray);
    }
}
