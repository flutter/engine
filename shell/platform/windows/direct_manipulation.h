// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_DIRECT_MANIPULATION_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_DIRECT_MANIPULATION_H_

#include "flutter/fml/memory/ref_counted.h"

#include <wrl/client.h>
#include "directmanipulation.h"

namespace flutter {

class WindowWin32;
class WindowBindingHandlerDelegate;

class DirectManipulationEventHandler;

class DirectManipulationOwner {
 public:
  explicit DirectManipulationOwner(WindowWin32* window);
  int Init(unsigned int width, unsigned int height);
  void ResizeViewport(unsigned int width, unsigned int height);
  void SetBindingHandlerDelegate(
      WindowBindingHandlerDelegate* binding_handler_delegate);
  void SetContact(UINT contactId);
  void Update();
  void Destroy();
  WindowBindingHandlerDelegate* binding_handler_delegate;

 private:
  WindowWin32* window_;
  DWORD viewportHandlerCookie_;
  Microsoft::WRL::ComPtr<IDirectManipulationManager> manager_;
  Microsoft::WRL::ComPtr<IDirectManipulationUpdateManager> updateManager_;
  Microsoft::WRL::ComPtr<IDirectManipulationViewport> viewport_;
  fml::RefPtr<DirectManipulationEventHandler> handler_;
};

class DirectManipulationEventHandler
    : public fml::RefCountedThreadSafe<DirectManipulationEventHandler>,
      public IDirectManipulationViewportEventHandler,
      public IDirectManipulationInteractionEventHandler {
  friend class DirectManipulationOwner;
  FML_FRIEND_REF_COUNTED_THREAD_SAFE(DirectManipulationEventHandler);
  FML_FRIEND_MAKE_REF_COUNTED(DirectManipulationEventHandler);

 public:
  explicit DirectManipulationEventHandler(WindowWin32* window,
                                          DirectManipulationOwner* owner)
      : window_(window), owner_(owner) {}

  STDMETHODIMP QueryInterface(REFIID iid, void** ppv) override;

  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;

  HRESULT STDMETHODCALLTYPE
  OnViewportStatusChanged(IDirectManipulationViewport* viewport,
                          DIRECTMANIPULATION_STATUS current,
                          DIRECTMANIPULATION_STATUS previous) override;

  HRESULT STDMETHODCALLTYPE
  OnViewportUpdated(IDirectManipulationViewport* viewport) override;

  HRESULT STDMETHODCALLTYPE
  OnContentUpdated(IDirectManipulationViewport* viewport,
                   IDirectManipulationContent* content) override;

  HRESULT STDMETHODCALLTYPE
  OnInteraction(IDirectManipulationViewport2* viewport,
                DIRECTMANIPULATION_INTERACTION_TYPE interaction) override;

 private:
  WindowWin32* window_;
  DirectManipulationOwner* owner_;
  // We need to reset some parts of DirectManipulation after each gesture
  // A flag is needed to ensure that false events created as the reset occurs
  // are not sent to the flutter framework.
  bool resetting_ = false;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_DIRECT_MANIPULATION_H_
