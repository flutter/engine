// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/transform_layer.h"

namespace flow {

TransformLayer::TransformLayer() = default;

TransformLayer::~TransformLayer() = default;

void TransformLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  SkMatrix childMatrix;
  childMatrix.setConcat(matrix, transform_);
  PrerollChildren(context, childMatrix);

  if (needs_system_composite())
    return;

  transform_.mapRect(&context->child_paint_bounds);
  set_paint_bounds(context->child_paint_bounds);
}

#if defined(OS_FUCHSIA)

void TransformLayer::UpdateScene(SceneUpdateContext& context,
                                 mozart::client::ContainerNode& container) {
  FTL_DCHECK(needs_system_composite());

  mozart::client::EntityNode node(context.session());

  // TODO(chinmaygarde): The perspective and shear components in the matrix are
  // not handled correctly.
  MatrixDecomposition decomposition(transform_);

  if (decomposition.IsValid()) {
    node.SetTranslation(decomposition.translation().x(),  //
                        decomposition.translation().y(),  //
                        decomposition.translation().z()   //
                        );
    node.SetScale(decomposition.scale().x(),  //
                  decomposition.scale().y(),  //
                  decomposition.scale().z()   //
                  );
    node.SetRotation(decomposition.rotation().fData[0],  //
                     decomposition.rotation().fData[1],  //
                     decomposition.rotation().fData[2],  //
                     decomposition.rotation().fData[3]   //
                     );
  }

  UpdateSceneChildrenInsideNode(context, container, node);
}

#endif  // defined(OS_FUCHSIA)

void TransformLayer::Paint(PaintContext& context) {
  TRACE_EVENT0("flutter", "TransformLayer::Paint");
  FTL_DCHECK(!needs_system_composite());

  SkAutoCanvasRestore save(&context.canvas, true);
  context.canvas.concat(transform_);
  PaintChildren(context);
}

}  // namespace flow
