// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_ACCESSIBILITY_ALERT_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_ACCESSIBILITY_ALERT_H_

#include <atlbase.h>
#include <atlcom.h>
#include <oleacc.h>

namespace flutter {

class AccessibilityAlert : public CComObjectRootEx<CComMultiThreadModel>,
                           public IAccessible {
 public:
  BEGIN_COM_MAP(AccessibilityAlert)
  COM_INTERFACE_ENTRY(IAccessible)
  END_COM_MAP()

};

}

#endif
