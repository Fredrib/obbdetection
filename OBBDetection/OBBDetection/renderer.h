/*************************************************************
**					OBBDetection Renderer					**
**		-> Graphics Rendering functions, Summer 2009		**
*************************************************************/
// File:		renderer.h
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

#ifndef renderer_h
#define renderer_h

#include	"main.h"
#include	"DrawableTex2D.h"
#include	"obbox.h"
#include	"debug_object.h"

class debug_object;

class renderer
{
public:
								renderer(void); // Default Constructor
								~renderer(void); // Destructor Destructor

	void						InitRenderer(void);
	void						RenderFrame(void);
	void						buildFX(void);
	void						onResetDevice(void);
	void						onLostDevice(void);
	void						drawShadowMap(void);
	void						InitWireframeRendering(void);
	void						DrawWireframe(IDirect3DVertexBuffer9 * vertBuff, DWORD vertSize, IDirect3DIndexBuffer9 * indBuff, DWORD indSize, D3DXMATRIX * mat);
	void						DrawWireframeLines(IDirect3DVertexBuffer9 * vertBuff, DWORD vertSize, IDirect3DIndexBuffer9 * indBuff, DWORD indSize, D3DXMATRIX * mat);
		// Next 2 are for color wireframe rendering (added later)
	void						DrawWireframeCol(IDirect3DVertexBuffer9 * vertBuff, DWORD vertSize, IDirect3DIndexBuffer9 * indBuff, DWORD indSize, D3DXMATRIX * mat);
	void						DrawWireframeColLines(IDirect3DVertexBuffer9 * vertBuff, DWORD vertSize, IDirect3DIndexBuffer9 * indBuff, DWORD indSize, D3DXMATRIX * mat);

	// D3D Effects - less overhead if public
	ID3DXEffect*				m_FX;
	ID3DXEffect*				m_wireFrameFX;
	ID3DXEffect*				m_wireFrameColFX;
	int							m_shaderVersion;

	// FX Handles - less overhead if public
	D3DXHANDLE					m_hTech,
								m_hBuildShadowMapTech,
								m_hLightWVP, 
								m_hWVP, 
								m_hWorld, 
								m_hMtrl, 
								m_hLight, 
								m_hEyePosW, 
								m_hTex, 
								m_hShadowMap,
								m_hShadowMapSize,
								m_hWireFrame,
								m_hWireFrameCol,
								m_hWireFrameWVP,
								m_hWireFrameColWVP;

	IDirect3DTexture9*			m_WhiteTex;

	debug_object *				debug_objects;

private:
	D3DXMATRIX					m_matProjection; // projection matrix
	float						m_fAspectRatio, // viewport ratio
								m_fFieldOfView, // view angle
								m_fNearPlane, // near clipping plane
								m_fFarPlane; // far clipping plane
	DrawableTex2D*				m_ShadowMap;
};

#endif