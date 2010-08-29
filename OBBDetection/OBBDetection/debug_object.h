// File:		debug_object.h
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

// A class to hold vertex and index buffers for rendering debug objects, stored in a linked list.
// The header of these is stored in the "renderer" class and rendered each frame in wireframe mode.

#include "main.h"
#include "obbox.h"

#ifndef debug_object_h
#define debug_object_h

enum debug_object_type 
{
	DO_UNDEFINED = 0,
	DO_LINES = 1,
	DO_TRIANGLES = 2,
};

class debug_object 
{
public:
									debug_object( void );
									~debug_object( void ); // Recursively delete list

	void							InitForRendering( debug_object_type type_in );
	void							Render( void );
	void							InsertAtEnd( debug_object * input_obj );
	static void						AddObboxDebugObject( obbox * obbox_in, D3DCOLOR color  );
	static void						AddObboxTriDebugObject( obbox * obbox_in, D3DCOLOR color  );

	// Object type and raw buffer data
	debug_object_type               type;
	UINT							numVert; // Number of verticies (number of floats = numVert * 3)
	float *							vert;
	UINT							numInd;
	UINT *							ind;
	D3DCOLOR						color;

	// Affine transformation, scale * rotation * translation
	D3DXMATRIX						matWorld;

	// D3D Buffers
	IDirect3DVertexBuffer9 *		vertexBuffer;
	IDirect3DIndexBuffer9 *			indexBuffer;

	// Linked list pointers
	debug_object *					next;
	debug_object *					prev;

};

#endif