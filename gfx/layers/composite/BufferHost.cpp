/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "BufferHost.h"
#include "mozilla/layers/ImageContainerParent.h"
#include "Effects.h"

namespace mozilla {
namespace layers {

void BufferHost::Update(const SharedImage& aImage,
                        SharedImage* aResult,
                        bool* aIsInitialised,
                        bool* aNeedsReset) {
  if (!GetTextureHost()) {
    *aResult = aImage;
    return;
  }

  GetTextureHost()->Update(aImage, aResult, aIsInitialised, aNeedsReset);
}

bool BufferHost::AddMaskEffect(EffectChain& aEffects,
                               const gfx::Matrix4x4& aTransform,
                               bool aIs3D)
{
  TextureSource* source = GetTextureHost()->GetPrimaryTextureSource();
  EffectMask* effect = new EffectMask(source,
                                      GetTextureHost()->GetSize(),
                                      aTransform);
  effect->mIs3D = aIs3D;
  aEffects.mEffects[EFFECT_MASK] = effect;
  return true;
}


} // namespace
} // namespace