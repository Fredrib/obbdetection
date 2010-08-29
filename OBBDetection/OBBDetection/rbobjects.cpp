/*************************************************************
**					Rigid Body Object						**
**			-> Store object states, Summer 2009				**
*************************************************************/
// File:		rbobject.cpp
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

// http://eta.physics.uoguelph.ca/tutorials/torque/Q.torque.inertia.html
// & http://www.gamedev.net/community/forums/topic.asp?topic_id=57001

#include	"main.h"

/************************************************************************/
/* Name:		rbobject												*/
/* Description: Default Constructor										*/
/************************************************************************/
rbobject::rbobject(void)
{
	// Zero all vectors and positions.  Make matricies identity matrix
	mass = 1.0f;
	D3DXMatrixIdentity(&Ibody);
	D3DXMatrixIdentity(&Ibody_inv);
	x.x = 0; x.y = 0; x.z = 0; x_t1.x = 0; x_t1.y = 0; x_t1.z = 0; 
	P.x = 0; P.y = 0; P.z = 0; P_t1.x = 0; P_t1.y = 0; P_t1.z = 0;
	L.x = 0; L.y = 0; L.z = 0; L_t1.x = 0; L_t1.y = 0; L_t1.z = 0;
	
	//q.w = 0; q.x = 0; q.y = 1; q.z = 0;
	q.w = 1; q.x = 0; q.y = 0; q.z = 0;   // --> Force all objects to start with R = indentity matrix

	q_t1.w = 0; q_t1.x = 0; q_t1.y = 0; q_t1.z = 0;
	D3DXMatrixIdentity(&Iinv);
	scale = 1.0f;
	v.x = 0; v.y = 0; v.z = 0;
	omega.x = 0; omega.y = 0; omega.z = 0;
	force.x = 0; force.y = 0; force.z = 0;
	torque.x = 0; torque.y = 0; torque.z = 0;
	dirtyScaleMatrix = true; dirtyRotMatrix = true; dirtyTransMatrix = true;
	immovable = false;
	vertexBuffer = NULL; indexBuffer = NULL; numFaces = 0; OBBTree = NULL;
	UpdateMatricies();
}
/************************************************************************/
/* Name:		UpdateBoundingBox										*/
/* Description:	Update bounding box bounds for changing world conditions*/
/************************************************************************/
void rbobject::UpdateBoundingBox(void)
{
	//http://www.toymaker.info/Games/html/collisions.html
	UpdateMatricies();
	// Get bounding box in world coordinates
	for( int i = 0; i < 8; i++ )
		D3DXVec3TransformCoord( &m_worldBounds[i], &m_objectBounds[i], &matWorld );

	// Now get bounding box in axis aligned world coordinates --> Box area will grow.
	m_minAABB.x = m_worldBounds[0].x; m_minAABB.y = m_worldBounds[0].y; m_minAABB.z = m_worldBounds[0].z;
	m_maxAABB.x = m_worldBounds[0].x; m_maxAABB.y = m_worldBounds[0].y; m_maxAABB.z = m_worldBounds[0].z;
	// Due to rotations, don't know which verticies will give minimum and maximum
	for (int i=1;i<8;i++)
	{
		   if (m_worldBounds[i].x < m_minAABB.x) m_minAABB.x=m_worldBounds[i].x;
		   if (m_worldBounds[i].x > m_maxAABB.x) m_maxAABB.x=m_worldBounds[i].x;
		   if (m_worldBounds[i].y < m_minAABB.y) m_minAABB.y=m_worldBounds[i].y;
		   if (m_worldBounds[i].y > m_maxAABB.y) m_maxAABB.y=m_worldBounds[i].y;
		   if (m_worldBounds[i].z < m_minAABB.z) m_minAABB.z=m_worldBounds[i].z;
		   if (m_worldBounds[i].z > m_maxAABB.z) m_maxAABB.z=m_worldBounds[i].z;
	}
}
/************************************************************************/
/* Name:		UpdateMatricies											*/
/* Description:	Update matricies if required							*/
/************************************************************************/
void rbobject::UpdateMatricies(void)
{
	bool dirtyWorldMatrix = false;
	if(dirtyScaleMatrix)
	{
		D3DXMatrixScaling( &scaleMat, scale, scale, scale );
		dirtyScaleMatrix = false;
		dirtyWorldMatrix = true;
	}
	if(dirtyRotMatrix)
	{
		D3DXMatrixRotationQuaternion( &R , &q );
		dirtyRotMatrix = false;
		dirtyWorldMatrix = true;
	}
	if(dirtyTransMatrix)
	{
		D3DXMatrixTranslation( &Trans, x.x, x.y, x.z );
		dirtyTransMatrix = false;
		dirtyWorldMatrix = true;
	}
	if(dirtyWorldMatrix)
	{
		matWorld = scaleMat * R * Trans;
	}
}
/************************************************************************/
/* Name:		rboMesh													*/
/* Description:	Default constructor										*/
/************************************************************************/
rboMesh::rboMesh(void)
{
	// Remember Parent constructor is called first.
	pMesh = NULL; pDeclaration = NULL;
}
/************************************************************************/
/* Name:		rboMesh													*/
/* Description:	Default destructor										*/
/************************************************************************/
rboMesh::~rboMesh(void)
{
	// Release all mesh components
	materials.clear();
	for (int i = 0; i < static_cast<int>(textures.size()); ++i)
    {
        ReleaseCOM(textures[i]);
    }
    textures.clear();
    ReleaseCOM(pMesh);
	ReleaseCOM(pDeclaration);
	if(vertexBuffer)
	{
		delete [] vertexBuffer;
		vertexBuffer = NULL;
	}
	if(indexBuffer)
	{
		delete [] indexBuffer;
		indexBuffer = NULL;
	}
	if(OBBTree)
	{
		delete OBBTree;
		OBBTree = NULL;
	}
}
/************************************************************************/
/* Name:		Create													*/
/* Description:	This function is taken from (and VERY heavily modified) */
/*				http://www.dhpoware.com/demos/							*/
/************************************************************************/
void rboMesh::Create(LPCWSTR Filename, IDirect3DDevice9 *pDevice)
{
	// LOAD IN THE .X FILE DATA
	std::wstring xFilename = Filename; xFilename.append(L".x");
	LoadXFile(xFilename.c_str(),&pMesh, materials, textures);

	// Add verticies and triangles to statistics
	g_hud->addVertices(pMesh->GetNumVertices());
	g_hud->addTriangles(pMesh->GetNumFaces());

	// COMPUTE MASS PROPERTIES
	// Normalize();
	GetVertexIndexData(); // Not centered about COM
	ComputeMassProperties();
	GetVertexIndexData(); // Centered about COM --> Do again

	// CREATE THE OBB TREE FROM DISK OR FROM SCRATCH
	OBBTree = new obbtree(numFaces, numVerticies);
	OBBTree->InitOBBTree(this, Filename);
}

