import unittest
import scan_flattened_deps

class TestDepsScanMethods(unittest.TestCase):

    def test_common_ancestor_commit(self):
        dep = ['https://chromium.googlesource.com/webm/libwebp.git', '7dfde712a477e420968732161539011e0fd446cf']
        self.assertEqual('fedac6cc69cda3e9e04b780d324cf03921fb3ff4', scan_flattened_deps.getCommonAncestorCommit(dep))

if __name__ == '__main__':
    unittest.main()