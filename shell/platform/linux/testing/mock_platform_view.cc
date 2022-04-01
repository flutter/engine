// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/testing/mock_platform_view.h"

struct _FlMockPlatformView {
  FlPlatformView parent_instance;

  int64_t view_identifier;

  GtkWidget* (*get_view)(FlPlatformView* self);
};

G_DEFINE_TYPE(FlMockPlatformView,
              fl_mock_platform_view,
              fl_platform_view_get_type())

static GtkWidget* fl_mock_platform_view_get_view(
    FlPlatformView* platform_view) {
  g_return_val_if_fail(FL_IS_MOCK_PLATFORM_VIEW(platform_view), nullptr);
  FlMockPlatformView* self = FL_MOCK_PLATFORM_VIEW(platform_view);
  if (!self->get_view)
    return nullptr;
  else
    return self->get_view(FL_PLATFORM_VIEW(self));
}

static void fl_mock_platform_view_class_init(FlMockPlatformViewClass* klass) {
  FL_PLATFORM_VIEW_CLASS(klass)->get_view = fl_mock_platform_view_get_view;
}

static void fl_mock_platform_view_init(FlMockPlatformView* self) {}

FlMockPlatformView* fl_mock_platform_view_new(
    GtkWidget* (*get_view)(FlPlatformView* self)) {
  FlMockPlatformView* self = FL_MOCK_PLATFORM_VIEW(
      g_object_new(fl_mock_platform_view_get_type(), nullptr));
  self->get_view = get_view;
  return self;
}

struct _FlMockViewFactory {
  GObject parent_instance;

  FlPlatformView* (*create_platform_view)(FlPlatformViewFactory* self,
                                          int64_t view_identifier,
                                          FlValue* args);
  FlMessageCodec* (*get_create_arguments_codec)(FlPlatformViewFactory* self);
};

static void fl_mock_view_factory_fl_platform_view_factory_iface_init(
    FlPlatformViewFactoryInterface* iface);

G_DEFINE_TYPE_WITH_CODE(
    FlMockViewFactory,
    fl_mock_view_factory,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(
        fl_platform_view_factory_get_type(),
        fl_mock_view_factory_fl_platform_view_factory_iface_init))

static FlPlatformView* fl_mock_view_factory_create_platform_view(
    FlPlatformViewFactory* factory,
    int64_t view_identifier,
    FlValue* args) {
  FlMockViewFactory* self = FL_MOCK_VIEW_FACTORY(factory);
  return self->create_platform_view(factory, view_identifier, args);
}

static FlMessageCodec* fl_mock_view_factory_get_create_arguments_codec(
    FlPlatformViewFactory* factory) {
  FlMockViewFactory* self = FL_MOCK_VIEW_FACTORY(factory);
  if (!self->get_create_arguments_codec)
    return nullptr;
  else
    return self->get_create_arguments_codec(factory);
}

static void fl_mock_view_factory_fl_platform_view_factory_iface_init(
    FlPlatformViewFactoryInterface* iface) {
  iface->create_platform_view = fl_mock_view_factory_create_platform_view;
  iface->get_create_arguments_codec =
      fl_mock_view_factory_get_create_arguments_codec;
}

static void fl_mock_view_factory_class_init(FlMockViewFactoryClass* klass) {}

static void fl_mock_view_factory_init(FlMockViewFactory* self) {}

// Creates a mock platform_view_factory
FlMockViewFactory* fl_mock_view_factory_new(
    FlPlatformView* (*create_platform_view)(FlPlatformViewFactory* self,
                                            int64_t view_identifier,
                                            FlValue* args),
    FlMessageCodec* (*get_create_arguments_codec)(
        FlPlatformViewFactory* self)) {
  FlMockViewFactory* self = FL_MOCK_VIEW_FACTORY(
      g_object_new(fl_mock_view_factory_get_type(), nullptr));
  self->create_platform_view = create_platform_view;
  self->get_create_arguments_codec = get_create_arguments_codec;
  return self;
}
