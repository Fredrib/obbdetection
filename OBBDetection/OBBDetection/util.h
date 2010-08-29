//=================================================================================================
// util.h - utility functions
//=================================================================================================
// This is borrowed from http://thetavern.servebeer.com/?p=articles&a=D3DTutorial1
// I liked their implimentation of error handling during initialization
//
// There is other stuff thrown in here that doesn't fit anywhere good.

#ifndef util_H_INCLUDED
#define util_H_INCLUDED

#include "main.h"
#include "d3dformats.h"
#include "hull.h"

#ifndef HR
#define HR(x,y)																			\
{																						\
	HRESULT hr = x;																		\
	if(FAILED(hr))																		\
	{																					\
		std::wstring error = y + util::DXErrorToString(GetLastError());					\
		throw std::runtime_error(toNarrowString(error.c_str(),-1));						\
	}																					\
}
#endif

void GetCorrectUp(D3DXVECTOR3 * lookAtPt, D3DXVECTOR3 * eyePt, D3DXVECTOR3 * up);

namespace util
{
	// Convert a Windows error to a string
	std::wstring Win32ErrorToString(DWORD dwErrorCode);

	// Convert a D3D error to a string
	std::wstring DXErrorToString(HRESULT hResult);
}

// THE FOLLOWING IS FROM:
// http://www.codeproject.com/KB/stl/upgradingstlappstounicode.aspx?fid=16224&df=90&mpp=25&noise=3&sort=Position&view=Quick&select=1481519
std::wstring toWideString( const char* pStr , int len );
string toNarrowString( const wchar_t* pStr , int len );

// FROM F. LUNA
void LoadXFile(const std::wstring& filename, ID3DXMesh** meshOut, std::vector<Mtrl>& mtrls, std::vector<IDirect3DTexture9*>& texs);

// My own code to test whether or not a matrix is a rotation matrix
class double3x3;
class double3;
bool isRotation(double3x3 * mat);
bool isBasis(double3 * vec0,double3 * vec1,double3 * vec2);

#endif // util_H_INCLUDED