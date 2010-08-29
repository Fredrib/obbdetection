//=============================================================================
// DrawableTex2D.cpp by Frank Luna (C) 2005 All Rights Reserved.
//=============================================================================

#include "main.h"

DrawableTex2D::DrawableTex2D(UINT width, UINT height, UINT mipLevels,
		D3DFORMAT texFormat, bool useDepthBuffer,
		D3DFORMAT depthFormat, D3DVIEWPORT9& viewport,  bool autoGenMips)
: mTex(0), mRTS(0), mTopSurf(0), mWidth(width), mHeight(height), 
  mMipLevels(mipLevels), mTexFormat(texFormat), mUseDepthBuffer(useDepthBuffer),
  mDepthFormat(depthFormat), mViewPort(viewport), mAutoGenMips(autoGenMips)
{
}

DrawableTex2D::~DrawableTex2D()
{
	onLostDevice();
}

IDirect3DTexture9* DrawableTex2D::d3dTex()
{
	return mTex;
}

void DrawableTex2D::onLostDevice()
{
	ReleaseCOM(mTex);
	ReleaseCOM(mRTS);
	ReleaseCOM(mTopSurf);
}

void DrawableTex2D::onResetDevice()
{
	UINT usage = D3DUSAGE_RENDERTARGET;
	if(mAutoGenMips)
		usage |= D3DUSAGE_AUTOGENMIPMAP;

	HR(D3DXCreateTexture(g_app->m_pD3DDev, mWidth, mHeight, mMipLevels, usage, mTexFormat, D3DPOOL_DEFAULT, &mTex),
		L"DrawableTex2D::onResetDevice: D3DXCreateTexture failed: ");
	HR(D3DXCreateRenderToSurface(g_app->m_pD3DDev, mWidth, mHeight, mTexFormat, mUseDepthBuffer, mDepthFormat, &mRTS),
		L"DrawableTex2D::onResetDevice: D3DXCreateRenderToSurface failed: ");
	HR(mTex->GetSurfaceLevel(0, &mTopSurf),L"DrawableTex2D::onResetDevice: GetSurfaceLevel failed: ");
}

void DrawableTex2D::beginScene()
{
	mRTS->BeginScene(mTopSurf, &mViewPort);
}

void DrawableTex2D::endScene()
{
	mRTS->EndScene(D3DX_FILTER_NONE);
}