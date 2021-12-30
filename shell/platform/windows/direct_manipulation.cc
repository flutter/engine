// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/logging.h"

#include "flutter/shell/platform/windows/direct_manipulation.h"
#include "flutter/shell/platform/windows/window_binding_handler_delegate.h"
#include "flutter/shell/platform/windows/window_win32.h"

#define VERIFY_HR(operation)                   \
  if (FAILED(operation)) {                     \
    FML_LOG(ERROR) << #operation << " failed"; \
    manager_ = nullptr;                        \
    updateManager_ = nullptr;                  \
    viewport_ = nullptr;                       \
    return -1;                                 \
  }

#define WARN_HR(operation)                     \
  if (FAILED(operation)) {                     \
    FML_LOG(ERROR) << #operation << " failed"; \
  }

namespace flutter {

STDMETHODIMP DirectManipulationEventHandler::QueryInterface(REFIID iid,
                                                            void** ppv) {
  if ((iid == IID_IUnknown) ||
      (iid == IID_IDirectManipulationViewportEventHandler)) {
    *ppv = static_cast<IDirectManipulationViewportEventHandler*>(this);
    AddRef();
    return S_OK;
  } else if (iid == IID_IDirectManipulationInteractionEventHandler) {
    *ppv = static_cast<IDirectManipulationInteractionEventHandler*>(this);
    AddRef();
    return S_OK;
  }
  return E_NOINTERFACE;
}

HRESULT DirectManipulationEventHandler::OnViewportStatusChanged(
    IDirectManipulationViewport* viewport,
    DIRECTMANIPULATION_STATUS current,
    DIRECTMANIPULATION_STATUS previous) {
  if (current == DIRECTMANIPULATION_RUNNING) {
    if (!resetting_) {
      // Not a false event
      if (owner_->binding_handler_delegate) {
        owner_->binding_handler_delegate->OnPointerPanZoomStart(
            (int32_t) reinterpret_cast<int64_t>(this));
      }
    }
  } else if (previous == DIRECTMANIPULATION_RUNNING) {
    if (resetting_) {
      // The resetting transition has concluded
      resetting_ = false;
    } else {
      if (owner_->binding_handler_delegate) {
        owner_->binding_handler_delegate->OnPointerPanZoomEnd(
            (int32_t) reinterpret_cast<int64_t>(this));
      }
      // Need to reset the content transform to its original position
      // so that we are ready for the next gesture
      // Use resetting_ flag to prevent sending reset also to the framework
      resetting_ = true;
      RECT rect;
      HRESULT hr = viewport->GetViewportRect(&rect);
      if (FAILED(hr)) {
        FML_LOG(ERROR) << "Failed to get the current viewport rect";
        return E_FAIL;
      }
      hr = viewport->ZoomToRect(rect.left, rect.top, rect.right, rect.bottom,
                                false);
      if (FAILED(hr)) {
        FML_LOG(ERROR) << "Failed to reset the gesture using ZoomToRect";
        return E_FAIL;
      }
    }
  }
  return S_OK;
}

HRESULT DirectManipulationEventHandler::OnViewportUpdated(
    IDirectManipulationViewport* viewport) {
  return S_OK;
}

HRESULT DirectManipulationEventHandler::OnContentUpdated(
    IDirectManipulationViewport* viewport,
    IDirectManipulationContent* content) {
  float transform[6];
  HRESULT hr = content->GetContentTransform(transform, ARRAYSIZE(transform));
  if (FAILED(hr)) {
    FML_LOG(ERROR) << "GetContentTransform failed";
    return S_OK;
  }
  if (!resetting_) {
    // DirectManipulation provides updates with very high precision. If the user
    // holds their fingers steady on a trackpad, DirectManipulation sends
    // jittery updates. This calculation will reduce the precision of the scale
    // value of the event to avoid jitter
    const int mantissa_bits_chop = 2;
    const float factor = (1 << mantissa_bits_chop) + 1;
    float c = factor * transform[0];
    float scale = c - (c - transform[0]);
    float pan_x = transform[4];
    float pan_y = transform[5];
    if (owner_->binding_handler_delegate) {
      owner_->binding_handler_delegate->OnPointerPanZoomUpdate(
          (int32_t) reinterpret_cast<int64_t>(this), pan_x, pan_y, scale, 0);
    }
  }
  return S_OK;
}

HRESULT DirectManipulationEventHandler::OnInteraction(
    IDirectManipulationViewport2* viewport,
    DIRECTMANIPULATION_INTERACTION_TYPE interaction) {
  return S_OK;
}

ULONG STDMETHODCALLTYPE DirectManipulationEventHandler::AddRef() {
  RefCountedThreadSafe::AddRef();
  return 0;
}

ULONG STDMETHODCALLTYPE DirectManipulationEventHandler::Release() {
  RefCountedThreadSafe::Release();
  return 0;
}

DirectManipulationOwner::DirectManipulationOwner(WindowWin32* window)
    : window_(window) {}

int DirectManipulationOwner::Init(unsigned int width, unsigned int height) {
  VERIFY_HR(CoCreateInstance(CLSID_DirectManipulationManager, nullptr,
                             CLSCTX_INPROC_SERVER,
                             IID_IDirectManipulationManager, &manager_));
  VERIFY_HR(manager_->GetUpdateManager(IID_IDirectManipulationUpdateManager,
                                       &updateManager_));
  VERIFY_HR(manager_->CreateViewport(nullptr, window_->GetWindowHandle(),
                                     IID_IDirectManipulationViewport,
                                     &viewport_));
  DIRECTMANIPULATION_CONFIGURATION configuration =
      DIRECTMANIPULATION_CONFIGURATION_INTERACTION |
      DIRECTMANIPULATION_CONFIGURATION_TRANSLATION_X |
      DIRECTMANIPULATION_CONFIGURATION_TRANSLATION_Y |
      DIRECTMANIPULATION_CONFIGURATION_SCALING;
  VERIFY_HR(viewport_->ActivateConfiguration(configuration));
  VERIFY_HR(viewport_->SetViewportOptions(
      DIRECTMANIPULATION_VIEWPORT_OPTIONS_MANUALUPDATE));
  handler_ = fml::MakeRefCounted<DirectManipulationEventHandler>(window_, this);
  VERIFY_HR(viewport_->AddEventHandler(
      window_->GetWindowHandle(), handler_.get(), &viewportHandlerCookie_));
  RECT rect = {0, 0, (LONG)width, (LONG)height};
  VERIFY_HR(viewport_->SetViewportRect(&rect));
  VERIFY_HR(manager_->Activate(window_->GetWindowHandle()));
  VERIFY_HR(viewport_->Enable());
  VERIFY_HR(updateManager_->Update(nullptr));
  return 0;
}

void DirectManipulationOwner::ResizeViewport(unsigned int width,
                                             unsigned int height) {
  if (viewport_) {
    RECT rect = {0, 0, (LONG)width, (LONG)height};
    WARN_HR(viewport_->SetViewportRect(&rect));
  }
}

void DirectManipulationOwner::Destroy() {
  if (handler_) {
    handler_->window_ = nullptr;
    handler_->owner_ = nullptr;
  }

  if (viewport_) {
    WARN_HR(viewport_->Disable());
    WARN_HR(viewport_->Disable());
    WARN_HR(viewport_->RemoveEventHandler(viewportHandlerCookie_));
    WARN_HR(viewport_->Abandon());
  }

  if (window_ && manager_) {
    WARN_HR(manager_->Deactivate(window_->GetWindowHandle()));
  }

  handler_ = nullptr;
  viewport_ = nullptr;
  updateManager_ = nullptr;
  manager_ = nullptr;
  window_ = nullptr;
}

void DirectManipulationOwner::SetContact(UINT contactId) {
  if (viewport_) {
    viewport_->SetContact(contactId);
  }
}

void DirectManipulationOwner::SetBindingHandlerDelegate(
    WindowBindingHandlerDelegate* delegate) {
  binding_handler_delegate = delegate;
}

void DirectManipulationOwner::Update() {
  if (updateManager_) {
    HRESULT hr = updateManager_->Update(nullptr);
    if (FAILED(hr)) {
      FML_LOG(ERROR) << "updateManager_->Update failed";
      auto error = GetLastError();
      FML_LOG(ERROR) << error;
      LPWSTR message = nullptr;
      size_t size = FormatMessageW(
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
              FORMAT_MESSAGE_IGNORE_INSERTS,
          NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          reinterpret_cast<LPWSTR>(&message), 0, NULL);
      FML_LOG(ERROR) << message;
    }
  }
}

}  // namespace flutter
