// File:		hud.h
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

// Class to handle rendering of hud display.  Copied and modified from F Luna's source code (GfxStats.h)
#include "main.h"

#ifndef hud_h
#define hud_h

class hud
{
public:
	hud();
	~hud();

	void InitHud();

	void onLostDevice();
	void onResetDevice();

	void addVertices(DWORD n);
	void subVertices(DWORD n);
	void addTriangles(DWORD n);
	void subTriangles(DWORD n);

	void setTriCount(DWORD n);
	void setVertexCount(DWORD n);

	void update(float dt);
	void display();
	void RenderProgressText(int curObbox, int totalObboxes );

private:
	// Prevent copying
	hud(const hud& rhs);
	hud& operator=(const hud& rhs);
	
private:
	ID3DXFont* mFont;
	float mFPS;
	float mMilliSecPerFrame;
	DWORD mNumTris;
	DWORD mNumVertices;
};
#endif // hud_h