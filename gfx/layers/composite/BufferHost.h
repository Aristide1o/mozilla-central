/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLA_GFX_BUFFERHOST_H
#define MOZILLA_GFX_BUFFERHOST_H

#include "mozilla/layers/Compositor.h"

namespace mozilla {
namespace layers {

/**
 * A BufferHost is owned by a layer and manages one or several TextureHosts.
 * The implementation should be platform-independent.
 */
class BufferHost : public RefCounted<BufferHost>
{
public:
  virtual ~BufferHost() {}

  virtual BufferType GetType() = 0;

  // composite the contents of this buffer host to the compositor's surface
  virtual void Composite(EffectChain& aEffectChain,
                         float aOpacity,
                         const gfx::Matrix4x4& aTransform,
                         const gfx::Point& aOffset,
                         const gfx::Filter& aFilter,
                         const gfx::Rect& aClipRect,
                         const nsIntRegion* aVisibleRegion = nullptr) = 0;

  virtual void AddTextureHost(const TextureInfo& aTextureInfo,
                              TextureHost* aTextureHost) = 0;

  virtual void SetDeAllocator(ISurfaceDeAllocator* aDeAllocator) {}
};

}
}

#endif
