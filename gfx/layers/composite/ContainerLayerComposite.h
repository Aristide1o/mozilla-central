/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GFX_ContainerLayerComposite_H
#define GFX_ContainerLayerComposite_H

#include "mozilla/layers/PLayers.h"
#include "mozilla/layers/ShadowLayers.h"

#include "Layers.h"
#include "LayerManagerComposite.h"
#include "LayerImplComposite.h"

namespace mozilla {
namespace layers {

class ContainerLayerComposite : public ShadowContainerLayer,
                                public LayerComposite,
                                private ContainerLayerImpl<ContainerLayerComposite,
                                                           LayerComposite,
                                                           LayerManagerComposite>
{
  template<class ContainerT,
           class LayerT,
           class ManagerT>
  friend class mozilla::layers::ContainerLayerImpl;

public:
  ContainerLayerComposite(LayerManagerComposite *aManager);
  ~ContainerLayerComposite();

  void InsertAfter(Layer* aChild, Layer* aAfter);

  void RemoveChild(Layer* aChild);
  
  void RepositionChild(Layer* aChild, Layer* aAfter);

  // LayerComposite Implementation
  virtual Layer* GetLayer() { return this; }

  virtual void Destroy();

  LayerComposite* GetFirstChildComposite();

  virtual void RenderLayer(const nsIntPoint& aOffset,
                           const nsIntRect& aClipRect,
                           CompositingRenderTarget* aPreviousTarget = nullptr);

  virtual void ComputeEffectiveTransforms(const gfx3DMatrix& aTransformToSurface)
  {
    DefaultComputeEffectiveTransforms(aTransformToSurface);
  }

  virtual void CleanupResources();

  BufferHost* GetBufferHost() MOZ_OVERRIDE { return nullptr; }
};

class CompositeRefLayer : public ShadowRefLayer,
                          public LayerComposite,
                          protected ContainerLayerImpl<CompositeRefLayer,
                                                       LayerComposite,
                                                       LayerManagerComposite>
{
  template<class ContainerT,
         class LayerT,
         class ManagerT>
  friend class mozilla::layers::ContainerLayerImpl;

public:
  CompositeRefLayer(LayerManagerComposite *aManager);
  ~CompositeRefLayer();

  /** LayerOGL implementation */
  Layer* GetLayer() { return this; }

  void Destroy();

  LayerComposite* GetFirstChildComposite();

  virtual void RenderLayer(const nsIntPoint& aOffset,
                           const nsIntRect& aClipRect,
                           CompositingRenderTarget* aPreviousTarget = nullptr);

  virtual void ComputeEffectiveTransforms(const gfx3DMatrix& aTransformToSurface)
  {
    DefaultComputeEffectiveTransforms(aTransformToSurface);
  }

  virtual void CleanupResources();

    BufferHost* GetBufferHost() MOZ_OVERRIDE { return nullptr; }
};

} /* layers */
} /* mozilla */

#endif /* GFX_ContainerLayerComposite_H */
