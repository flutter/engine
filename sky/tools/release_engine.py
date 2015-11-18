#!/usr/bin/env python
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# See https://github.com/domokit/sky_engine/wiki/Release-process

import argparse
import os
import subprocess
import sys
import distutils.util


DRY_RUN = False


def run(cwd, args):
    print 'RUNNING:', ' '.join(args), 'IN:', cwd
    if DRY_RUN:
        return
    subprocess.check_call(args, cwd=cwd)


def confirm(prompt):
    user_input = raw_input("%s (y/N) " % prompt)
    try:
        return distutils.util.strtobool(user_input) == 1
    except ValueError:
        return False


def git_revision(cwd):
    return subprocess.check_output([
        'git', 'rev-parse', 'HEAD',
    ], cwd=cwd).strip()


class Artifact(object):
    def __init__(self, category, path):
        self.category = category
        self.path = path


GS_URL = 'gs://mojo/sky/%(category)s/%(config)s/%(commit_hash)s/%(name)s'


ARTIFACTS = {
    'android-arm': [
        Artifact('shell', 'chromium-debug.keystore'),
        Artifact('shell', 'icudtl.dat'),
        Artifact('shell', 'dist/shell/SkyShell.apk'),
        Artifact('shell', 'dist/shell/flutter.mojo'),
        Artifact('shell', 'dist/shell/libflutter_library.so'),
        Artifact('shell', 'gen/sky/shell/shell/classes.dex'),
        Artifact('shell', 'gen/sky/shell/shell/shell/libs/armeabi-v7a/libsky_shell.so'),
    ],
    'linux-x64': [
        Artifact('shell', 'dist/shell/icudtl.dat'),
        Artifact('shell', 'dist/shell/sky_shell'),
        Artifact('shell', 'dist/shell/sky_snapshot'),
        Artifact('shell', 'dist/shell/flutter.mojo'),
        Artifact('shell', 'dist/shell/libflutter_library.so'),
    ]
}


def find_missing_artifacts(config, config_root):
    result = []
    for artifact in ARTIFACTS[config]:
        artifact_path = os.path.join(config_root, artifact.path)
        if not os.path.exists(artifact_path):
            result.append(artifact_path)
    return result


def upload_artifacts(dist_root, config, commit_hash):
    for artifact in ARTIFACTS[config]:
        dst = GS_URL % {
            'category': artifact.category,
            'config': config,
            'commit_hash': commit_hash,
            'name': os.path.basename(artifact.path),
        }
        z = ','.join([ 'mojo', 'dat', 'so', 'dex' ])
        run(dist_root, ['gsutil', 'cp', '-z', z, artifact.path, dst])


def main():
    parser = argparse.ArgumentParser(description='Deploy!')
    parser.add_argument('--dry-run', action='store_true', default=False,
        help='Just print commands w/o executing.')
    parser.add_argument('--revision', help='The git revision to publish.')
    args = parser.parse_args()

    global DRY_RUN
    DRY_RUN = args.dry_run

    engine_root = os.path.abspath('.')
    if not os.path.exists(os.path.join(engine_root, 'sky')):
        print "Cannot find //sky. Is %s the Flutter engine repository?" % engine_root
        return 1

    commit_hash = git_revision(engine_root)

    if commit_hash != args.revision:
        print "Current revision %s does not match requested revision %s." % (commit_hash, args.revision)
        print "Please update the current revision to %s." % args.revision
        return 1

    # Derived paths:
    dart_sdk_root = os.path.join(engine_root, 'third_party/dart-sdk/dart-sdk')
    pub_path = os.path.join(dart_sdk_root, 'bin/pub')
    android_out_root = os.path.join(engine_root, 'out/android_Release')
    linux_out_root = os.path.join(engine_root, 'out/Release')
    sky_engine_package_root = os.path.join(android_out_root, 'dist/packages/sky_engine/sky_engine')
    sky_services_package_root = os.path.join(android_out_root, 'dist/packages/sky_services/sky_services')
    sky_engine_revision_file = os.path.join(sky_engine_package_root, 'lib', 'REVISION')

    run(engine_root, ['sky/tools/gn', '--android', '--release'])
    run(engine_root, ['ninja', '-C', 'out/android_Release', ':dist'])

    run(engine_root, ['sky/tools/gn', '--release'])
    run(engine_root, ['ninja', '-C', 'out/Release', ':dist'])

    with open(sky_engine_revision_file, 'w') as stream:
        stream.write(commit_hash)

    configs = [('android-arm', android_out_root),
               ('linux-x64', linux_out_root)]

    missing_artifacts = []
    for config, config_root in configs:
        missing_artifacts.extend(find_missing_artifacts(config, config_root))
    if missing_artifacts:
        print ('Build is missing files:\n%s' %
               '\n'.join('\t%s' % path for path in missing_artifacts))
        return 1

    for config, config_root in configs:
        upload_artifacts(config_root, config, commit_hash)

    run(sky_engine_package_root, [pub_path, 'publish', '--force'])
    run(sky_services_package_root, [pub_path, 'publish', '--force'])


if __name__ == '__main__':
    sys.exit(main())
