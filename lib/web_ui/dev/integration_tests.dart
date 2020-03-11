
class IntegrationTestsManager {

  final String _browser;
  IntegrationTestsManager(this._browser);

  // (DONE) TODO: update the paths on environment.dart.
  // TODO: Go to integration tests directory and get a list of tests to run.

  // (DONE) TODO: Check the browser version. Give warning if not chrome.
  // TODO: Install and run driver for chrome.
  // TODO: Run the tests one by one.
  // TODO: For each run print out success fail. Also print out how to rerun.
  void runTests() {
    print('run testssss: $_browser');
    if(_browser != 'chrome') {
      print('WARNING: integration tests are only suppoted on chrome for now');
      return;
    } else {
      print('run tests IntegrationTestsManager');
    }
  }
}
