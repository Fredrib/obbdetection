/*************************************************************
**					OBBDetection Renderer					**
**		-> Graphics Rendering functions, Summer 2009		**
*************************************************************/
// File:		renderer.cpp
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

// Some free models: http://artist-3d.com/free_3d_models/

#include	"renderer.h"

/************************************************************************/
/* Name:		renderer												*/
/* Description:	Default Constructor: Set render constants				*/
/************************************************************************/
renderer::renderer(void)
{
	m_fFieldOfView = D3DX_PI / 4.0f; // view angle
	m_fNearPlane = 1.0f; // near clipping plane
	m_fFarPlane = 1000.0f; // far clipping plane
	m_fAspectRatio = (float)g_app->m_dwWidth / (float)g_app->m_dwHeight;
	m_ShadowMap = NULL; m_FX = NULL; m_WhiteTex = NULL; m_wireFrameFX = NULL; m_wireFrameColFX = NULL;
	debug_objects = NULL;
}
/************************************************************************/
/* Name:		~renderer												*/
/* Description:	Default Destructor										*/
/************************************************************************/
renderer::~renderer(void)
{
	if(debug_objects)	{delete debug_objects; debug_objects = NULL;} // Recursively delete the linked list
	if(m_ShadowMap)		{delete m_ShadowMap; m_ShadowMap = NULL;}
	ReleaseCOM(m_FX);
	ReleaseCOM(m_wireFrameFX);
	ReleaseCOM(m_wireFrameColFX);
	ReleaseCOM(m_WhiteTex);
}
/************************************************************************/
/* Name:		InitRenderer											*/
/* Description:	set projection matrix, render states and texture states	*/
/************************************************************************/
void renderer::InitRenderer(void)
{
	// INITIALIZE THE WIREFRAME RENDERING --> USED MOSTLY FOR DEBUG
	InitWireframeRendering();

	// LOAD IN THE SKY MODELS
	g_sky->InitSky(L"models/lostvalley.dds", 10000.0f);
	HR(D3DXCreateTextureFromFile(g_app->m_pD3DDev, L"models/whitetex.dds", &m_WhiteTex),L"InitRenderer: Failed to load whitetex.dds: ");
 
	// LOAD IN THE MODELS
	//g_rbobjects[0]->Create(L"models/BasicColumnScene", g_app->m_pD3DDev);
	g_rbobjects[0]->Create(L"models/ionic_temple", g_app->m_pD3DDev);
	D3DXQUATERNION temp; D3DXQuaternionRotationAxis(&temp, &(D3DXVECTOR3(1.0f,0.0f,0.0f)), D3DX_PI*+0.5f);
	g_rbobjects[0]->SetRotation(temp);
	g_rbobjects[0]->SetScale(5.0f);
	g_rbobjects[0]->immovable = true; // This is the floor

	g_rbobjects[1]->Create(L"models/bigship1", g_app->m_pD3DDev);
	g_rbobjects[1]->SetPosition(D3DXVECTOR3(-5.1f, 0.0f, -50.0f));
	g_rbobjects[1]->SetScale(0.65f);
	g_rbobjects[1]->SetLinearMomentum(D3DXVECTOR3(0.0f,7.5f,20.0f));
	g_rbobjects[1]->SetAngularMomentum(D3DXVECTOR3(0.0f,0.0f,0.0f));
	D3DXQuaternionRotationAxis(&temp, &(D3DXVECTOR3(0.0f,1.0f,0.0f)), D3DX_PI);
	g_rbobjects[1]->SetRotation(temp);

	// INITIALIZE THE LIGHT
	g_light[0].InitLightColor(D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f),	// ambient
							  D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f),	// diffuse
							  D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f));	// spec
	g_light[0].InitLightPos(D3DXVECTOR3(125.0f, 50.0f, 0.0f),	// position (formally 20.0, 10.0, 0.0)
							D3DXVECTOR3(0.0f, 0.0f, 0.0f),		// target
							D3DXVECTOR3(0.0f, 1.0f, 0.0f),		// up (inital value
							16.0f,								// power (formally 16.0f)
							D3DX_PI*0.25f);						// FOV

	// INITIALIZE THE SHADOW MAP
	D3DVIEWPORT9 vp = {0, 0, SMAP_SIZE, SMAP_SIZE, 0.0f, 1.0f};
	m_ShadowMap = new DrawableTex2D(SMAP_SIZE, SMAP_SIZE, 1, D3DFMT_R32F, true, D3DFMT_D24X8, vp, false);

	// LOAD IN THE .x (EFFECTS) FILES
	buildFX();
	float smap_size = (float)SMAP_SIZE;
	HR(m_FX->SetValue(m_hShadowMapSize, &smap_size, sizeof(float)),L"renderer::InitRenderer: Set Value m_hShadowMapSize Failed: ");

	// INITIALIZE PHYSICS ENGINE WITH NEW MODELS
	g_physics->InitPhysics();

	// PERFORM A RESET TO INITILIZE SOME OBJECTS
	onResetDevice();
}//InitScene

