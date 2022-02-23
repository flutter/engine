// Copyright 2022 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fuchsia_pkg_url.h"

// #include <re2/re2.h>
#include <regex>
#include <string>
#include <string_view>

namespace component {

// Subsitutue functions copied from fxl.
static std::string SubstituteWithArray(std::string_view format,
                                       std::string_view* args,
                                       size_t nargs);

std::string Substitute(std::string_view format, std::string_view arg0) {
  std::array<std::string_view, 1> arr = {arg0};
  return SubstituteWithArray(format, arr.data(), arr.size());
}

std::string Substitute(std::string_view format,
                       std::string_view arg0,
                       std::string_view arg1) {
  std::array<std::string_view, 2> arr = {arg0, arg1};
  return SubstituteWithArray(format, arr.data(), arr.size());
}

std::string Substitute(std::string_view format,
                       std::string_view arg0,
                       std::string_view arg1,
                       std::string_view arg2) {
  std::array<std::string_view, 3> arr = {arg0, arg1, arg2};
  return SubstituteWithArray(format, arr.data(), arr.size());
}

static const std::string kFuchsiaPkgPrefix = "fuchsia-pkg://";

// kFuchsiaPkgRexp has the following group matches:
// 1: user/domain/port/etc (everything after scheme, before path)
// 2: package name
// 3: package variant
// 4: package merkle-root hash
// 5: resource path
static const std::regex* kFuchsiaPkgRexp = std::regex(
    "^fuchsia-pkg://([^/]+)/([^/#?]+)(?:/([^/"
    "#?]+))?(?:\\?hash=([^&#]+))?(?:#(.+))?$");

// static
bool FuchsiaPkgUrl::IsFuchsiaPkgScheme(const std::string& url) {
  return url.compare(0, kFuchsiaPkgPrefix.length(), kFuchsiaPkgPrefix) == 0;
}

std::string FuchsiaPkgUrl::GetDefaultComponentCmxPath() const {
  return fxl::Substitute("meta/$0.cmx", package_name());
}

bool FuchsiaPkgUrl::Parse(const std::string& url) {
  package_name_.clear();
  resource_path_.clear();

  // if (!re2::RE2::FullMatch(url, *kFuchsiaPkgRexp, &host_name_,
  // &package_name_,
  //                          &variant_, &hash_, &resource_path_)) {
  //   return false;
  // }

  char* cstr = new char[url.size()];
  std::copy(url.begin(), url.end(), cstr);

  std::cmatch cm;
  bool full_match = std::regex_match(cstr, cm, kFuchsiaPkgRexp);
  if (!full_match) {
    return false;
  }

  host_name_ = cm[1];
  package_name_ = cm[2];
  variant_ = cm[3];
  hash_ = cm[4];
  resource_path_ = cm[5];

  url_ = url;

  if (variant_.empty()) {
    // TODO(fxbug.dev/4002): Currently this defaults to "0" if not present, but
    // variant will eventually be required in fuchsia-pkg URLs.
    variant_ = "0";
  }

  return true;
}

bool FuchsiaPkgUrl::operator==(const FuchsiaPkgUrl& rhs) const {
  return (this->host_name() == rhs.host_name() &&
          this->package_name() == rhs.package_name() &&
          this->variant() == rhs.variant() &&
          this->resource_path() == rhs.resource_path() &&
          this->hash() == rhs.hash());
}

std::string FuchsiaPkgUrl::pkgfs_dir_path() const {
  return fxl::Substitute("/pkgfs/packages/$0/$1", package_name_, variant_);
}

std::string FuchsiaPkgUrl::package_path() const {
  std::string query = "";
  if (!hash_.empty()) {
    query = fxl::Substitute("?hash=$0", hash_);
  }

  return fxl::Substitute("fuchsia-pkg://$0/$1/$2$3", host_name_, package_name_,
                         variant_, query);
}

const std::string& FuchsiaPkgUrl::ToString() const {
  return url_;
}

}  // namespace component
