// File:		hud.cpp
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

// Class to handle rendering of hud display.  Copied and modified from F Luna's source code (GfxStats.cpp).

#include "hud.h"

hud::hud()
{
	mFont = 0;
	mFPS = 0.0f;
	mMilliSecPerFrame = 0.0f;
	mNumTris = 0;
	mNumVertices = 0;
}

hud::~hud()
{
	ReleaseCOM(mFont);
}

void hud::InitHud()
{
	D3DXFONT_DESC fontDesc;
	fontDesc.Height          = 18;
    fontDesc.Width           = 0;
    fontDesc.Weight          = 0;
    fontDesc.MipLevels       = 1;
    fontDesc.Italic          = false;
    fontDesc.CharSet         = DEFAULT_CHARSET;
    fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
    fontDesc.Quality         = DEFAULT_QUALITY;
    fontDesc.PitchAndFamily  = DEFAULT_PITCH | FF_DONTCARE;
    _tcscpy(fontDesc.FaceName, _T("Times New Roman"));

	HR(D3DXCreateFontIndirect(g_app->m_pD3DDev, &fontDesc, &mFont),L"hud::hud(): D3DXCreateFontIndirect failed: ");
}

void hud::onLostDevice()
{
	HR(mFont->OnLostDevice(),L"hud::onLostDevice(): OnLostDevice failed: ");
}

void hud::onResetDevice()
{
	HR(mFont->OnResetDevice(),L"hud::onResetDevice(): onResetDevice failed: ");
}

void hud::addVertices(DWORD n)
{
	mNumVertices += n;
}

void hud::subVertices(DWORD n)
{
	mNumVertices -= n;
}

void hud::addTriangles(DWORD n)
{
	mNumTris += n;
}

void hud::subTriangles(DWORD n)
{
	mNumTris -= n;
}

void hud::setTriCount(DWORD n)
{
	mNumTris = n;
}

void hud::setVertexCount(DWORD n)
{
	mNumVertices = n;
}

void hud::update(float dt)
{
	// Make static so that their values persist accross function calls.
	static float numFrames   = 0.0f;
	static float timeElapsed = 0.0f;

	// Increment the frame count.
	numFrames += 1.0f;

	// Accumulate how much time has passed.
	timeElapsed += dt;

	// Has one second passed?--we compute the frame statistics once 
	// per second.  Note that the time between frames can vary so 
	// these stats are averages over a second.
	if( timeElapsed >= 1.0f )
	{
		// Frames Per Second = numFrames / timeElapsed,
		// but timeElapsed approx. equals 1.0, so 
		// frames per second = numFrames.

		mFPS = numFrames;

		// Average time, in miliseconds, it took to render a single frame.
		mMilliSecPerFrame = 1000.0f / mFPS;

		// Reset time counter and frame count to prepare for computing
		// the average stats over the next second.
		timeElapsed = 0.0f;
		numFrames   = 0.0f;
	}
}

void hud::display()
{
	// Make static so memory is not allocated every frame.
	// POTENTIAL FOR BUFFER OVERFLOW IF STRING IS MORE THAN 1023 CHARS LONG
	static char buffer[512];

	sprintf(buffer, "Frames Per Second = %.2f\n"
		"Milliseconds Per Frame = %.4f\n"
		"Triangle Count = %d\n"
		"Vertex Count = %d\n\n"
		"Q,ESC  -> quit\n"
		"WASD  -> Move Camera\n"
		"shift  -> Run\n"
		"C  -> Convex Hull Rendering ON/OFF\n"
		"O  -> OBBTree Rendering ON/OFF\n"
		"M  -> Model Rendering ON/OFF\n"
		"P  -> pause physics\n"
		"L  -> Rendering to parent\n"
		"<  -> Rendering to Left-Child\n"
		">  -> Rendering to Right-Child\n"
		"N  -> Next OBB pair in OBBDebugMode\n"
		"F1  -> Fullscreen ON/OFF\n", mFPS, mMilliSecPerFrame, mNumTris, mNumVertices);

	RECT R = {5, 5, 0, 0};
	HR(mFont->DrawText(0, toWideString(buffer,511).c_str(), -1, &R, DT_NOCLIP, D3DCOLOR_XRGB(0,0,0)),L"hud::display(): DrawText failed: ");
}

void hud::RenderProgressText(int curObbox, int totalObboxes )
{
	// Make static so memory is not allocated every frame.
	// POTENTIAL FOR BUFFER OVERFLOW
	static char buffer[256];
	sprintf(buffer, "Rebuilding ObbTree, node %d of %d\n",curObbox,totalObboxes);

	HR(g_app->m_pD3DDev->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255,255,255), 1.0f, 0),L"renderer::drawShadowMap: Failed to clear device: ");

	HR(g_app->m_pD3DDev->BeginScene(),L"renderer::RenderFrame: BeginScene failed: ");
	// DRAW ALL OBJECTS HERE
		RECT R = {5, 5, 0, 0};
		HR(mFont->DrawText(0, toWideString(buffer,255).c_str(), -1, &R, DT_NOCLIP, D3DCOLOR_XRGB(0,0,0)),L"hud::display(): DrawText failed: ");
	// FINISH DRAWING OBJECTS HERE

	HR(g_app->m_pD3DDev->EndScene(),L"renderer::RenderFrame: EndScene failed: ");
	// Present the backbuffer.
	HR(g_app->m_pD3DDev->Present(0, 0, 0, 0),L"renderer::RenderFrame: Present failed: ");
}