#include "parse_url.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <string>

namespace dart_runner {

std::string GetLabelFromUrl(const std::string& url) {
  for (size_t i = url.length() - 1; i > 0; i--) {
    if (url[i] == '/') {
      return url.substr(i + 1, url.length() - 1);
    }
  }
  return url;
}

// Find the name of the component.
// fuchsia-pkg://fuchsia.com/hello_dart#meta/hello_dart.cm -> hello_dart
std::string GetComponentNameFromUrl(const std::string& url) {
  const std::string label = GetLabelFromUrl(url);
  for (size_t i = 0; i < label.length(); ++i) {
    if (label[i] == '.') {
      return label.substr(0, i);
    }
  }
  return label;
}

}  // namespace dart_runner