// The methods bounds(), normalize(), and scale() are based on source code
// from http://www.mvps.org/directx/articles/scalemesh9.htm
// rboMesh::Bounds
// No longer in use
/*void rboMesh::Bounds(D3DXVECTOR3 *pCenter, float *pRadius)
{
    if (!pMesh)
		throw std::runtime_error("Bounds: Mesh not initialized"); 
    
    BYTE *pBytes = 0;
    DWORD dwNumVertices = pMesh->GetNumVertices();
    DWORD dwVertexSize = D3DXGetFVFVertexSize(pMesh->GetFVF());

    HR(pMesh->LockVertexBuffer(0, reinterpret_cast<LPVOID*>(&pBytes)),L"Bounds: Failed to lock vertex buffer: "); 

    if(FAILED(D3DXComputeBoundingSphere( reinterpret_cast<D3DXVECTOR3*>(pBytes),
										 dwNumVertices, dwVertexSize, pCenter, pRadius)))
	{
		pMesh->UnlockVertexBuffer();
		throw std::runtime_error("Bounds: Failed to get bounding sphere"); 
	}

    HR(pMesh->UnlockVertexBuffer(),L"Bounds: Failed to unlock vertex buffer: ");
}*/

// rboMesh::Normalize
// No longer in use
/*void rboMesh::Normalize(float scaleTo, bool center)
{
    float radius;
    D3DXVECTOR3 centerPos;

	Bounds(&centerPos, &radius);

    float scalingFactor = scaleTo / radius;
    D3DXVECTOR3 offset = (center) ? -centerPos : D3DXVECTOR3(0.0f, 0.0f, 0.0f);

    return Scale(scalingFactor, &offset);
} */

