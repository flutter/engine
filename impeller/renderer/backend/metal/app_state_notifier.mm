// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/metal/app_state_notifier.h"

#import <Foundation/Foundation.h>

#include "flutter/fml/logging.h"

#if !__has_feature(objc_arc)
#error ARC must be enabled !
#endif

#ifdef FML_OS_IOS
#import <UIKit/UIKit.h>
#endif

@interface ImpellerAppStateNotifier : NSObject
@property(nonatomic, assign) impeller::AppStateNotifier* parent;
- (instancetype)initWithParent:(impeller::AppStateNotifier*)parent
    NS_DESIGNATED_INITIALIZER;
- (void)onAppBecameActive:(NSNotification*)notification;
@end

#ifdef FML_OS_IOS
@implementation ImpellerAppStateNotifier
- (instancetype)initWithParent:(impeller::AppStateNotifier*)parent {
  self = [super init];
  if (self) {
    _parent = parent;
    [[NSNotificationCenter defaultCenter]
        addObserver:self
           selector:@selector(onAppBecameActive:)
               name:UIApplicationDidBecomeActiveNotification
             object:nil];
  }
  return self;
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)onAppBecameActive:(NSNotification*)notification {
  _parent->OnAppBecameActive();
}

@end
#else

@implementation ImpellerAppStateNotifier

- (instancetype)init {
  self = [self initWithParent:nil];
  return self;
}

- (instancetype)initWithParent:(impeller::AppStateNotifier*)parent {
  // Note: Not supported on macOS.
  return [super init];
}

- (void)onAppBecameActive:(NSNotification*)notification {
}
@end

#endif  // FML_OS_IOS

namespace impeller {

struct AppStateNotifier::AppStateNotifierImpl {
  ImpellerAppStateNotifier* notifier_;
};

AppStateNotifier::AppStateNotifier()
    : impl_(
          new AppStateNotifierImpl{
              .notifier_ =
                  [[ImpellerAppStateNotifier alloc] initWithParent:this],
          },
          &AppStateNotifier::DeleteImpl) {}

void AppStateNotifier::OnAppBecameActive() {
  if (app_did_become_active_callback_) {
    app_did_become_active_callback_();
  }
}

void AppStateNotifier::DeleteImpl(AppStateNotifierImpl* impl) {
  delete impl;
}

}  // namespace impeller
