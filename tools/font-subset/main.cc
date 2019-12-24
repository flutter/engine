// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <hb-subset.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <set>

#include "hb_wrappers.h"

hb_codepoint_t ParseCodepoint(const char* arg) {
  unsigned long value = 0;
  // Check for \u123, u123, otherwise let strtoul work it out.
  if (arg[0] == 'u') {
    value = strtoul(arg + 1, nullptr, 16);
  } else {
    value = strtoul(arg, nullptr, 0);
  }

  if (value == 0 || value > std::numeric_limits<hb_codepoint_t>::max()) {
    std::cerr << "The value '" << arg
              << "' could not be parsed as a valid unicode codepoint; ignoring."
              << std::endl;
    return 0;
  }
  return value;
}

void Usage() {
  std::cout << "Usage:" << std::endl;
  std::cout << "font-subset <output.ttf> <input.ttf> [CODEPOINTS]" << std::endl;
  std::cout << std::endl;
  std::cout << "The output.ttf file will be overwritten if it exists already "
               "and the subsetting operation succeeds."
            << std::endl;
  std::cout << "At least one code point must be specified. The code points "
               "should be separated by spaces, and must be input as decimal "
               "numbers (123), hexidecimal numbers (0x7B), or unicode "
               "hexidecimal characters (\\u7B)."
            << std::endl;
  std::cout
      << "This program will de-duplicate codepoints if the same codepoint is "
         "specified multiple times, e.g. '123 123' will be treated as '123'."
      << std::endl;
}

int main(int argc, char** argv) {
  if (argc <= 3) {
    Usage();
    return -1;
  }
  std::string output_file_path(argv[1]);
  std::string input_file_path(argv[2]);
  std::cout << "Using output file: " << output_file_path << std::endl;
  std::cout << "Using source file: " << input_file_path << std::endl;

  HarfbuzzWrappers::HbBlobPtr font_blob(
      hb_blob_create_from_file(input_file_path.c_str()));
  if (!hb_blob_get_length(font_blob.get())) {
    std::cerr << "Failed to load input font " << input_file_path
              << "; aborting." << std::endl;
    return -1;
  }

  HarfbuzzWrappers::HbFacePtr font_face(hb_face_create(font_blob.get(), 0));
  if (font_face.get() == hb_face_get_empty()) {
    std::cerr << "Failed to load input font face " << input_file_path
              << "; aborting." << std::endl;
    return -1;
  }

  HarfbuzzWrappers::HbSubsetInputPtr input(hb_subset_input_create_or_fail());
  {
    hb_set_t* desired_codepoints = hb_subset_input_unicode_set(input.get());
    HarfbuzzWrappers::HbSetPtr actual_codepoints(hb_set_create());
    hb_face_collect_unicodes(font_face.get(), actual_codepoints.get());
    for (int i = 3; i < argc; i++) {
      auto codepoint = ParseCodepoint(argv[i]);
      if (codepoint) {
        if (!hb_set_has(actual_codepoints.get(), codepoint)) {
          std::cerr << "Codepoint " << argv[i]
                    << " not found in font, aborting." << std::endl;
          return -1;
        }
        hb_set_add(desired_codepoints, codepoint);
      }
    }
  }
  HarfbuzzWrappers::HbFacePtr new_face(hb_subset(font_face.get(), input.get()));

  if (new_face.get() == hb_face_get_empty()) {
    std::cerr << "Failed to subset font; aborting." << std::endl;
    return -1;
  }

  HarfbuzzWrappers::HbBlobPtr result(hb_face_reference_blob(new_face.get()));
  if (!hb_blob_get_length(result.get())) {
    std::cerr << "Failed get new font bytes; aborting" << std::endl;
    return -1;
  }

  unsigned int data_length;
  const char* data = hb_blob_get_data(result.get(), &data_length);

  std::ofstream output_font_file;
  output_font_file.open(output_file_path,
                        std::ios::out | std::ios::trunc | std::ios::binary);
  output_font_file.write(data, data_length);
  output_font_file.close();

  std::cout << "Wrote " << data_length << " bytes to " << output_file_path
            << std::endl;
  return 0;
}
