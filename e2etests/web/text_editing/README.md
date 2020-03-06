```
# To run the Text Editing test and use the developer tools in the browser.
flutter run --target=test_driver/text_editing_e2e.dart -d web-server --web-port=8080 --release --local-engine=host_debug_unopt

# To test the Text Editing test with driver:
flutter drive -v --target=test_driver/text_editing_e2e.dart -d web-server --release --browser-name=chrome --local-engine=host_debug_unopt
```