/************************************************************************/
/* Name:		onLostDevice & onResetDevice							*/
/* Description:	When device goes out of focus --> Fix it				*/
/************************************************************************/
void renderer::onLostDevice()
{
	g_sky->onLostDevice();
	m_ShadowMap->onLostDevice();
	HR(m_FX->OnLostDevice(),L"renderer::onLostDevice: m_FX->OnLostDevice() failed: ");
	HR(m_wireFrameFX->OnLostDevice(),L"renderer::onLostDevice: m_wireFrameFX->OnLostDevice() failed: ");
	HR(m_wireFrameColFX->OnLostDevice(),L"renderer::onLostDevice: m_wireFrameColFX->OnLostDevice() failed: ");
}

void renderer::onResetDevice()
{
	g_hud->onResetDevice();
	g_sky->onResetDevice();
	m_ShadowMap->onResetDevice();
	HR(m_FX->OnResetDevice(),L"renderer::onResetDevice: m_FX->OnResetDevice() failed: ");
	HR(m_wireFrameFX->OnResetDevice(),L"renderer::onResetDevice: m_wireFrameFX->OnResetDevice() failed: ");
	HR(m_wireFrameColFX->OnResetDevice(),L"renderer::onResetDevice: m_wireFrameColFX->OnResetDevice() failed: ");
}	

/************************************************************************/
/* Name:		buildFX													*/
/* Description:	Create the FX from a .fx file.							*/
/************************************************************************/
void renderer::buildFX()
{
	 // Create the FX from a .fx file.
	LPCWSTR shaderFile;
	if(g_app->m_Shadows)
	{
		switch(m_shaderVersion)
		{
		case 2: shaderFile = L"Shaders/LightShadow_2_0.fx"; break;
		case 3: shaderFile = L"Shaders/LightShadow_3_0.fx"; break;
		default: throw std::runtime_error("buildFX: m_shaderVersion must be 2 or 3."); break;
		}
	}
	else
	{
		switch(m_shaderVersion)
		{
		case 2: shaderFile = L"Shaders/LightShadow_2_0_noShadows.fx"; break;
		case 3: shaderFile = L"Shaders/LightShadow_3_0_noShadows.fx"; break;
		default: throw std::runtime_error("buildFX: m_shaderVersion must be 2 or 3."); break;
		}
	}
	ID3DXBuffer* errors = 0;
	if(FAILED(D3DXCreateEffectFromFile(g_app->m_pD3DDev, shaderFile, 0, 0, D3DXSHADER_DEBUG, 0, &m_FX, &errors)) || errors)
	{
		if( errors )
			MessageBox(0, toWideString((char*)errors->GetBufferPointer(),-1).c_str(), 0, 0);
		throw std::runtime_error("buildFX: D3DXCreateEffectFromFile failed");
	}

	// Obtain handles.
	m_hTech               = m_FX->GetTechniqueByName("LightShadowTech");
	m_hBuildShadowMapTech = m_FX->GetTechniqueByName("BuildShadowMapTech");
	m_hLightWVP           = m_FX->GetParameterByName(0, "gLightWVP");
	m_hWVP                = m_FX->GetParameterByName(0, "gWVP");
	m_hWorld              = m_FX->GetParameterByName(0, "gWorld");
	m_hMtrl               = m_FX->GetParameterByName(0, "gMtrl");
	m_hLight              = m_FX->GetParameterByName(0, "gLight");
	m_hEyePosW            = m_FX->GetParameterByName(0, "gEyePosW");
	m_hTex                = m_FX->GetParameterByName(0, "gTex");
	m_hShadowMap          = m_FX->GetParameterByName(0, "gShadowMap");
	m_hShadowMapSize      = m_FX->GetParameterByName(0, "gShadowMapSize");
}

