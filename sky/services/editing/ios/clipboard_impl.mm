// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/services/editing/ios/clipboard_impl.h"

#include "base/logging.h"
#include "base/strings/sys_string_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include <UIKit/UIKit.h>
#include <unicode/utf16.h>

namespace sky {
namespace services {
namespace editing {

ClipboardImpl::ClipboardImpl(
    mojo::InterfaceRequest<::editing::Clipboard> request)
    : binding_(this, request.Pass()) {}

ClipboardImpl::~ClipboardImpl() {
}

void ClipboardImpl::SetClipData(::editing::ClipDataPtr clip) {
  UIPasteboard* pasteboard = [UIPasteboard generalPasteboard];
  pasteboard.string = [NSString stringWithUTF8String:clip->text.data()];
}

void ClipboardImpl::GetClipData(
    const ::editing::Clipboard::GetClipDataCallback& callback) {
  ::editing::ClipDataPtr clip;

  UIPasteboard* pasteboard = [UIPasteboard generalPasteboard];
  NSString* str = pasteboard.string;
  if (str) {
    clip = ::editing::ClipData::New();
    clip->text = str.UTF8String;
  }

  callback.Run(clip.Pass());
}

void ClipboardFactory::Create(
    mojo::ApplicationConnection* connection,
    mojo::InterfaceRequest<::editing::Clipboard> request) {
  new ClipboardImpl(request.Pass());
}

}  // namespace editing
}  // namespace services
}  // namespace sky
