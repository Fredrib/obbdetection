// File:		sky.cpp
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com
// Origional code taken from F. Luna's book.

//=============================================================================
// Sky.cpp by Frank Luna (C) 2004 All Rights Reserved.
//=============================================================================

#include "main.h"
sky::sky()
{
	mSphere = 0;
	mRadius = 1.0f;
	mEnvMap = NULL;
	m_FX = NULL;
	mhTech = 0;
	mhEnvMap = 0;
	mhWVP = 0;
}

void sky::InitSky(const std::wstring& envmapFilename, float skyRadius)
{
	mRadius = skyRadius;
	HR(D3DXCreateSphere(g_app->m_pD3DDev, skyRadius, 30, 30, &mSphere, 0),L"sky::sky: Failed to create sky sphere: ");
	HR(D3DXCreateCubeTextureFromFile(g_app->m_pD3DDev, envmapFilename.c_str(), &mEnvMap),
		L"sky::sky: Failed to create sky texture from file: ");

	LPCWSTR shaderFile;
	switch(g_renderer->m_shaderVersion)
	{
	case 2: shaderFile = L"Shaders/sky_2_0.fx"; break;
	case 3: shaderFile = L"Shaders/sky_3_0.fx"; break;
	default: throw std::runtime_error("buildFX: m_shaderVersion must be 2 or 3."); break;
	}
	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectFromFileEx(g_app->m_pD3DDev, shaderFile, 0, 0, 0, 0, 0, &m_FX, &errors),
		L"sky::sky: Failed to create sky effects: ");
	if( errors )
	{
		MessageBox(0, toWideString((char*)errors->GetBufferPointer(),-1).c_str(), 0, 0);
		throw std::runtime_error("sky::sky: errors exist in initializing sky");
	}

	mhTech   = m_FX->GetTechniqueByName("SkyTech");
	mhWVP    = m_FX->GetParameterByName(0, "gWVP");
	mhEnvMap = m_FX->GetParameterByName(0, "gEnvMap");

	// Set effect parameters that do not vary.
	HR(m_FX->SetTechnique(mhTech),L"sky::sky: Failed to set sky technique: ");
	HR(m_FX->SetTexture(mhEnvMap, mEnvMap),L"sky::sky: Failed to set sky texture: ");
}

sky::~sky()
{
	ReleaseCOM(mSphere);
	ReleaseCOM(mEnvMap);
	ReleaseCOM(m_FX);
}

DWORD sky::getNumTriangles()
{
	return mSphere->GetNumFaces();
}

DWORD sky::getNumVertices()
{
	return mSphere->GetNumVertices();
}

IDirect3DCubeTexture9* sky::getEnvMap()
{
	return mEnvMap;
}

float sky::getRadius()
{
	return mRadius;
}

void sky::onLostDevice()
{
	HR(m_FX->OnLostDevice(),L"sky::onLostDevice: Failed: ");
}

void sky::onResetDevice()
{
	HR(m_FX->OnResetDevice(),L"sky::onResetDevice: Failed: ");
}

void sky::draw()
{
	// sky always centered about camera's position.
	D3DXMATRIX W;
	D3DXVECTOR3 p = g_camera->GetEyePt();
	D3DXMatrixTranslation(&W, p.x, p.y, p.z);
	HR(m_FX->SetMatrix(mhWVP, &(W*g_camera->m_ViewProj)),L"sky::draw: SetMatrix failed: ");
	
	UINT numPasses = 0;
	HR(m_FX->Begin(&numPasses, 0),L"sky::draw: Begin failed: ");
	HR(m_FX->BeginPass(0),L"sky::draw: BeginPass failed: ");
	HR(mSphere->DrawSubset(0),L"sky::draw: DrawSubset failed: ");
	HR(m_FX->EndPass(),L"sky::draw: EndPass failed: ");
	HR(m_FX->End(),L"sky::draw: End failed: ");
}