/************************************************************************/
/* Name:		RenderFrame (+ helper funcitons)						*/
/* Description:	Render the scene										*/
/************************************************************************/
void renderer::RenderFrame(void)
{
	g_camera->Update();

	if(g_app->m_Shadows)
		drawShadowMap();
	else
		HR(g_app->m_pD3DDev->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0),L"renderer::RenderFrame: Failed to clear device: ");

	HR(g_app->m_pD3DDev->BeginScene(),L"renderer::RenderFrame: BeginScene failed: ");

	// DRAW ALL OBJECTS HERE
		g_sky->draw();
		HR(m_FX->SetTechnique(m_hTech),L"renderer::RenderFrame: SetTechnique(m_hTech) Failed: ");
	
		HR(m_FX->SetValue(m_hLight, &g_light[0].m_SpotLight, sizeof(SpotLight)),L"renderer::RenderFrame: Set Value m_hLight Failed: ");
		HR(m_FX->SetValue(m_hEyePosW, &g_camera->m_vEyePt, sizeof(D3DXVECTOR3)),L"renderer::RenderFrame: Set Value m_hEyePosW Failed: ");
		HR(m_FX->SetTexture(m_hShadowMap, m_ShadowMap->d3dTex()),L"renderer::RenderFrame: Set Value m_hShadowMap Failed: ")

		UINT numPasses = 0;
		HR(m_FX->Begin(&numPasses, 0),L"renderer::RenderFrame: m_FX->Begin Failed: ");
		HR(m_FX->BeginPass(0),L"renderer::RenderFrame: m_FX->BeginPass Failed: ");

		for(int i = 0; i < NO_RBOBJECTS; i++)
		{
			g_rbobjects[i]->Render();

			if(g_rbobjects[i]->OBBTree) // If a tree exists
			{
				obbox * obbToRender = g_rbobjects[i]->OBBTree->GetObbToRender();
				if(obbToRender) // just to catch errors
				{
					if(g_app->m_drawConvexHull)
						obbToRender->DrawConvexHull( &(g_rbobjects[i]->matWorld * g_camera->m_ViewProj) ); // Start recursive procedure at tree base
					if(g_app->m_drawOBBTree)
					{
						obbToRender->DrawOBB( &(g_rbobjects[i]->matWorld * g_camera->m_ViewProj) );
					}
				}
				else
					throw std::runtime_error("RenderFrame:: Trying to draw an uninitialized OBBTree");
			}
		}

		// Draw the debug objects
		if(debug_objects)
			debug_objects->Render(); // Recursively draw the debug_objects linked-list.

		HR(m_FX->EndPass(),L"renderer::RenderFrame: m_FX->EndPass Failed: ");
		HR(m_FX->End(),L"renderer::RenderFrame: m_FX->End Failed: ");

		// Render the HUD
		g_hud->display();

	// FINISH DRAWING OBJECTS HERE

	HR(g_app->m_pD3DDev->EndScene(),L"renderer::RenderFrame: EndScene failed: ");

	// Present the backbuffer.
	HR(g_app->m_pD3DDev->Present(0, 0, 0, 0),L"renderer::RenderFrame: Present failed: ");

}//RenderFrame
void rboMesh::Render(void)
{
	if(!pMesh)
		throw std::runtime_error("Render: Trying to render a mesh that isn't initialized"); 
	// UpdateMatricies() already called during shadow map rendering (no need to do it again)!

	// Setup the matricies
	HR(g_renderer->m_FX->SetMatrix(g_renderer->m_hLightWVP, &(g_light[0].m_LightVP)), // matWorld is current RBO model->world transform
		L"rboMesh::Render: Failed to set mhLightWVP matrix: ");
	HR(g_renderer->m_FX->SetMatrix(g_renderer->m_hWVP, &(matWorld * g_camera->m_ViewProj)),
		L"rboMesh::Render: Failed to set mhWVP matrix: ");
	HR(g_renderer->m_FX->SetMatrix(g_renderer->m_hWorld, &matWorld),
		L"rboMesh::Render: Failed to set mhWorld matrix: ");

	// Set the textures and draw the subsets
	for(UINT j = 0; j < materials.size(); ++j)
	{
		HR(g_renderer->m_FX->SetValue(g_renderer->m_hMtrl, &materials[j], sizeof(D3DMATERIAL9)),L"rboMesh::Render: Failed to set value m_hMtrl: ");
	
		if(textures[j] != 0) {
			HR(g_renderer->m_FX->SetTexture(g_renderer->m_hTex, textures[j]),L"rboMesh::Render: Failed to set a texture: ");
		} else {
			HR(g_renderer->m_FX->SetTexture(g_renderer->m_hTex, g_renderer->m_WhiteTex),L"rboMesh::Render: Failed to set the white: ");
		}
	
		HR(g_renderer->m_FX->CommitChanges(),L"rboMesh::Render: CommitChanges failed: ");

		if(g_app->m_drawModels)
		{
			HR(pMesh->DrawSubset(j),L"rboMesh::Render: Failed to draw subset: ");
		}
	}		

}//rboMesh::Render

