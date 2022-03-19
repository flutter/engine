# Usage: create_pkg_manifest.py --deps <DEPS file> --output <jiri manifest>
#
# This script parses the DEPS file, extracts dependencies that live under
# third_party/pkg, and writes them to a file suitable for consumption as a
# jiri manifest for Fuchsia. It is assumed that the Dart tree is under
# //dart in the Fuchsia world, and so the dependencies extracted by this script
# will go under //dart/third_party/pkg.

import argparse
import os
import sys

SCRIPT_DIR = os.path.dirname(sys.argv[0])
DART_ROOT = os.path.realpath(os.path.join(SCRIPT_DIR, '..'))


# Used in parsing the DEPS file.
class VarImpl(object):
    _env_vars = {
        "host_cpu": "x64",
        "host_os": "linux",
    }

    def __init__(self, local_scope):
        self._local_scope = local_scope

    def Lookup(self, var_name):
        """Implements the Var syntax."""
        if var_name in self._local_scope.get("vars", {}):
            return self._local_scope["vars"][var_name]
        # Inject default values for env variables
        if var_name in self._env_vars:
            return self._env_vars[var_name]
        raise Exception("Var is not defined: %s" % var_name)


def ParseDepsFile(deps_file):
    local_scope = {}
    var = VarImpl(local_scope)
    global_scope = {
        'Var': var.Lookup,
        'deps_os': {},
    }
    # Read the content.
    with open(deps_file, 'r') as fp:
        deps_content = fp.read()

    # Eval the content.
    exec (deps_content, global_scope, local_scope)

    # Extract the deps and filter.
    deps = local_scope.get('deps', {})
    filtered_deps = []
    for k, v in deps.items():
        # We currently do not support packages or cipd which are represented
        # as dictionaries.
        if isinstance(v, str):
            filtered_deps.append(v)

    return filtered_deps


def WriteManifest(deps, manifest_file):
    print('\n'.join(sorted(deps)))
    with open(manifest_file, 'w') as manifest:
        manifest.write('\n'.join(sorted(deps)))


def ParseArgs(args):
    args = args[1:]
    parser = argparse.ArgumentParser(
        description='A script to generate a jiri manifest for third_party/pkg.')

    parser.add_argument(
        '--deps',
        '-d',
        type=str,
        help='Input DEPS file.',
        default=os.path.join(DART_ROOT, 'DEPS'))
    parser.add_argument(
        '--output',
        '-o',
        type=str,
        help='Output jiri manifest.',
        default=os.path.join(DART_ROOT, 'deps_flatten.txt'))

    return parser.parse_args(args)


def Main(argv):
    args = ParseArgs(argv)
    deps = ParseDepsFile(args.deps)
    WriteManifest(deps, args.output)
    return 0


if __name__ == '__main__':
    sys.exit(Main(sys.argv))
