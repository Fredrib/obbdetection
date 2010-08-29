//=================================================================================================
// util.cpp - utility functions
//=================================================================================================
#include "main.h"
#include "d3dformats.h"
#include <dxerr.h>
#pragma comment(lib, "dxerr.lib")

// Calculates correct up vector for a given look at point and eye point.
// --> Rotates up vector to be perpendicular with direction of eye.
void GetCorrectUp(D3DXVECTOR3 * lookAtPt, D3DXVECTOR3 * eyePt, D3DXVECTOR3 * up)
{
	D3DXVECTOR3 rotAxis, dir;
	D3DXMATRIX matRot;

	dir = *lookAtPt - *eyePt;									// find direction vector
	D3DXVec3Normalize(&dir, &dir);								// normalize direction vector
	D3DXVec3Normalize(up, up);									// normalize the up vector

	float rotAngle = acos(D3DXVec3Dot(&dir, up));				// acosine of dot product will give angle between vectors if their magnitudes are 1
	// Check that up is not on the same axis as direction vector
	if(abs(rotAngle-D3DX_PI) < EPSILON || abs(rotAngle-0.0f) < EPSILON)
		throw std::runtime_error("GetCorrectUp: up vector is perpendicular to direction vector!");
	rotAngle = rotAngle - D3DX_PI / 2.0f;						// want new up to be perpendicular to direction vector

	D3DXVec3Cross(&rotAxis, up, &dir);							// cross product of up and direction will give axis of rotation
	D3DXVec3Normalize(&rotAxis, &rotAxis);						// normalize the rotation axis

	D3DXMatrixRotationAxis(&matRot,&rotAxis, rotAngle);			// Build a rotation matrix from angle and axis
	D3DXVec3TransformCoord(up, up, &matRot);					// rotate the up vector
} //GetCorrectUp

double Determinant(const double3x3& m );
double  dot( const double3& a, const double3& b );
double3 cross( const double3& a, const double3& b );
double  magnitude( const double3& v );
double3x3 Inverse(const double3x3& a);
double3x3 Transpose( const double3x3& m );

// Tests whether a matrix is rotational or not:
// The condition for this is: (from http://en.wikipedia.org/wiki/Rotation_matrix)
// R' * R = I (or R' = R^-1)
// and
// det(R) =1
/*  NOTE: THERE IS SOME REDUNDANCY IN THESE TWO FUNCTIONS...  THERE ARE A MILLION WAYS TO TEST FOR THIS */
bool isRotation(double3x3 * mat)
{
	double epsilon = 0.01; // margin to allow for rounding errors
	double3x3 matInv = Inverse(*mat);
	double3x3 matTranspose = Transpose(*mat);
	if(abs(matInv.x.x - matTranspose.x.x)>epsilon) { return false; }
	if(abs(matInv.x.y - matTranspose.x.y)>epsilon) { return false; }
	if(abs(matInv.x.z - matTranspose.x.z)>epsilon) { return false; }
	if(abs(matInv.y.x - matTranspose.y.x)>epsilon) { return false; }
	if(abs(matInv.y.y - matTranspose.y.y)>epsilon) { return false; }
	if(abs(matInv.y.z - matTranspose.y.z)>epsilon) { return false; }
	if(abs(matInv.z.x - matTranspose.z.x)>epsilon) { return false; }
	if(abs(matInv.z.y - matTranspose.z.y)>epsilon) { return false; }
	if(abs(matInv.z.z - matTranspose.z.z)>epsilon) { return false; }
	return (abs(abs(Determinant(*mat))-1) < epsilon);
}
bool isBasis(double3 * vec0,double3 * vec1,double3 * vec2)
{
	double epsilon = 0.01; // margin to allow for rounding errors
	if (abs(magnitude(*vec0)-1) > epsilon) return false;
	if (abs(magnitude(*vec1)-1) > epsilon) return false;
	if (abs(magnitude(*vec2)-1) > epsilon) return false;
	if(abs(abs(dot(*vec2,cross(*vec0,*vec1)))-1) > epsilon) return false;
	if (abs(dot(*vec0,*vec1)) > epsilon) return false;
	if (abs(dot(*vec0,*vec2)) > epsilon) return false;
	if (abs(dot(*vec1,*vec2)) > epsilon) return false;
	return true;
}

//=================================================================================================

std::wstring util::Win32ErrorToString(DWORD dwErrorCode)
{
	wchar_t* szMsgBuffer;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwErrorCode, 0, (LPTSTR)&szMsgBuffer, 0, NULL);
	std::wstring strRet(szMsgBuffer);
	LocalFree(szMsgBuffer);
	return strRet;
}

//=================================================================================================

std::wstring util::DXErrorToString(HRESULT hResult)
{
	return DXGetErrorStringW(hResult);
}

//=================================================================================================

// THE FOLLOWING IS FROM:
// http://www.codeproject.com/KB/stl/upgradingstlappstounicode.aspx?fid=16224&df=90&mpp=25&noise=3&sort=Position&view=Quick&select=1481519

