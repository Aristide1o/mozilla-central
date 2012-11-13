/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
#ifndef MOZILLA_GFX_TEXTUREOGL_H
#define MOZILLA_GFX_TEXTUREOGL_H
 
#include "ImageLayerOGL.h"
#include "CompositorOGL.h"
#include "GLContext.h"
#include "gfx2DGlue.h"
 
namespace mozilla {
namespace layers {
 
class TextureSourceOGL : public TextureSource
{
public:
  virtual GLuint GetTextureHandle() = 0;
  virtual gfx::IntSize GetSize() = 0;
  virtual GLenum GetWrapMode() = 0;
#ifdef DEBUG
  virtual bool IsAlpha() { return true; }
#endif
};

//TODO[nrc] TextureOGL and Texture are only used by CreateTextureForData,
// which is not used anywhere, so what are they for?
class TextureOGL : public Texture
{
  typedef mozilla::gl::GLContext GLContext;
public:
  TextureOGL(GLContext* aGL, GLuint aTextureHandle, const gfx::IntSize& aSize)
    : mGL(aGL)
    , mTextureHandle(aTextureHandle)
    , mSize(aSize)
  {}
 
  virtual GLuint GetTextureHandle()
  {
    return mTextureHandle;
  }
 
  virtual gfx::IntSize GetSize()
  {
    return mSize;
  }
 
  virtual void UpdateTexture(const nsIntRegion& aRegion,
                             int8_t *aData,
                             uint32_t aStride) MOZ_OVERRIDE;
 
  void SetProperties(GLenum aFormat,
                    GLenum aInternalFormat,
                    GLenum aType,
                    uint32_t aPixelSize)
  {
    mFormat = aFormat;
    mInternalFormat = aInternalFormat;
    mType = aType;
    mPixelSize = aPixelSize;
  }
 
private:
  GLuint mTextureHandle;
  GLenum mFormat;
  GLenum mInternalFormat;
  GLenum mType;
  nsRefPtr<GLContext> mGL;
  uint32_t mPixelSize;
  gfx::IntSize mSize;
};

class TextureHostOGL : public TextureHost
{
public:
  virtual gfx::IntSize GetSize()
  {
    return mSize;
  }
 
  virtual GLenum GetWrapMode()
  {
    return mWrapMode;
  }
 
  virtual void SetWrapMode(GLenum aWrapMode)
  {
    mWrapMode = aWrapMode;
  }
protected:
  TextureHostOGL()
    : mWrapMode(LOCAL_GL_REPEAT)
  {}
 
  gfx::IntSize mSize;
  GLenum mWrapMode;
};

class TextureSourceHostOGL : public TextureHostOGL
                           , public TextureSourceOGL
{
public:
  void AddRef() { TextureBase::AddRef(); }
  void Release() { TextureBase::Release(); }

  virtual gfx::IntSize GetSize() { return TextureHostOGL::GetSize(); }
  virtual GLenum GetWrapMode() { return TextureHostOGL::GetWrapMode(); }
  virtual TextureSource* GetAsTextureSource() { return this; }
protected:
    TextureSourceHostOGL() {}
};
 
//thin TextureHost wrapper around a TextureImage
class TextureImageAsTextureHost : public TextureSourceHostOGL
                                , public TileIterator
{
public:
  virtual gfx::IntSize GetSize()
  {
    return mSize;
  }
 
  virtual GLuint GetTextureHandle()
  {
    return mTexImage->GetTextureID();
  }
 
  virtual GLenum GetWrapMode()
  {
    return mTexImage->mWrapMode;
  }
 
  virtual void SetWrapMode(GLenum aWrapMode)
  {
    mTexImage->mWrapMode = aWrapMode;
  }
 
  virtual void Update(const SharedImage& aImage,
                      SharedImage* aResult = nullptr,
                      bool* aIsInitialised = nullptr,
                      bool* aNeedsReset = nullptr);
  virtual void Update(gfxASurface* aSurface, nsIntRegion& aRegion);
  
  virtual void Abort();

  virtual TileIterator* GetAsTileIterator() { return this; }
  virtual Effect* Lock(const gfx::Filter& aFilter);
 
  void SetFilter(const gfx::Filter& aFilter) { mTexImage->SetFilter(gfx::ThebesFilter(aFilter)); }
  virtual void BeginTileIteration() { mTexImage->BeginTileIteration(); }
  virtual nsIntRect GetTileRect() { return mTexImage->GetTileRect(); }
  virtual size_t GetTileCount() { return mTexImage->GetTileCount(); }
  virtual bool NextTile() { return mTexImage->NextTile(); }

#ifdef DEBUG
  virtual bool IsAlpha() { return mTexImage->GetContentType() == gfxASurface::CONTENT_ALPHA; }
#endif
 
#ifdef MOZ_DUMP_PAINTING
  virtual already_AddRefed<gfxImageSurface> Dump()
  {
    return mGL->GetTexImage(GetTextureHandle(), false, mTexImage->GetShaderProgramType());
  }
#endif
 
protected:
  typedef mozilla::gl::GLContext GLContext;
  typedef mozilla::gl::TextureImage TextureImage;
 
  TextureImageAsTextureHost(GLContext* aGL)
    : mGL(aGL)
    , mTexImage(nullptr)
  {}
 
  GLContext* mGL;
  nsRefPtr<TextureImage> mTexImage;
 
  friend class CompositorOGL;
};
 
// a TextureImageAsTextureHost for use with main thread composition
// i.e., where we draw to it directly, and do not have a texture client
class TextureImageHost : public TextureImageAsTextureHost
{
public:
  TextureImageHost(GLContext* aGL, TextureImage* aTexImage);
 