/************************************************************************/
/* Name:		drawShadowMap (+ helper funcitons)						*/
/* Description:	Render scene from the light perspective as shadow map	*/
/************************************************************************/
void renderer::drawShadowMap()
{
	m_ShadowMap->beginScene();
	HR(g_app->m_pD3DDev->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0),L"renderer::drawShadowMap: Failed to clear device: ");
	
	HR(m_FX->SetTechnique(m_hBuildShadowMapTech),L"renderer::drawShadowMap: Failed to set shadow map technique: ");

	UINT numPasses = 0;
	HR(m_FX->Begin(&numPasses, 0),L"renderer::drawShadowMap: m_FX->Begin() failed: ");
	HR(m_FX->BeginPass(0),L"renderer::drawShadowMap: m_FX->BeginPass() failed: ");

	// DRAW ALL OBJECTS HERE
		for(int i = 0; i < NO_RBOBJECTS; i++)
		{
			if(g_app->m_drawModels)
			{
				g_rbobjects[i]->RenderSM();
			}
		}

	// FINISH DRAWING OBJECTS HERE

	HR(m_FX->EndPass(),L"renderer::drawShadowMap: EndPass() failed: ");
	HR(m_FX->End(),L"renderer::drawShadowMap: End() failed: ");

	m_ShadowMap->endScene();
}//drawShadowMap
void rboMesh::RenderSM(void) // Render during shadow map
{
	if(!pMesh)
		throw std::runtime_error("rboMesh::RenderSM: Trying to render a mesh that isn't initialized"); 
	UpdateMatricies();

	HR(g_renderer->m_FX->SetMatrix(g_renderer->m_hLightWVP, &(matWorld * g_light[0].m_LightVP)), // matWorld is current RBO model->world transform
		L"rboMesh::RenderSM: Failed to set m_hLightWVP matrix: "); 
	HR(g_renderer->m_FX->CommitChanges(),L"rboMesh::RenderSM: CommitChanges failed: ");
	for(UINT j = 0; j < materials.size(); ++j)
	{
		HR(pMesh->DrawSubset(j),L"rboMesh::RenderSM: DrawSubset failed: ");
	}

}//rboMesh::RenderSM
/************************************************************************/
/* Name:		InitWireframeRendering									*/
/* Description: Initialize rendering for wireframes						*/
/************************************************************************/
void renderer::InitWireframeRendering(void)
{
	LPCWSTR shaderFile;
	LPCWSTR shaderFile_col; // Color wireframe renderer
	switch(g_renderer->m_shaderVersion)
	{
	case 2: shaderFile = L"Shaders/wireFrame_2_0.fx"; shaderFile_col = L"Shaders/wireFrameCol_2_0.fx"; break;
	case 3: shaderFile = L"Shaders/wireFrame_3_0.fx"; shaderFile_col = L"Shaders/wireFrameCol_3_0.fx"; break;
	default: throw std::runtime_error("InitWireframeRendering: m_shaderVersion must be 2 or 3."); break;
	}

	// WIREFRAME WITHOUT COLORS
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFileEx(g_app->m_pD3DDev, shaderFile, 0, 0, 0, 0, 0, &m_wireFrameFX, &errors),
		L"InitWireframeRendering: Failed to create Wireframe effects FX: ");
	if( errors )
	{
		MessageBox(0, toWideString((char*)errors->GetBufferPointer(),-1).c_str(), 0, 0);
		throw std::runtime_error("InitWireframeRendering: errors exist in initializing Wireframe");
	}

	m_hWireFrame = m_wireFrameFX->GetTechniqueByName("WireFrameTech");
	m_hWireFrameWVP  = m_wireFrameFX->GetParameterByName(0, "gWVP");

	// Set effect parameters that do not vary.
	HR(m_wireFrameFX->SetTechnique(m_hWireFrame),L"InitWireframeRendering: Failed to set wireFrame technique: ");

	// WIREFRAME WITH COLORS
	errors = 0;
	HR(D3DXCreateEffectFromFileEx(g_app->m_pD3DDev, shaderFile_col, 0, 0, 0, 0, 0, &m_wireFrameColFX, &errors),
		L"InitWireframeRendering: Failed to create WireframeCol effects FX: ");
	if( errors )
	{
		MessageBox(0, toWideString((char*)errors->GetBufferPointer(),-1).c_str(), 0, 0);
		throw std::runtime_error("InitWireframeRendering: errors exist in initializing Color Wireframe");
	}

	m_hWireFrameCol = m_wireFrameColFX->GetTechniqueByName("WireFrameColTech");
	m_hWireFrameColWVP  = m_wireFrameColFX->GetParameterByName(0, "gWVP");

	// Set effect parameters that do not vary.
	HR(m_wireFrameColFX->SetTechnique(m_hWireFrameCol),L"InitWireframeRendering: Failed to set wireFrameCol technique: ");
}
/************************************************************************/
/* Name:		DrawWireframe											*/
/* Description: Draw a wireframe										*/
/************************************************************************/
void renderer::DrawWireframe(IDirect3DVertexBuffer9 * vertBuff, DWORD vertSize, IDirect3DIndexBuffer9 * indBuff, DWORD indSize, D3DXMATRIX * mat)
{
	HR(g_app->m_pD3DDev->SetStreamSource(0, vertBuff, 0, sizeof(VertexPos)),L"DrawWireframe: SetStreamSource failed: ");
	HR(g_app->m_pD3DDev->SetIndices(indBuff),L"DrawWireframe: SetIndices failed: ");
	HR(g_app->m_pD3DDev->SetVertexDeclaration(VertexPos::Decl),L"DrawWireframe: SetVertexDeclaration failed: ");

	// Setup the rendering FX
	UINT numPasses = 0;
	HR(g_renderer->m_wireFrameFX->Begin(&numPasses, 0),L"DrawWireframe: Begin failed: ");
	HR(g_renderer->m_wireFrameFX->BeginPass(0),L"DrawWireframe: BeginPass failed: ");

	HR(g_renderer->m_wireFrameFX->SetMatrix(g_renderer->m_hWireFrameWVP, mat),
		L"DrawWireframe: SetMatrix failed: ");
	HR(g_renderer->m_wireFrameFX->CommitChanges(),L"DrawWireframe: CommitChanges failed: ");
	HR(g_app->m_pD3DDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertSize, 0, indSize/3),
		L"DrawWireframe: DrawIndexedPrimitive failed: ");

	HR(g_renderer->m_wireFrameFX->EndPass(),L"DrawWireframe: EndPass failed: ");
	HR(g_renderer->m_wireFrameFX->End(),L"DrawWireframe: End failed: ");
}
/************************************************************************/
/* Name:		DrawWireframeCol										*/
/* Description: Draw a color wireframe									*/
/************************************************************************/
void renderer::DrawWireframeCol(IDirect3DVertexBuffer9 * vertBuff, DWORD vertSize, IDirect3DIndexBuffer9 * indBuff, DWORD indSize, D3DXMATRIX * mat)
{
	HR(g_app->m_pD3DDev->SetStreamSource(0, vertBuff, 0, sizeof(VertexPosCol)),L"DrawWireframeCol: SetStreamSource failed: ");
	HR(g_app->m_pD3DDev->SetIndices(indBuff),L"DrawWireframeCol: SetIndices failed: ");
	HR(g_app->m_pD3DDev->SetVertexDeclaration(VertexPosCol::Decl),L"DrawWireframeCol: SetVertexDeclaration failed: ");

	// Setup the rendering FX
	UINT numPasses = 0;
	HR(g_renderer->m_wireFrameColFX->Begin(&numPasses, 0),L"DrawWireframeCol: Begin failed: ");
	HR(g_renderer->m_wireFrameColFX->BeginPass(0),L"DrawWireframeCol: BeginPass failed: ");

	HR(g_renderer->m_wireFrameColFX->SetMatrix(g_renderer->m_hWireFrameColWVP, mat),
		L"DrawWireframeCol: SetMatrix failed: ");
	HR(g_renderer->m_wireFrameColFX->CommitChanges(),L"DrawWireframeCol: CommitChanges failed: ");
	HR(g_app->m_pD3DDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertSize, 0, indSize/3),
		L"DrawWireframeCol: DrawIndexedPrimitive failed: ");

	HR(g_renderer->m_wireFrameColFX->EndPass(),L"DrawWireframeCol: EndPass failed: ");
	HR(g_renderer->m_wireFrameColFX->End(),L"DrawWireframeCol: End failed: ");
}
/************************************************************************/
/* Name:		DrawWireframeLines										*/
/* Description: Draw a wireframe using D3DPT_LINELIST primatives		*/
/************************************************************************/
void renderer::DrawWireframeLines(IDirect3DVertexBuffer9 * vertBuff, DWORD vertSize, IDirect3DIndexBuffer9 * indBuff, DWORD indSize, D3DXMATRIX * mat)
{
	HR(g_app->m_pD3DDev->SetStreamSource(0, vertBuff, 0, sizeof(VertexPos)),L"DrawWireframeLines: SetStreamSource failed: ");
	HR(g_app->m_pD3DDev->SetIndices(indBuff),L"DrawWireframeLines: SetIndices failed: ");
	HR(g_app->m_pD3DDev->SetVertexDeclaration(VertexPos::Decl),L"DrawWireframeLines: SetVertexDeclaration failed: ");

	// Setup the rendering FX
	UINT numPasses = 0;
	HR(g_renderer->m_wireFrameFX->Begin(&numPasses, 0),L"DrawWireframeLines: Begin failed: ");
	HR(g_renderer->m_wireFrameFX->BeginPass(0),L"DrawWireframeLines: BeginPass failed: ");

	HR(g_renderer->m_wireFrameFX->SetMatrix(g_renderer->m_hWireFrameWVP, mat),
		L"DrawWireframeLines: SetMatrix failed: ");
	HR(g_renderer->m_wireFrameFX->CommitChanges(),L"DrawWireframeLines: CommitChanges failed: ");
	HR(g_app->m_pD3DDev->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, vertSize, 0, indSize/2),
		L"DrawWireframeLines: DrawIndexedPrimitive failed: ");

	HR(g_renderer->m_wireFrameFX->EndPass(),L"DrawWireframeLines: EndPass failed: ");
	HR(g_renderer->m_wireFrameFX->End(),L"DrawWireframeLines: End failed: ");
}
/************************************************************************/
/* Name:		DrawWireframeColLines										*/
/* Description: Draw a color wireframe using D3DPT_LINELIST primatives	*/
/************************************************************************/
void renderer::DrawWireframeColLines(IDirect3DVertexBuffer9 * vertBuff, DWORD vertSize, IDirect3DIndexBuffer9 * indBuff, DWORD indSize, D3DXMATRIX * mat)
{
	HR(g_app->m_pD3DDev->SetStreamSource(0, vertBuff, 0, sizeof(VertexPosCol)),L"DrawWireframeColLines: SetStreamSource failed: ");
	HR(g_app->m_pD3DDev->SetIndices(indBuff),L"DrawWireframeColLines: SetIndices failed: ");
	HR(g_app->m_pD3DDev->SetVertexDeclaration(VertexPosCol::Decl),L"DrawWireframeColLines: SetVertexDeclaration failed: ");

	// Setup the rendering FX
	UINT numPasses = 0;
	HR(g_renderer->m_wireFrameColFX->Begin(&numPasses, 0),L"DrawWireframeColLines: Begin failed: ");
	HR(g_renderer->m_wireFrameColFX->BeginPass(0),L"DrawWireframeColLines: BeginPass failed: ");

	HR(g_renderer->m_wireFrameColFX->SetMatrix(g_renderer->m_hWireFrameColWVP, mat),
		L"DrawWireframeColLines: SetMatrix failed: ");
	HR(g_renderer->m_wireFrameColFX->CommitChanges(),L"DrawWireframeColLines: CommitChanges failed: ");
	HR(g_app->m_pD3DDev->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, vertSize, 0, indSize/2),
		L"DrawWireframeColLines: DrawIndexedPrimitive failed: ");

	HR(g_renderer->m_wireFrameColFX->EndPass(),L"DrawWireframeColLines: EndPass failed: ");
	HR(g_renderer->m_wireFrameColFX->End(),L"DrawWireframeColLines: End failed: ");
}