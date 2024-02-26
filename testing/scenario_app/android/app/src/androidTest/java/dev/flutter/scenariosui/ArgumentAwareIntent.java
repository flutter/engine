// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package dev.flutter.scenariosui;

import android.content.Intent;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.test.InstrumentationRegistry;
import io.flutter.embedding.engine.FlutterShellArgs;
import org.junit.rules.TestRule;
import org.junit.runner.Description;
import org.junit.runners.model.Statement;

public class ArgumentAwareIntent implements TestRule {
  public ArgumentAwareIntent() {
    this.intent = new Intent(Intent.ACTION_MAIN);
  }

  public ArgumentAwareIntent(@NonNull Intent intent) {
    this.intent = intent;
  }

  private @NonNull Intent intent;

  public @NonNull Intent getIntent() {
    return intent;
  }

  @Override
  public @NonNull Statement apply(@NonNull Statement base, @NonNull Description description) {
    return new Statement() {
      @Override
      public void evaluate() throws Throwable {
        Bundle arguments = InstrumentationRegistry.getArguments();
        if ("true".equals(arguments.getString(FlutterShellArgs.ARG_KEY_ENABLE_IMPELLER))) {
          getIntent().putExtra(FlutterShellArgs.ARG_KEY_ENABLE_IMPELLER, true);
          String backend = arguments.getString("impeller-backend");
          if (backend != null) {
            getIntent().putExtra("impeller-backend", backend);
          }
        }
        base.evaluate();
      }
    };
  }
}