// rboMesh::Scale
// No longer in use
/*void rboMesh::Scale(float Scale, D3DXVECTOR3 *pOffset)
{
    D3DXVECTOR3 offset = (pOffset) ? *pOffset : D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    DWORD dwNumVertices = pMesh->GetNumVertices();
    DWORD dwFVF = pMesh->GetFVF();
    DWORD dwVertexSize = D3DXGetFVFVertexSize(pMesh->GetFVF());
    BYTE *pBytes = 0;
    
    HR(pMesh->LockVertexBuffer(0, reinterpret_cast<LPVOID*>(&pBytes)),L"Scale: Failed to lock vertex buffer: ");
    
    D3DXVECTOR3 *pVector = 0;

    for (DWORD i = 0; i < dwNumVertices; ++i)
    {
        pVector = reinterpret_cast<D3DXVECTOR3*>(pBytes);

        *pVector += offset;
        
        pVector->x *= Scale;
        pVector->y *= Scale;
        pVector->z *= Scale;

        pBytes += dwVertexSize;
    }

    HR(pMesh->UnlockVertexBuffer(),L"Scale: Failed to unlock vertex buffer: ");
}*/ 
/************************************************************************/
/* Name:		ComputeMassProperties									*/
/* Description:	Calculate center of mass and inertia tensor matrix. I	*/
/*		use code at: http://www.melax.com/volint.html for simplicity.	*/
/*		ASSUMES THAT GetVertexIndexData HAS BEEN CALLED BEFORE			*/
/************************************************************************/
void rboMesh::ComputeMassProperties(void)
{
	// CONVERT VERTEX BUFFER TO TYPE USED BY S. MELAX SO WE CAN USE HIS CODE - O(n): but done on startup
	double3 * vert = new double3[numVerticies];
	for(DWORD i = 0; i < numVerticies; i ++)
	{
		vert[i].x =  vertexBuffer[i].x; 
		vert[i].y =  vertexBuffer[i].y; 
		vert[i].z =  vertexBuffer[i].z; 
	}

	// CONVERT INDEX BUFFER TO TYPE USED BY S. MELAX SO WE CAN USE HIS CODE - O(n): but done on startup
	int3 * ind = new int3[numFaces];
	for(DWORD i = 0; i < numFaces; i ++)
	{
		ind[i].x =  indexBuffer[3*i+0]; 
		ind[i].y =  indexBuffer[3*i+1];
		ind[i].z =  indexBuffer[3*i+2];
	}	

	// CALCULATE THE CENTER OF MASS -> USING S. MELAX CODE
	double3 COM = CenterOfMass(vert, ind, numFaces);
	// Now shift all verticies by this amount
	BYTE *pBytes = NULL;
	HR(pMesh->LockVertexBuffer(D3DLOCK_READONLY, reinterpret_cast<LPVOID*>(&pBytes)), L"ComputeMassProperties: Failed to lock vertex buffer: ");
	D3DXVECTOR3 *pVector = 0;
	DWORD dwVertexSize = D3DXGetFVFVertexSize(pMesh->GetFVF());
	for(DWORD i = 0; i < numVerticies; i ++)
	{
		pVector = reinterpret_cast<D3DXVECTOR3*>(pBytes);
		pVector->x -= (float)(COM.x); pVector->y -= (float)(COM.y); pVector->z -= (float)(COM.z);
		// Also shift vertex buffer (so it is up to date)
		vertexBuffer[i].x -= (float)(COM.x); vertexBuffer[i].y -= (float)(COM.y); vertexBuffer[i].z -= (float)(COM.z); 
		vert[i].x -= COM.x; vert[i].y -= COM.y; vert[i].z -= COM.z;
		pBytes += dwVertexSize;
	}
	HR(pMesh->UnlockVertexBuffer(), L"ComputeMassProperties: Failed to unlock vertex buffer: ");

	// Make sure COM now is zero!
	COM = CenterOfMass(vert, ind, numFaces);
	if(abs(COM.x) > 0.01f || abs(COM.y) > 0.01f || abs(COM.z) > 0.01f)
		throw std::runtime_error("ComputeMassProperties: Couldn't initialize object coordinate origion to center of mass");

	// GET BOUNDING BOX
	pBytes = 0;
	HR(pMesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&pBytes), L"ComputeMassProperties: Failed to lock vertex buffer: ");
	D3DXVECTOR3 minBounds,maxBounds;
	D3DXComputeBoundingBox((D3DXVECTOR3*)pBytes, pMesh->GetNumVertices(), D3DXGetFVFVertexSize(pMesh->GetFVF()), &minBounds, &maxBounds);
	HR(pMesh->UnlockVertexBuffer(), L"ComputeMassProperties: Failed to unlock vertex buffer: ");

	// We have min and max values, use these to get the 8 corners of the bounding box in object space
	m_objectBounds[0] = D3DXVECTOR3( minBounds.x, minBounds.y, minBounds.z ); // xyz
	m_objectBounds[1] = D3DXVECTOR3( maxBounds.x, minBounds.y, minBounds.z ); // Xyz
	m_objectBounds[2] = D3DXVECTOR3( minBounds.x, maxBounds.y, minBounds.z ); // xYz
	m_objectBounds[3] = D3DXVECTOR3( maxBounds.x, maxBounds.y, minBounds.z ); // XYz
	m_objectBounds[4] = D3DXVECTOR3( minBounds.x, minBounds.y, maxBounds.z ); // xyZ
	m_objectBounds[5] = D3DXVECTOR3( maxBounds.x, minBounds.y, maxBounds.z ); // XyZ
	m_objectBounds[6] = D3DXVECTOR3( minBounds.x, maxBounds.y, maxBounds.z ); // xYZ
	m_objectBounds[7] = D3DXVECTOR3( maxBounds.x, maxBounds.y, maxBounds.z ); // XYZ

	GetItensor(vert, ind, numFaces, &Ibody, &Ibody_inv);
