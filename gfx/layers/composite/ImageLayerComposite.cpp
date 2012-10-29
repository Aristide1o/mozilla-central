/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gfxSharedImageSurface.h"
#include "mozilla/layers/ImageContainerParent.h"

#include "ipc/AutoOpenSurface.h"
#include "ImageLayerComposite.h"
#include "ImageHost.h"
#include "gfxImageSurface.h"
#include "gfx2DGlue.h"

#include "Compositor.h"

using namespace mozilla::gfx;

namespace mozilla {
namespace layers {

ImageLayerComposite::ImageLayerComposite(LayerManagerComposite* aManager)
  : ShadowImageLayer(aManager, nullptr)
  , LayerComposite(aManager)
  , mImageHost(nullptr)
{
  mImplData = static_cast<LayerComposite*>(this);
}

ImageLayerComposite::~ImageLayerComposite()
{}

void
ImageLayerComposite::AddTextureHost(const TextureIdentifier& aTextureIdentifier, TextureHost* aTextureHost)
{
  EnsureImageHost(aTextureIdentifier.mBufferType);

  mImageHost->AddTextureHost(aTextureIdentifier, aTextureHost);
}

void
ImageLayerComposite::SwapTexture(const TextureIdentifier& aTextureIdentifier,
                                 const SharedImage& aFront,
                                 SharedImage* aNewBack)
{
  if (mDestroyed ||
      !mImageHost) {
    *aNewBack = aFront;
    return;
  }

  *aNewBack = mImageHost->UpdateImage(aTextureIdentifier, aFront);
}

void
ImageLayerComposite::EnsureImageHost(BufferType aHostType)
{
  if (!mImageHost ||
      mImageHost->GetType() != aHostType) {
    RefPtr<BufferHost> bufferHost = mCompositor->CreateBufferHost(aHostType);
    mImageHost = static_cast<ImageHost*>(bufferHost.get());
  }
}

void
ImageLayerComposite::Disconnect()
{
  Destroy();
}

void
ImageLayerComposite::Destroy()
{
  if (!mDestroyed) {
    mDestroyed = true;
    CleanupResources();
  }
}

Layer*
ImageLayerComposite::GetLayer()
{
  return this;
}

void
ImageLayerComposite::RenderLayer(const nsIntPoint& aOffset,
                                 const nsIntRect& aClipRect,
                                 Surface*)
{
  if (mCompositeManager->CompositingDisabled()) {
    return;
  }

  if (!mImageHost) {
    return;
  }

  mCompositor->MakeCurrent();

  EffectChain effectChain;
  effectChain.mEffects[EFFECT_MASK] =
    LayerManagerComposite::MakeMaskEffect(mMaskLayer);

  gfx::Matrix4x4 transform;
  ToMatrix4x4(GetEffectiveTransform(), transform);
  gfx::Rect clipRect(aClipRect.x, aClipRect.y, aClipRect.width, aClipRect.height);

  mImageHost->Composite(effectChain,
                        GetEffectiveOpacity(),
                        transform,
                        gfx::Point(aOffset.x, aOffset.y),
                        gfx::ToFilter(mFilter),
                        clipRect);
}

TemporaryRef<TextureSource> 
ImageLayerComposite::AsTextureSource()
{
  // Our image host ought to be able to supply us with its texture host
  // which ought to be convertable to a texture source. If we want to use
  // complex ImageHosts/TextureHosts as mask layers then we will have to
  // do more work here.

  RefPtr<TextureHost> textureHost = mImageHost->GetTextureHost();
  if (!textureHost) {
    return nullptr;
  }

  return textureHost->GetAsTextureSource();
}

void
ImageLayerComposite::SetPictureRect(const nsIntRect& aPictureRect)
{
  if (mImageHost) {
    mImageHost->SetPictureRect(aPictureRect);
  }
}

void
ImageLayerComposite::CleanupResources()
{
  mImageHost = nullptr;
}

} /* layers */
} /* mozilla */
