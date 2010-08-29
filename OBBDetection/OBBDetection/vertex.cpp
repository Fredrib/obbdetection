//=============================================================================
// Vertex.h by Frank Luna (C) 2005 All Rights Reserved.
//=============================================================================

#include "main.h"

// Initialize static variables.
IDirect3DVertexDeclaration9* VertexPNT::Decl = 0;
IDirect3DVertexDeclaration9* VertexPos::Decl = 0;
IDirect3DVertexDeclaration9* VertexPosCol::Decl = 0;

void InitAllVertexDeclarations()
{

	//===============================================================
	// VertexPNT

	D3DVERTEXELEMENT9 VertexPNTElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};	
	HR(g_app->m_pD3DDev->CreateVertexDeclaration(VertexPNTElements, &VertexPNT::Decl),
		L"InitAllVertexDeclarations: CreateVertexDeclaration for PNT elements failed: ");

	//===============================================================
	// VertexPos

	D3DVERTEXELEMENT9 VertexPosElements[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		D3DDECL_END()
	};	
	HR(g_app->m_pD3DDev->CreateVertexDeclaration(VertexPosElements, &VertexPos::Decl),
		L"InitAllVertexDeclarations: CreateVertexDeclaration for Pos elements failed: ");

	//===============================================================
	// VertexPosCol

	D3DVERTEXELEMENT9 VertexPosColElements[] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		D3DDECL_END()
	};
	HR(g_app->m_pD3DDev->CreateVertexDeclaration(VertexPosColElements, &VertexPosCol::Decl),
		L"InitAllVertexDeclarations: CreateVertexDeclaration for PosCol elements failed: ");

}

void DestroyAllVertexDeclarations()
{
	ReleaseCOM(VertexPNT::Decl);
	ReleaseCOM(VertexPos::Decl);
	ReleaseCOM(VertexPosCol::Decl);
}