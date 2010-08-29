#ifndef HULL_H
#define HULL_H

#include "main.h"

/*----------------------------------------------------------------------
		Copyright (c) 2004 Open Dynamics Framework Group
					www.physicstools.org
		All rights reserved.

		Redistribution and use in source and binary forms, with or without modification, are permitted provided
		that the following conditions are met:

		Redistributions of source code must retain the above copyright notice, this list of conditions
		and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.

		Neither the name of the Open Dynamics Framework Group nor the names of its contributors may
		be used to endorse or promote products derived from this software without specific prior written permission.

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES,
		INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
		DISCLAIMED. IN NO EVENT SHALL THE INTEL OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
		EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
		LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
		IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
		THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/

/*****************************************************************/
// JONATHAN TOMPSON - PUT THESE HERE TO BE USED EXTERNALLY
class double3 // 3D
{
	public:
	double x,y,z;
	double3(){x=0;y=0;z=0;};
	double3(double _x,double _y,double _z){x=_x;y=_y;z=_z;};
	//operator double *() { return &x;};
	double& operator[](int i) {assert(i>=0&&i<3);return ((double*)this)[i];}
	const double& operator[](int i) const {assert(i>=0&&i<3);return ((double*)this)[i];}
	double3 & operator=(const double3 &rhs)
	{
		if (this != &rhs) {
		  this->x = rhs.x;
		  this->y = rhs.y;
		  this->z = rhs.z;
		}

		return *this;
	}
};

class int3  
{
public:
	int x,y,z;
	int3(){};
	int3(int _x,int _y, int _z){x=_x;y=_y;z=_z;}
	const int& operator[](int i) const {return (&x)[i];}
	int& operator[](int i) {return (&x)[i];}
};

class double3x3
{
	public:
	double3 x,y,z;  // the 3 rows of the Matrix
	double3x3(){}
	double3x3(double init):x(init,init,init),y(init,init,init),z(init,init,init){} // ADD TOMPSON
	double3x3(double xx,double xy,double xz,double yx,double yy,double yz,double zx,double zy,double zz):x(xx,xy,xz),y(yx,yy,yz),z(zx,zy,zz){}
	double3x3(double3 _x,double3 _y,double3 _z):x(_x),y(_y),z(_z){}
	double3&       operator[](int i)       {assert(i>=0&&i<3);return (&x)[i];}
	const double3& operator[](int i) const {assert(i>=0&&i<3);return (&x)[i];}
	double&        operator()(int r, int c)       {assert(r>=0&&r<3&&c>=0&&c<3);return ((&x)[r])[c];}
	const double&  operator()(int r, int c) const {assert(r>=0&&r<3&&c>=0&&c<3);return ((&x)[r])[c];}
}; 
class PHullResult
{
public:

	PHullResult(void)
	{
		mVcount = 0;
		mIndexCount = 0;
		mFaceCount = 0;
		mVertices = 0;
		mIndices  = 0;
	}

	unsigned int mVcount;
	unsigned int mIndexCount;
	unsigned int mFaceCount;
	double       *mVertices;
	unsigned int *mIndices;
};
void ReleaseHull(PHullResult &result);

double3 operator-( const double3& a, const double3& b );
double3 operator+( const double3& a, const double3& b );
double3 operator-( const double3& v );
double3 operator*( const double3& v, const double s );
double3 operator*( const double s, const double3& v );
double3 operator/( const double3& v, const double s );
inline int operator==( const double3 &a, const double3 &b ) { return (a.x==b.x && a.y==b.y && a.z==b.z); }
inline int operator!=( const double3 &a, const double3 &b ) { return (a.x!=b.x || a.y!=b.y || a.z!=b.z); }
double3x3			Inverse(const double3x3 &a);
double3				CenterOfMass(const double3 *vertices, const int3 *tris, const int count);
double3x3			Inertia(const double3 *vertices, const int3 *tris, const int count, const double3& com=double3(0,0,0));
double3				cross( const double3& a, const double3& b );
double				dot( const double3& a, const double3& b );
double3x3			Transpose( const double3x3& m );
void				getSymMatEigenvectorsMelax(double3 * ret, const double3x3 &A); // --> DOESN'T WORK!! TESTED AGAINST MATLAB
double3				safenormalize(const double3 &v);
void				double3x3ToD3DXMATRIX( D3DXMATRIX * m_out, double3x3 * m_in);
bool				ComputeHull(unsigned int vcount,const double *vertices,PHullResult &result,unsigned int vlimit,double inflate);

// For multiplying matricies accross different class types - added by me
void				MatrixMult( double3x3 * m_out, D3DXMATRIX * m_A, double3x3 * m_B );			// matrix * matrix
void				MatrixMult( double3x3 * m_out, double3x3 * m_A, D3DXMATRIX * m_B );			// matrix * matrix
void				MatrixMult( double3 * ret, double3x3 * a, double3 * b );					// matrix * vector
void				MatrixMult( double3 * ret, D3DXMATRIX * a, double3 * b );					// matrix * vector
void				MatrixMult( float * ret, D3DXMATRIX * a, float * b );						// matrix * vector

void				MatrixAffineMult( float * ret, D3DXMATRIX * a, float * b );					// matrix * vector (with translation)

void				MatrixMult( double3x3 * ret, double3x3 * m_A, double3x3 * m_B );			// matrix * matrix

