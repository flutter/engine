import os
import sys
import unittest
from deps_parser import VarImpl

SCRIPT_DIR = os.path.dirname(sys.argv[0])
CHECKOUT_ROOT = os.path.realpath(os.path.join(SCRIPT_DIR, '..'))
DEPS = os.path.join(CHECKOUT_ROOT, 'DEPS')

class TestDepsParserMethods(unittest.TestCase):

    # extract both mirrored dep names and URLs &
    # upstream names and URLs from DEPs file
    def setUp(self):
        with open(DEPS) as f:
            local_scope_upstream = {}
            global_scope_upstream = {'Var': lambda x: x} # dummy lambda
            # Read the content.
            with open(DEPS, 'r') as f:
                deps_content = f.read()

            # Eval the content.
            exec(deps_content, global_scope_upstream, local_scope_upstream)

            # Extract the upstream URLs
            self.upstream_urls = local_scope_upstream.get('vars').get('upstream_urls')

            local_scope_mirror = {}
            var = VarImpl(local_scope_mirror)
            global_scope_mirror = {
                'Var': var.lookup,
                'deps_os': {},
            }

            # Eval the content.
            exec(deps_content, global_scope_mirror, local_scope_mirror)

            # Extract the deps and filter.
            deps = local_scope_mirror.get('deps', {})
            filtered_deps = []
            for k, v in deps.items():
                # We currently do not support packages or cipd which are represented
                # as dictionaries.
                if isinstance(v, str):
                    filtered_deps.append(v)

            self.deps = filtered_deps

    def test_each_dep_has_upstream_url(self):
        # for each DEP in the deps file, check for an associated upstream URL in deps file
        for dep in self.deps:
            dep_repo = dep.split('@')[0]
            dep_name = dep_repo.split('/')[-1].split('.')[0]
            if dep_name != "vulkan-deps" and dep_name != "khronos":
                # vulkan-deps and khronos do not have one upstream URL
                # all other deps should have an associated upstream URL for vuln scanning purposes
                self.assertTrue(dep_name in self.upstream_urls, msg = dep_name + " not found in upstream URL list")

    def test_each_upstream_url_has_dep(self):
        # for each DEP in the deps file, check for an associated upstream URL in deps file
        for dep in self.upstream_urls:
            self.assertTrue(dep in self.dep)

if __name__ == '__main__':
    unittest.main()