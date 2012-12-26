/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/layers/Compositor.h"
#include "mozilla/layers/LayersSurfaces.h"
#include "mozilla/layers/ShadowLayers.h" // for ISurfaceDeAllocator
#include "mozilla/layers/ImageContainerParent.h"

namespace mozilla {
namespace layers {

TextureHost::TextureHost(Buffering aBuffering)
  : mFlags(NoFlags)
  , mBuffering(aBuffering)
  , mAsyncContainerID(0)
  , mAsyncTextureVersion(0)
  , mCompositorID(0)
{
  if (aBuffering != Buffering::NONE) {
    mBuffer = new SharedImage;
  }
}

TextureHost::~TextureHost()
{
  if (IsBuffered()) {
    MOZ_ASSERT(mDeAllocator);
    mDeAllocator->DestroySharedSurface(mBuffer);
    delete mBuffer;
  }
}

void TextureHost::Update(const SharedImage& aImage,
                         SharedImage* aResult,
                         bool* aIsInitialised,
                         bool* aNeedsReset)
{
  UpdateImpl(aImage, aIsInitialised, aNeedsReset);

  // buffering
  if (IsBuffered()) {
    if (aResult) {
      *aResult = *mBuffer;
    }
    *mBuffer = aImage;
  } else {
    if (aResult) {
      *aResult = aImage;
    }
  }
}

bool TextureHost::UpdateAsyncTexture()
{
  if (!IsAsync()) {
    return true;
  }
  ImageContainerParent::SetCompositorIDForImage(mAsyncContainerID, mCompositorID);
  uint32_t imgVersion = ImageContainerParent::GetSharedImageVersion(mAsyncContainerID);
  if (imgVersion != mAsyncTextureVersion) {
    SharedImage* img = ImageContainerParent::GetSharedImage(mAsyncContainerID);
    if (!img) {
      return false;
    }
    Update(*img, img, nullptr, nullptr);
    mAsyncTextureVersion = imgVersion;
  }
  return true;
}

void TextureHost::Update(gfxASurface* aSurface, nsIntRegion& aRegion) {
  UpdateImpl(aSurface, aRegion);
  MOZ_ASSERT(!IsBuffered(), "Buffered TextureHosts are not meant to do thebes updates");
}


} // namespace
} // namespace