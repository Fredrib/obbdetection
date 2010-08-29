// File:		sky.h
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com
// Origional code taken from F. Luna's book.

// NICE SKYBOX TUTORIAL USING TERAGEN:
// Can be found at: "OBBDetection_references_&_installs\BenMcDuff TG2 tutorial.pdf"

// SOME SKYBOX TEXTURES:
// http://www.humus.name/index.php?page=Textures&&start=0 Then use the directx tool to make .dds file (after de-resing in ifranview)
// http://www.hazelwhorley.com/textures.html

#ifndef sky_h
#define sky_h

#include "main.h"

//=============================================================================
// Sky.h by Frank Luna (C) 2004 All Rights Reserved.
//=============================================================================

class sky
{
public:
	sky();
	~sky();

	void InitSky(const std::wstring& envmapFilename, float skyRadius);

	IDirect3DCubeTexture9* getEnvMap();
	float getRadius();

	DWORD getNumTriangles();
	DWORD getNumVertices();

	void onLostDevice();
	void onResetDevice();

	void draw();

private:
	ID3DXMesh* mSphere;
	float mRadius;
	IDirect3DCubeTexture9* mEnvMap;
	ID3DXEffect* m_FX;
	D3DXHANDLE mhTech;
	D3DXHANDLE mhEnvMap;
	D3DXHANDLE mhWVP;
};

#endif
