/*************************************************************
**				obbox_renderitems class						**
*************************************************************/
// File:		obbox_renderitems.h
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

// These items used to be included in obbox class --> Lead to higher than necessary memory overhead.
// Now only include pointers if needed.

#include	"main.h"

#ifndef obbox_renderitems_h
#define obbox_renderitems_h

class obbox_renderitems
{
public:
	friend class rboMesh;
	friend class obbox;

									obbox_renderitems();
									~obbox_renderitems();
// private:

	// Convex hull store - Usually NULL after initialization (unless we're keeping to save it to disk)
	double *						cHullVert;
	UINT							cHullVertCount;
	PUINT							cHullInd;
	UINT							cHullIndCount;

	// Rendering objects
	IDirect3DVertexBuffer9 *		convexHullVertexBuffer;
	IDirect3DIndexBuffer9 *			convexHullIndexBuffer;
	DWORD							convexHullVertexBufferSize;
	DWORD							convexHullIndexBufferSize;
	IDirect3DVertexBuffer9 *		OBBVertexBuffer;
	IDirect3DIndexBuffer9 *			OBBIndexBuffer;
	DWORD							OBBVertexBufferSize;
	DWORD							OBBIndexBufferSize;
};

#endif