  TextureImage* GetTextureImage() { return mTexImage; }
  void SetTextureImage(TextureImage* aTexImage)
  {
    mTexImage = aTexImage;
    mSize = gfx::IntSize(mTexImage->mSize.width, mTexImage->mSize.height);
  }

  virtual void Update(const SharedImage& aImage,
                      SharedImage* aResult = nullptr,
                      bool* aIsInitialised = nullptr,
                      bool* aNeedsReset = nullptr) {}
  virtual void Update(gfxASurface* aSurface, nsIntRegion& aRegion) {}
};

class TextureImageAsTextureHostWithBuffer : public TextureImageAsTextureHost
{
public:
  ~TextureImageAsTextureHostWithBuffer();
 
  virtual void Update(const SurfaceDescriptor& aNewBuffer,
                      SharedImage* aResult = nullptr,
                      bool* aIsInitialised = nullptr,
                      bool* aNeedsReset = nullptr);
  /**
   * Set deallocator for data recieved from IPC protocol
   * We should be able to set allocator right before swap call
   * that is why allowed multiple call with the same Allocator
   */
  virtual void SetDeAllocator(ISurfaceDeAllocator* aDeAllocator)
  {
    NS_ASSERTION(!mDeAllocator || mDeAllocator == aDeAllocator, "Stomping allocator?");
    mDeAllocator = aDeAllocator;
  }
 
protected:
  TextureImageAsTextureHostWithBuffer(GLContext* aGL)
    : TextureImageAsTextureHost(aGL)
  {}
 
  // returns true if the buffer was reset
  bool EnsureBuffer(nsIntSize aSize);
 
  ISurfaceDeAllocator* mDeAllocator;
  SurfaceDescriptor mBufferDescriptor;
 
  friend class CompositorOGL;
};
 
class TextureHostOGLShared : public TextureSourceHostOGL
{
public:
  virtual ~TextureHostOGLShared()
  {
    mGL->MakeCurrent();
    mGL->ReleaseSharedHandle(mShareType, mSharedHandle);
    if (mTextureHandle) {
      mGL->fDeleteTextures(1, &mTextureHandle);
    }
  }
 
  virtual gfx::IntSize GetSize()
  {
    return mSize;
  }
 
  virtual GLuint GetTextureHandle()
  {
    return mTextureHandle;
  }
 
  virtual void Update(const SharedImage& aImage,
                      SharedImage* aResult = nullptr,
                      bool* aIsInitialised = nullptr,
                      bool* aNeedsReset = nullptr);
  virtual Effect* Lock(const gfx::Filter& aFilter);
  virtual void Unlock();
 
protected:
  typedef mozilla::gl::GLContext GLContext;
  typedef mozilla::gl::TextureImage TextureImage;
 