/*
	// Test GetItensor with a cube (from <-1,-1,-1> --> <+1,+1,+1>)
	double3 * vertTest = new double3[8];
	int3 * indTest = new int3[12];
	vertTest[0].x = -2.25; vertTest[0].y = -2.25; vertTest[0].z = -2.25; 
	vertTest[1].x = +2.25; vertTest[1].y = -2.25; vertTest[1].z = -2.25;
	vertTest[2].x = +2.25; vertTest[2].y = +2.25; vertTest[2].z = -2.25;
	vertTest[3].x = -2.25; vertTest[3].y = +2.25; vertTest[3].z = -2.25;
	vertTest[4].x = -2.25; vertTest[4].y = -2.25; vertTest[4].z = +2.25;
	vertTest[5].x = +2.25; vertTest[5].y = -2.25; vertTest[5].z = +2.25;
	vertTest[6].x = +2.25; vertTest[6].y = +2.25; vertTest[6].z = +2.25;
	vertTest[7].x = -2.25; vertTest[7].y = +2.25; vertTest[7].z = +2.25;
	indTest[0].x = 0; indTest[0].y = 1; indTest[0].z = 3;
	indTest[1].x = 3; indTest[1].y = 1; indTest[1].z = 2;
	indTest[2].x = 0; indTest[2].y = 4; indTest[2].z = 1;
	indTest[3].x = 4; indTest[3].y = 5; indTest[3].z = 1;
	indTest[4].x = 1; indTest[4].y = 5; indTest[4].z = 2;
	indTest[5].x = 2; indTest[5].y = 5; indTest[5].z = 6;
	indTest[6].x = 6; indTest[6].y = 3; indTest[6].z = 2;
	indTest[7].x = 6; indTest[7].y = 7; indTest[7].z = 3;
	indTest[8].x = 7; indTest[8].y = 4; indTest[8].z = 3;
	indTest[9].x = 4; indTest[9].y = 0; indTest[9].z = 3;
	indTest[10].x = 7; indTest[10].y = 6; indTest[10].z = 5;
	indTest[11].x = 7; indTest[11].y = 5; indTest[11].z = 4;
	double3 COMTest = CenterOfMass(vertTest, indTest, 12);
	D3DXMATRIX IbodyTest; D3DXMATRIX IbodyTest_inv; 
	GetItensor(vertTest, indTest, 12, &IbodyTest, &IbodyTest_inv);
	// COM should be 0,0,0 --> Intertia Tensor should be diagonal with entries for a AxAxA box: 2*A^2/12
*/
	GetItensor(vert, ind, numFaces, &Ibody, &Ibody_inv);

	UpdateBoundingBox(); // Works out axis-aligned BB values --> Makes culling faster.

	// Clean up since we're done
	delete [] vert;
	delete [] ind;
}
/************************************************************************/
/* Name:		GetVertexIndexData										*/
/* Description:	Copy vertex and index buffer into local storage.		*/
/************************************************************************/
// Useful guide for acessing mesh data: http://www.mvps.org/directx/articles/d3dxmesh.htm
void rboMesh::GetVertexIndexData(void)
{
    numVerticies = pMesh->GetNumVertices();
	numFaces = pMesh->GetNumFaces();
    DWORD dwFVF = pMesh->GetFVF();
    DWORD dwVertexSize = D3DXGetFVFVertexSize(pMesh->GetFVF());
	int meshOptions = pMesh->GetOptions();
	bool indices32Bit = (meshOptions & D3DXMESH_32BIT) == D3DXMESH_32BIT; // Do a bit mask

	// Iterate through and copy mesh's vertex buffer into local data structure
	if(vertexBuffer != NULL)
		delete [] vertexBuffer; // Number of faces may have changed
	vertexBuffer = new D3DXVECTOR3[numVerticies];
	BYTE *pBytes = NULL;
	HR(pMesh->LockVertexBuffer(D3DLOCK_READONLY, reinterpret_cast<LPVOID*>(&pBytes)),L"GetVertexIndexData: Failed to lock vertex buffer: ");
    D3DXVECTOR3 *pVector = 0;
    for (DWORD i = 0; i < numVerticies; ++i)
    {
        pVector = reinterpret_cast<D3DXVECTOR3*>(pBytes);
		vertexBuffer[i].x = pVector->x;
		vertexBuffer[i].y = pVector->y;
		vertexBuffer[i].z = pVector->z;
        pBytes += dwVertexSize;
    }
    HR(pMesh->UnlockVertexBuffer(),L"GetVertexIndexData: Failed to unlock vertex buffer: ");

	// Iterate through and copy mesh's index buffer into local data structure
	if(indexBuffer != NULL)
		delete [] indexBuffer; // Number of faces may have changed
	indexBuffer = new int[numFaces*3]; 
	pBytes = NULL;
	HR(pMesh->LockIndexBuffer(D3DLOCK_READONLY, reinterpret_cast<LPVOID*>(&pBytes)),L"GetVertexIndexData: Failed to lock index buffer: ");
	if(indices32Bit)
	{
		DWORD *pIndex = 0;
		for (DWORD i = 0; i < (numFaces*3); ++i)
		{
			pIndex = reinterpret_cast<DWORD*>(pBytes);
			indexBuffer[i] = (int)*pIndex;
			pBytes += 4; // 4 bytes (32 bits)
		}
	}
	else
	{
		WORD *pIndex = 0;
		for (DWORD i = 0; i < (numFaces*3); ++i)
		{
			pIndex = reinterpret_cast<WORD*>(pBytes);
			indexBuffer[i] = (int)*pIndex;
			pBytes += 2; // 2 bytes (16 bits)
		}
	}
    HR(pMesh->UnlockIndexBuffer(),L"GetVertexIndexData: Failed to unlock index buffer: ");
}
/************************************************************************/
/* Name:		GetItensorBox	OLD CODE!!!								*/
/* Description:	Calculate tensor matrix from bouning box coordinates	*/
/*		Function assumes that box is symmetric about axes (ie, that 	*/
/*		model and bounding boxes have been normalized to center of mass)*/
/************************************************************************/
void rbobject::GetItensorBox(D3DXVECTOR3 *maxBound, D3DXMATRIX *Ib, D3DXMATRIX *Ib_inv)
{
	// Page D26 of Baraff and Witkin: Physically Based Modelling Notes
	float massScale = mass / 12;
	// Ixx = M/12 * (y0^2 + z0^2) --> where y0 and z0 are length of the cube sides
	Ib->_11 = 4*(maxBound->y)*(maxBound->y) + 4*(maxBound->z)*(maxBound->z);
	// Iyy = M/12 * (x0^2 + z0^2)
	Ib->_22 = 4*(maxBound->x)*(maxBound->x) + 4*(maxBound->z)*(maxBound->z);
	// Izz = M/12 * (x0^2 + y0^2)
	Ib->_33 = 4*(maxBound->x)*(maxBound->x) + 4*(maxBound->y)*(maxBound->y);
	Ib->_12 = 0; Ib->_13 = 0; Ib->_14 = 0;
	Ib->_21 = 0; Ib->_23 = 0; Ib->_24 = 0;
	Ib->_31 = 0; Ib->_32 = 0; Ib->_34 = 0;
	Ib->_41 = 0; Ib->_42 = 0; Ib->_43 = 0; Ib->_44 = 1;

	// Inverse of a diagonal matrix is just inverse of diagonal elements
	*Ib_inv = *Ib;
	Ib_inv->_11 =  1.0f/Ib_inv->_11; Ib_inv->_22 =  1.0f/Ib_inv->_22; Ib_inv->_33 =  1.0f/Ib_inv->_33;
}
/************************************************************************/
/* Name:		GetItensor												*/
/* Description:	Wrapper function to use Stan Melax's code.				*/
/*		Assume verticies normalized so that 0,0,0 is center of mass.	*/
/************************************************************************/
void rbobject::GetItensor(double3 * verts, int3 * ind, DWORD numFaces, D3DXMATRIX *Ib, D3DXMATRIX *Ib_inv)
{
	D3DXMatrixIdentity(Ib); D3DXMatrixIdentity(Ib_inv); 
	//float3x3 Inertia(const float3 *vertices, const int3 *tris, consts int count, const float3& com=float3(0,0,0))  
	double3x3 _Ib = Inertia(verts, ind, numFaces); // center of mass input is default (ie, <0,0,0>)
	// S Melax: "All you have to do is multiply the resulting matrix by your objects mass to scale it appropriately."
	_Ib.x.x *=  mass; _Ib.x.y *=  mass; _Ib.x.z *=  mass;
	_Ib.y.x *=  mass; _Ib.y.y *=  mass; _Ib.y.z *=  mass;
	_Ib.z.x *=  mass; _Ib.z.y *=  mass; _Ib.z.z *=  mass;
	// Find inverse of matrix
	double3x3 _Ib_inv = Inverse(_Ib);
	// set the input matricies to the correct values
	Ib->_11 = (float)_Ib.x.x; Ib->_12 = (float)_Ib.x.y;  Ib->_13 = (float)_Ib.x.z; 
	Ib->_21 = (float)_Ib.y.x; Ib->_22 = (float)_Ib.y.y;  Ib->_23 = (float)_Ib.y.z; 
	Ib->_31 = (float)_Ib.z.x; Ib->_32 = (float)_Ib.z.y;  Ib->_33 = (float)_Ib.z.z; 
	Ib_inv->_11 = (float)_Ib_inv.x.x; Ib_inv->_12 = (float)_Ib_inv.x.y;  Ib_inv->_13 = (float)_Ib_inv.x.z; 
	Ib_inv->_21 = (float)_Ib_inv.y.x; Ib_inv->_22 = (float)_Ib_inv.y.y;  Ib_inv->_23 = (float)_Ib_inv.y.z; 
	Ib_inv->_31 = (float)_Ib_inv.z.x; Ib_inv->_32 = (float)_Ib_inv.z.y;  Ib_inv->_33 = (float)_Ib_inv.z.z; 
}
/************************************************************************/
/* Name:		StateToArray											*/
/* Description:	Given a rbobject, copy state into array y				*/
/************************************************************************/
void rbobject::StateToArray(rbobject * rb, float *y)
{
	// Assumes memory has been allocated in y!!!
	// STORE POSITION
	*y++ = rb->x.x; *y++ = rb->x.y; *y++ = rb->x.z;
	// STORE QUATERNION
	*y++ = rb->q.w; *y++ = rb->q.x; *y++ = rb->q.y; *y++ = rb->q.z;
	// STORE LINEAR MOMENTUM
	*y++ = rb->P.x; *y++ = rb->P.y; *y++ = rb->P.z;
	// STORE ANGULAR MOMENTUM
	*y++ = rb->L.x; *y++ = rb->L.y; *y++ = rb->L.z;
}
/************************************************************************/
/* Name:		ArrayToState											*/
/* Description:	Given an array y, copy state into rbobject				*/
/************************************************************************/
void rbobject::ArrayToState(rbobject * rb, float *y)
{
	// LOAD POSITION
	rb->x.x = *y++; rb->x.y = *y++; rb->x.z = *y++;	
	// LOAD QUATERNION
	rb->q.w = *y++; rb->q.x = *y++; rb->q.y = *y++; rb->q.z = *y++;	
	// LOAD LINEAR MOMENTUM
	rb->P.x = *y++; rb->P.y = *y++; rb->P.z = *y++;	
	// LOAD ANGULAR MOMENTUM
	rb->L.x = *y++; rb->L.y = *y++; rb->L.z = *y++;	
	
	// Compute other variables
	rb->v = rb->P / rb->mass;

	if(abs(D3DXQuaternionLength(&rb->q))>EPSILON)
		D3DXQuaternionNormalize(&rb->q, &rb->q);
	D3DXMatrixRotationQuaternion(&rb->R, &rb->q);
	D3DXMatrixTranspose(&rb->R_t, &rb->R);
	rb->Iinv = rb->R * rb->Ibody_inv * rb->R_t;
	D3DXVec3TransformCoord(&rb->omega, &rb->L, &rb->Iinv); 
}
/************************************************************************/
/* Name:		ArrayToState											*/
/* Description:	Copy all rbobject states into array yout				*/
/************************************************************************/
void rbobject::ObjectsToState(float *yout) // Bodies_to_Array in Baraff and Witkin
{
	for(int i = 0; i < NO_RBOBJECTS; i ++)
		StateToArray(g_rbobjects[i], &yout[i * STATESIZE]);
}
/************************************************************************/
/* Name:		StateToObjects											*/
/* Description:	Copy all rbobject states from array yout				*/
/************************************************************************/
void rbobject::StateToObjects(float *yin) // Array_to_Bodies in Baraff and Witkin
{
	for(int i = 0; i < NO_RBOBJECTS; i ++)
		ArrayToState(g_rbobjects[i], &yin[i * STATESIZE]);
}