std::wstring toWideString( const char* pStr , int len )
{
    // figure out how many wide characters we are going to get 

    int nChars = MultiByteToWideChar( CP_ACP , 0 , pStr , len , NULL , 0 ) ; 
    if ( len == -1 )
        -- nChars ; 
    if ( nChars == 0 )
        return L"" ;

    // convert the narrow string to a wide string 

    // nb: slightly naughty to write directly into the string like this

    wstring buf ;
    buf.resize( nChars ) ; 
    MultiByteToWideChar( CP_ACP , 0 , pStr , len , 
        const_cast<wchar_t*>(buf.c_str()) , nChars ) ; 

    return buf ;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */

string toNarrowString( const wchar_t* pStr , int len )
{
    // figure out how many narrow characters we are going to get 

    int nChars = WideCharToMultiByte( CP_ACP , 0 , 
             pStr , len , NULL , 0 , NULL , NULL ) ; 
    if ( len == -1 )
        -- nChars ; 
    if ( nChars == 0 )
        return "" ;

    // convert the wide string to a narrow string

    // nb: slightly naughty to write directly into the string like this

    string buf ;
    buf.resize( nChars ) ;
    WideCharToMultiByte( CP_ACP , 0 , pStr , len , 
          const_cast<char*>(buf.c_str()) , nChars , NULL , NULL ) ; 

    return buf ; 
}

// From F. Luna
void LoadXFile(const std::wstring& filename, ID3DXMesh** meshOut, std::vector<Mtrl>& mtrls, std::vector<IDirect3DTexture9*>& texs)
{
	// Step 1: Load the .x file from file into a system memory mesh.
	ID3DXMesh* meshSys      = 0;
	ID3DXBuffer* adjBuffer  = 0;
	ID3DXBuffer* mtrlBuffer = 0;
	DWORD numMtrls          = 0;

	HR(D3DXLoadMeshFromX(filename.c_str(), D3DXMESH_SYSTEMMEM, g_app->m_pD3DDev, &adjBuffer, &mtrlBuffer, 0, &numMtrls, &meshSys),
		L"LoadXFile: D3DXLoadMeshFromX failed: ");

	// Step 2: Find out if the mesh already has normal info?
	D3DVERTEXELEMENT9 elems[MAX_FVF_DECL_SIZE];
	HR(meshSys->GetDeclaration(elems),L"LoadXFile: GetDeclaration failed: ");
	
	bool hasNormals = false;
	D3DVERTEXELEMENT9 term = D3DDECL_END();
	for(int i = 0; i < MAX_FVF_DECL_SIZE; ++i)
	{
		// Did we reach D3DDECL_END() {0xFF,0,D3DDECLTYPE_UNUSED, 0,0,0}?
		if(elems[i].Stream == 0xff )
			break;

		if( elems[i].Type == D3DDECLTYPE_FLOAT3 &&
			elems[i].Usage == D3DDECLUSAGE_NORMAL &&
			elems[i].UsageIndex == 0 )
		{
			hasNormals = true;
			break;
		}
	}

	// Step 3: Change vertex format to VertexPNT.
	D3DVERTEXELEMENT9 elements[64];
	UINT numElements = 0;
	VertexPNT::Decl->GetDeclaration(elements, &numElements);

	ID3DXMesh* temp = 0;
	HR(meshSys->CloneMesh(D3DXMESH_SYSTEMMEM, elements, g_app->m_pD3DDev, &temp),L"LoadXFile: CloneMesh failed: ");
	ReleaseCOM(meshSys);
	meshSys = temp;

	// Step 4: If the mesh did not have normals, generate them.
	if( hasNormals == false)
		HR(D3DXComputeNormals(meshSys, 0),L"LoadXFile: D3DXComputeNormals failed: ");


	// Step 5: Optimize the mesh.
	//R(meshSys->Optimize(D3DXMESH_MANAGED | D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, 
	//					(DWORD*)adjBuffer->GetBufferPointer(), 0, 0, 0, meshOut),
	//	L"LoadXFile: Optimize failed: ");
	*meshOut = meshSys;
	//ReleaseCOM(meshSys); // Done w/ system mesh.
	//ReleaseCOM(adjBuffer); // Done with buffer.

	// Step 6: Extract the materials and load the textures.
	if( mtrlBuffer != 0 && numMtrls != 0 )
	{
		D3DXMATERIAL* d3dxmtrls = (D3DXMATERIAL*)mtrlBuffer->GetBufferPointer();

		for(DWORD i = 0; i < numMtrls; ++i)
		{
			// Save the ith material.  Note that the MatD3D property does not have an ambient
			// value set when its loaded, so just set it to the diffuse value.
			Mtrl m;
			m.ambient   = d3dxmtrls[i].MatD3D.Diffuse;
			m.diffuse   = d3dxmtrls[i].MatD3D.Diffuse;
			m.spec		= d3dxmtrls[i].MatD3D.Specular;
			m.specPower = d3dxmtrls[i].MatD3D.Power;
			mtrls.push_back( m );

			// Check if the ith material has an associative texture
			if( d3dxmtrls[i].pTextureFilename != 0 )
			{
				// Yes, load the texture for the ith subset
				IDirect3DTexture9* tex = 0;
				std::wstring path = L"models/";
				path.append(toWideString(d3dxmtrls[i].pTextureFilename,-1).c_str());
				if(FAILED(D3DXCreateTextureFromFile(g_app->m_pD3DDev, path.c_str(), &tex)))
				{
					std::string error = "LoadXFile: D3DXCreateTextureFromFile Failed to load texture: ";
					error.append(toNarrowString(path.c_str(),-1));
					throw std::runtime_error(error.c_str());
				}
				texs.push_back( tex );	
			}
			else
			{
				// No texture for the ith subset
				texs.push_back( 0 );
			}
		}
	}
	ReleaseCOM(mtrlBuffer); // done w/ buffer
}