  TextureHostOGLShared(GLContext* aGL)
    : mGL(aGL)
  {}
 
  GLContext* mGL;
  GLuint mTextureHandle;
  gl::SharedTextureHandle mSharedHandle;
  gl::TextureImage::TextureShareType mShareType;
 
  friend class CompositorOGL;
};

class TextureHostOGLSharedWithBuffer : public TextureHostOGLShared
{
public:
  virtual void Update(const SharedImage& aImage,
                      SharedImage* aResult = nullptr,
                      bool* aIsInitialised = nullptr,
                      bool* aNeedsReset = nullptr);
 
protected:
  TextureHostOGLSharedWithBuffer(GLContext* aGL)
    : TextureHostOGLShared(aGL)
  {}
 
  SharedImage mBuffer;
 
  friend class CompositorOGL;
};

class GLTextureAsTextureSource : public TextureSourceOGL
{
  typedef mozilla::gl::GLContext GLContext;
public: 
  ~GLTextureAsTextureSource()
  {
    mTexture.Release();
  }

  void Update(gfx::IntSize aSize, uint8_t* aData, uint32_t aStride, GLContext* aGL);

  virtual GLuint GetTextureHandle()
  {
    return mTexture.GetTextureID();
  }

  virtual gfx::IntSize GetSize()
  {
    return mSize;
  }
 
  virtual GLenum GetWrapMode()
  {
    return LOCAL_GL_REPEAT;
  }

private:
  GLTexture mTexture;
  gfx::IntSize mSize;
};

class GLTextureAsTextureHost : public TextureSourceHostOGL
{
  typedef mozilla::gl::GLContext GLContext;
public:
  GLTextureAsTextureHost(GLContext* aGL)
    : mGL(aGL)
  {}
 
  ~GLTextureAsTextureHost()
  {
    mTexture.Release();
  }
 
  virtual GLuint GetTextureHandle()
  {
    return mTexture.GetTextureID();
  }
 
  virtual void Update(const SharedImage& aImage,
                      SharedImage* aResult = nullptr,
                      bool* aIsInitialised = nullptr,
                      bool* aNeedsReset = nullptr);
 
private:
  nsRefPtr<GLContext> mGL;
  GLTexture mTexture;
};

// a texture host with all three plains in one texture
class YCbCrTextureHost : public TextureHostOGL
{
  typedef mozilla::gl::GLContext GLContext;
public:
  YCbCrTextureHost(GLContext* aGL)
    : mGL(aGL)
  {}

  virtual void Update(const SharedImage& aImage,
                      SharedImage* aResult = nullptr,
                      bool* aIsInitialised = nullptr,
                      bool* aNeedsReset = nullptr);
  virtual Effect* Lock(const gfx::Filter& aFilter);

private:
  nsRefPtr<GLContext> mGL;
  GLTextureAsTextureSource mTextures[3];
};

#ifdef MOZ_WIDGET_GONK
// For direct texturing with OES_EGL_image_external extension. This
// texture is allocated when the image supports binding with
// BindExternalBuffer.
class DirectExternalTextureHost : public TextureSourceHostOGL
{
  typedef mozilla::gl::GLContext GLContext;
public:
  GLTextureAsTextureHost(GLContext* aGL)
    : mGL(aGL)
  {}
 
  ~GLTextureAsTextureHost()
  {
    mExternalBufferTexture.Release();
  }
 
  virtual GLuint GetTextureHandle()
  {
    return mExternalBufferTexture.GetTextureID()
  }
 
  virtual void Update(const SharedImage& aImage,
                      SharedImage* aResult = nullptr,
                      bool* aIsInitialised = nullptr,
                      bool* aNeedsReset = nullptr);
  virtual Effect* Lock(const gfx::Filter& aFilter);
 
private:
  nsRefPtr<GLContext> mGL;
  GLTexture mExternalBufferTexture;
};
#endif

}
}
 
#endif /* MOZILLA_GFX_TEXTUREOGL_H */