void				MatrixMult_ATran_B( double3x3 * ret, double3x3 * m_A, double3x3 * m_B );	// matrix^T * matrix
void				MatrixMult_ATran_B( double3x3 * ret, double3x3 * m_A, D3DXMATRIX * m_B );   // matrix^T * matrix
void				MatrixMult_ATran_B( double3x3 * ret, D3DXMATRIX * m_A, double3x3 * m_B );   // matrix^T * matrix
void				MatrixMult_ATran_B( double3x3 * ret, D3DXMATRIX * m_A, D3DXMATRIX * m_B );  // matrix^T * matrix
void				MatrixMult_ATran_B( double3 * ret, double3x3 * m_A, double3 * m_B );		// matrix^T * vector
void				MatrixMult_ATran_B( double3 * ret, D3DXMATRIX * m_A, double3 * m_B );		// matrix^T * vector
void				MatrixMult_ATran_B( float * ret, D3DXMATRIX * m_A, float * m_B );			// matrix^T * vector

void				MatrixAffineMult_ATran_B( float * ret, D3DXMATRIX * m_A, float * m_B );		// matrix^T * vector (with translation)

void				MatrixMult_A_BTran( double3x3 * ret, double3x3 * m_A, double3x3 * m_B );	// matrix * matrix^T
void				MatrixMult_A_BTran( double3x3 * ret, double3x3 * m_A, D3DXMATRIX * m_B );	// matrix * matrix^T
void				MatrixMult_A_BTran( double3x3 * ret, D3DXMATRIX * m_A, double3x3 * m_B );	// matrix * matrix^T

void				MatrixMult_ATran_BTran( double3x3 * ret, D3DXMATRIX * m_A, double3x3 * m_B );   // matrix^T * matrix^T 
void				MatrixMult_ATran_BTran( double3x3 * ret, double3x3 * m_A, D3DXMATRIX * m_B );	// matrix^T * matrix^T 

// For addition of vectors accross different class types
void				Add_D3DXVECTOR3_double3( double3 * ret, D3DXVECTOR3 * a, double3 * b );
void				Add_D3DXVECTOR3_D3DXVECTOR3( double3 * ret, D3DXVECTOR3 * a, D3DXVECTOR3 * b );

// SM_chull is my main interface function
bool SM_chull(double * vertexSet, int vertexSetSize, double ** cHullVert, UINT * cHullVertCount, PUINT * cHullInd, UINT * cHullIndCount );

// JONATHAN TOMPSON - END
/*****************************************************************/

class HullResult
{
public:
	HullResult(void)
	{
		mPolygons = true;
		mNumOutputVertices = 0;
		mOutputVertices = 0;
		mNumFaces = 0;
		mNumIndices = 0;
		mIndices = 0;
	}
	bool                    mPolygons;                  // true if indices represents polygons, false indices are triangles
	unsigned int            mNumOutputVertices;         // number of vertices in the output hull
	double                  *mOutputVertices;            // array of vertices, 3 doubles each x,y,z
	unsigned int            mNumFaces;                  // the number of faces produced
	unsigned int            mNumIndices;                // the total number of indices
	unsigned int           *mIndices;                   // pointer to indices.

// If triangles, then indices are array indexes into the vertex list.
// If polygons, indices are in the form (number of points in face) (p1, p2, p3, ..) etc..
};

enum HullFlag
{
	QF_TRIANGLES         = (1<<0),             // report results as triangles, not polygons.
	QF_REVERSE_ORDER     = (1<<1),             // reverse order of the triangle indices.
	QF_SKIN_WIDTH        = (1<<2),             // extrude hull based on this skin width
	QF_DEFAULT           = 0
};


class HullDesc
{
public:
	HullDesc(void)
	{
		mFlags          = QF_DEFAULT;
		mVcount         = 0;
		mVertices       = 0;
		mVertexStride   = sizeof(double)*3;
		mNormalEpsilon  = 0.001f;
		mMaxVertices	= 4096; // maximum number of points to be considered for a convex hull.
		mMaxFaces		= 4096;
		mSkinWidth		= 0.01f; // default is one centimeter
	};

	HullDesc(HullFlag flag,
						 unsigned int vcount,
						 const double *vertices,
						 unsigned int stride)
	{
		mFlags          = flag;
		mVcount         = vcount;
		mVertices       = vertices;
		mVertexStride   = stride;
		mNormalEpsilon  = 0.001f;
		mMaxVertices    = 4096;
		mSkinWidth = 0.01f; // default is one centimeter
	}

	bool HasHullFlag(HullFlag flag) const
	{
		if ( mFlags & flag ) return true;
		return false;
	}

	void SetHullFlag(HullFlag flag)
	{
		mFlags|=flag;
	}

	void ClearHullFlag(HullFlag flag)
	{
		mFlags&=~flag;
	}

	unsigned int      mFlags;           // flags to use when generating the convex hull.
	unsigned int      mVcount;          // number of vertices in the input point cloud
	const double      *mVertices;        // the array of vertices.
	unsigned int      mVertexStride;    // the stride of each vertex, in bytes.
	double             mNormalEpsilon;   // the epsilon for removing duplicates.  This is a normalized value, if normalized bit is on.
	double             mSkinWidth;
	unsigned int      mMaxVertices;               // maximum number of vertices to be considered for the hull!
	unsigned int      mMaxFaces;
};

enum HullError
{
	QE_OK,            // success!
	QE_FAIL           // failed.
};

class HullLibrary
{
public:

	HullError CreateConvexHull(const HullDesc       &desc,           // describes the input request
							   HullResult           &result);        // contains the resulst

	HullError ReleaseResult(HullResult &result); // release memory allocated for this result, we are done with it.

private:

	void BringOutYourDead(const double *verts,unsigned int vcount, double *overts,unsigned int &ocount,unsigned int *indices,unsigned indexcount);

	bool    CleanupVertices(unsigned int svcount,
													const double *svertices,
													unsigned int stride,
													unsigned int &vcount,       // output number of vertices
													double *vertices,                 // location to store the results.
													double  normalepsilon,
													double *scale);
};


#endif

