// File:		debug_object.cpp
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

// A class to hold vertex and index buffers for rendering debug objects.  
// A vector of these is stored in renderer class and rendered each frame in wireframeCol mode.

#include "main.h"

/************************************************************************/
/* Name:		debug_object											*/
/* Description:	Default constructor										*/
/************************************************************************/
debug_object::debug_object(void)
{
	vertexBuffer = NULL; indexBuffer = NULL;
	vert = NULL; ind = NULL;
	numVert = 0; numInd = 0;
	type = DO_UNDEFINED;
	next = NULL; prev = NULL;
	color =  D3DCOLOR_ARGB(255, 255, 255, 255); // Reset to black
}
/************************************************************************/
/* Name:		~debug_object											*/
/* Description:	Default destructor										*/
/************************************************************************/
debug_object::~debug_object(void)
{
	if(next){delete next; next = NULL;} // Recursively delete the rest of the linked list.
	ReleaseCOM(vertexBuffer);
	ReleaseCOM(indexBuffer);
	if(vert){delete [] vert; vert = NULL;}
	if(ind){delete [] ind; ind = NULL;}
}
/************************************************************************/
/* Name:		InitForRendering										*/
/* Description:	Initialize buffers for rendering primatives				*/
/************************************************************************/
void debug_object::InitForRendering( debug_object_type type_in )
{
#ifdef _DEBUG
	if(!vert || numVert < 1)
		throw std::runtime_error("debug_object::InitForRendering: vert array is empty");
	if(!ind || numInd < 1)
		throw std::runtime_error("debug_object::InitForRendering: vert array is empty");
#endif

	type = type_in;

	// Obtain a pointer to a new vertex buffer.
	HR(g_app->m_pD3DDev->CreateVertexBuffer(numVert * sizeof(VertexPosCol), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, & vertexBuffer, 0),
		L"debug_object::InitForRendering: CreateVertexBuffer failed: ");

	// Now lock it to obtain a pointer to its internal data, and write the grid's vertex data.
	VertexPosCol* v = 0;
	HR(vertexBuffer->Lock(0, 0, (void**)&v, 0), L"debug_object::InitForRendering: Failed to lock vertex buffer: ");

	// Now copy over vertex data
	for(UINT i = 0; i < numVert; i ++)
		v[i] = VertexPosCol(D3DXVECTOR3(vert[3*i + 0], vert[3*i + 1], vert[3*i + 2]),color); // Vert (x,y,z) and D3DCOLOR

	// Unlock the buffer since we are done
	HR(vertexBuffer->Unlock(), L"debug_object::InitForRendering: Failed to unlock vertex buffer: ");

	// Obtain a pointer to a new index buffer.
	HR(g_app->m_pD3DDev->CreateIndexBuffer(numInd * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, & indexBuffer, 0),
		L"debug_object::InitForRendering: CreateIndexBuffer failed: ");
	
	// Now lock it to obtain a pointer to its internal data, and write the grid's index data.
	WORD* k = 0;
	HR(indexBuffer->Lock(0, 0, (void**)&k, 0), L"debug_object::InitForRendering: Failed to lock index buffer: ");

	// Now copy over index data
	for(UINT i = 0; i < numInd; i ++)
		k[i] = (WORD) ind[i];

	// Unlock the buffer since we are done
	HR(indexBuffer->Unlock(), L"debug_object::InitForRendering: Failed to unlock index buffer: ");
}
/************************************************************************/
/* Name:		Render													*/
/* Description:	Render the primatives in wireframe mode					*/
/************************************************************************/
void debug_object::Render( void )
{
#ifdef _DEBUG
	if(type == DO_UNDEFINED || !vertexBuffer || !indexBuffer )
		throw std::runtime_error("debug_object::Render: Trying to render an un-initialized object");
#endif
	if(type == DO_TRIANGLES)
		//g_renderer->DrawWireframe(vertexBuffer, numVert, indexBuffer, numInd , & (matWorld * g_camera->m_ViewProj) );
		// Edit Aug 11 2010 - Now drawing debug object wireframes in color
		g_renderer->DrawWireframeCol(vertexBuffer, numVert, indexBuffer, numInd , & (matWorld * g_camera->m_ViewProj) );
	if(type == DO_LINES)
		//g_renderer->DrawWireframeLines(vertexBuffer, numVert, indexBuffer, numInd , & (matWorld * g_camera->m_ViewProj) );
		// Edit Aug 11 2010 - Now drawing debug object wireframes in color
		g_renderer->DrawWireframeColLines(vertexBuffer, numVert, indexBuffer, numInd , & (matWorld * g_camera->m_ViewProj) );

	if(next)
		next->Render(); // Recursively draw the rest of the debug objects
}
/************************************************************************/
/* Name:		InsertAtEnd												*/
/* Description:	Insert input debug_object at the end of the linked-list	*/
/************************************************************************/
void debug_object::InsertAtEnd( debug_object * input_obj )
{
#ifdef _DEBUG
	if( input_obj == NULL )
		throw std::runtime_error("debug_object::InsertAtEnd: NULL input to function");
#endif

	debug_object * curObject = this;
	while(curObject->next != NULL) // Iterate to the end of the array
		curObject = curObject->next;

	curObject->next = input_obj;
	input_obj->prev = curObject;

}
/************************************************************************/
/* Name:		AddObboxDebugObject										*/
/* Description:	Make an exact copy of the current Obbox for rendering.	*/
/*              Second input is a D3DCOLOR value						*/
/************************************************************************/
void debug_object::AddObboxDebugObject( obbox * obbox_in, D3DCOLOR color )
{
#ifdef _DEBUG
	if( obbox_in == NULL )
		throw std::runtime_error("debug_object::AddObboxDebugObject: NULL input to function");
	//if( !obbox_in->isLeaf )
	//	throw std::runtime_error("debug_object::AddObboxDebugObject: Input obbox is not a leaf");
#endif


	// Make a new debug object on the heap
	debug_object * DO = new debug_object;

	// Add it to the linked list of objects
	if(g_renderer->debug_objects) 
		g_renderer->debug_objects->InsertAtEnd(DO); // Insert at end of linked-list
	else
		g_renderer->debug_objects = DO; // Otherwise start a new linked-list if empty

	DO->type = DO_LINES;
	DO->numVert = 8; // 8 verticies in a box
	DO->numInd = 24; //  24 indicies to describe 12 edges (both sides)

	DO->vert = new float[DO->numVert*3]; // Three floats per vertex (x.y.z)
	DO->ind = new UINT[DO->numInd];

	DO->matWorld = obbox_in->rbo->matWorld;

	DO->color = color;

	// **** Now calculate the line primatives from the OBB box half-lengths and center ****
/*
	// WORK OUT BOX CORNERS FOR VERTEX BUFFER IN OBJECT COORDINATES
	double3 boxCoord; double3 objCoord; // Some temporary variables
	objCoord = obbox_in->boxCenterObjectCoord - ((obbox_in->boxDimension.x) * obbox_in->orient[0]) 
											  - ((obbox_in->boxDimension.y) * obbox_in->orient[1]) 
											  - ((obbox_in->boxDimension.z) * obbox_in->orient[2]);
	DO->vert[0]  = (float)objCoord.x;				// V0
	DO->vert[1]  = (float)objCoord.y; 
	DO->vert[2]  = (float)objCoord.z;

	objCoord = obbox_in->boxCenterObjectCoord + ((obbox_in->boxDimension.x) * obbox_in->orient[0]) 
											  - ((obbox_in->boxDimension.y) * obbox_in->orient[1]) 
											  - ((obbox_in->boxDimension.z) * obbox_in->orient[2]);
	DO->vert[3]  = (float)objCoord.x;				// V1
	DO->vert[4]  = (float)objCoord.y; 
	DO->vert[5]  = (float)objCoord.z;

	objCoord = obbox_in->boxCenterObjectCoord - ((obbox_in->boxDimension.x) * obbox_in->orient[0]) 
											  - ((obbox_in->boxDimension.y) * obbox_in->orient[1]) 
											  + ((obbox_in->boxDimension.z) * obbox_in->orient[2]);	
	DO->vert[6]  = (float)objCoord.x;				// V2
	DO->vert[7]  = (float)objCoord.y; 
	DO->vert[8]  = (float)objCoord.z;

	objCoord = obbox_in->boxCenterObjectCoord + ((obbox_in->boxDimension.x) * obbox_in->orient[0]) 
											  - ((obbox_in->boxDimension.y) * obbox_in->orient[1]) 
											  + ((obbox_in->boxDimension.z) * obbox_in->orient[2]);	
	DO->vert[9]  = (float)objCoord.x;				// V3
	DO->vert[10] = (float)objCoord.y; 
	DO->vert[11] = (float)objCoord.z;

	objCoord = obbox_in->boxCenterObjectCoord - ((obbox_in->boxDimension.x) * obbox_in->orient[0]) 
											  + ((obbox_in->boxDimension.y) * obbox_in->orient[1]) 
											  - ((obbox_in->boxDimension.z) * obbox_in->orient[2]);
	DO->vert[12] = (float)objCoord.x;				// V4
	DO->vert[13] = (float)objCoord.y; 
	DO->vert[14] = (float)objCoord.z;

	objCoord = obbox_in->boxCenterObjectCoord + ((obbox_in->boxDimension.x) * obbox_in->orient[0]) 
											  + ((obbox_in->boxDimension.y) * obbox_in->orient[1]) 
											  - ((obbox_in->boxDimension.z) * obbox_in->orient[2]);
	DO->vert[15] = (float)objCoord.x;				// V5
	DO->vert[16] = (float)objCoord.y; 
	DO->vert[17] = (float)objCoord.z;

	objCoord = obbox_in->boxCenterObjectCoord - ((obbox_in->boxDimension.x) * obbox_in->orient[0]) 
											  + ((obbox_in->boxDimension.y) * obbox_in->orient[1]) 
											  + ((obbox_in->boxDimension.z) * obbox_in->orient[2]);
	DO->vert[18] = (float)objCoord.x;				// V6
	DO->vert[19] = (float)objCoord.y; 
	DO->vert[20] = (float)objCoord.z;

	objCoord = obbox_in->boxCenterObjectCoord + ((obbox_in->boxDimension.x) * obbox_in->orient[0]) 
											  + ((obbox_in->boxDimension.y) * obbox_in->orient[1]) 
											  + ((obbox_in->boxDimension.z) * obbox_in->orient[2]);
	DO->vert[21] = (float)objCoord.x;				// V7
	DO->vert[22] = (float)objCoord.y; 
	DO->vert[23] = (float)objCoord.z;
*/

	// WORK OUT BOX CORNERS FOR VERTEX BUFFER IN OBJECT COORDINATES
	double3 boxCoord; double3 objCoord; // Some temporary variables
	boxCoord = obbox_in->boxCenterBoxCoord + double3(-1.0*obbox_in->boxDimension.x,-1.0*obbox_in->boxDimension.y,-1.0*obbox_in->boxDimension.z);
	MatrixMult( & objCoord, & obbox_in->orientMatrix, & boxCoord );
	DO->vert[0]  = (float)objCoord.x;				// V0
	DO->vert[1]  = (float)objCoord.y; 
	DO->vert[2]  = (float)objCoord.z;

	boxCoord = obbox_in->boxCenterBoxCoord + double3(+1.0*obbox_in->boxDimension.x,-1.0*obbox_in->boxDimension.y,-1.0*obbox_in->boxDimension.z);
	MatrixMult( & objCoord, & obbox_in->orientMatrix, & boxCoord );
	DO->vert[3]  = (float)objCoord.x;				// V1
	DO->vert[4]  = (float)objCoord.y; 
	DO->vert[5]  = (float)objCoord.z;

	boxCoord = obbox_in->boxCenterBoxCoord + double3(-1.0*obbox_in->boxDimension.x,-1.0*obbox_in->boxDimension.y,+1.0*obbox_in->boxDimension.z);
	MatrixMult( & objCoord, & obbox_in->orientMatrix, & boxCoord );
	DO->vert[6]  = (float)objCoord.x;				// V2
	DO->vert[7]  = (float)objCoord.y; 
	DO->vert[8]  = (float)objCoord.z;

	boxCoord = obbox_in->boxCenterBoxCoord + double3(+1.0*obbox_in->boxDimension.x,-1.0*obbox_in->boxDimension.y,+1.0*obbox_in->boxDimension.z);
	MatrixMult( & objCoord, & obbox_in->orientMatrix, & boxCoord );
	DO->vert[9]  = (float)objCoord.x;				// V3
	DO->vert[10] = (float)objCoord.y; 
	DO->vert[11] = (float)objCoord.z;

	boxCoord = obbox_in->boxCenterBoxCoord + double3(-1.0*obbox_in->boxDimension.x,+1.0*obbox_in->boxDimension.y,-1.0*obbox_in->boxDimension.z);
	MatrixMult( & objCoord, & obbox_in->orientMatrix, & boxCoord );
	DO->vert[12] = (float)objCoord.x;				// V4
	DO->vert[13] = (float)objCoord.y; 
	DO->vert[14] = (float)objCoord.z;

	boxCoord = obbox_in->boxCenterBoxCoord + double3(+1.0*obbox_in->boxDimension.x,+1.0*obbox_in->boxDimension.y,-1.0*obbox_in->boxDimension.z);
	MatrixMult( & objCoord, & obbox_in->orientMatrix, & boxCoord );
	DO->vert[15] = (float)objCoord.x;				// V5
	DO->vert[16] = (float)objCoord.y; 
	DO->vert[17] = (float)objCoord.z;

	boxCoord = obbox_in->boxCenterBoxCoord + double3(-1.0*obbox_in->boxDimension.x,+1.0*obbox_in->boxDimension.y,+1.0*obbox_in->boxDimension.z);
	MatrixMult( & objCoord, & obbox_in->orientMatrix, & boxCoord );
	DO->vert[18] = (float)objCoord.x;				// V6
	DO->vert[19] = (float)objCoord.y; 
	DO->vert[20] = (float)objCoord.z;

	boxCoord = obbox_in->boxCenterBoxCoord + double3(+1.0*obbox_in->boxDimension.x,+1.0*obbox_in->boxDimension.y,+1.0*obbox_in->boxDimension.z);
	MatrixMult( & objCoord, & obbox_in->orientMatrix, & boxCoord );
	DO->vert[21] = (float)objCoord.x;				// V7
	DO->vert[22] = (float)objCoord.y; 
	DO->vert[23] = (float)objCoord.z;
	
	// write the grid's index data.
	DO->ind[0]  = 0; DO->ind[1]  = 1; // Line 1
	DO->ind[2]  = 1; DO->ind[3]  = 5; // Line 2
	DO->ind[4]  = 5; DO->ind[5]  = 4; // Line 3
	DO->ind[6]  = 4; DO->ind[7]  = 0; // Line 4
	DO->ind[8]  = 0; DO->ind[9]  = 2; // Line 5
	DO->ind[10] = 1; DO->ind[11] = 3; // Line 6
	DO->ind[12] = 5; DO->ind[13] = 7; // Line 7
	DO->ind[14] = 4; DO->ind[15] = 6; // Line 8
	DO->ind[16] = 2; DO->ind[17] = 3; // Line 9
	DO->ind[18] = 3; DO->ind[19] = 7; // Line 10
	DO->ind[20] = 7; DO->ind[21] = 6; // Line 11
	DO->ind[22] = 2; DO->ind[23] = 6; // Line 12

	// Initilize debug object for rendering
	DO->InitForRendering(DO_LINES);

}
/************************************************************************/
/* Name:		AddObboxTriDebugObject									*/
/* Description:	Make an exact copy of the current Obbox's triangles		*/
/*              Second input is a D3DCOLOR value						*/
/************************************************************************/
void debug_object::AddObboxTriDebugObject( obbox * obbox_in, D3DCOLOR color )
{
#ifdef _DEBUG
	if( obbox_in == NULL )
		throw std::runtime_error("debug_object::AddObboxTriDebugObject: NULL input to function");
	if( !obbox_in->isLeaf )
		throw std::runtime_error("debug_object::AddObboxTriDebugObject: Input obbox is not a leaf");
#endif

	// Make a new debug object on the heap
	debug_object * DO = new debug_object;

	// Add it to the linked list of objects
	if(g_renderer->debug_objects) 
		g_renderer->debug_objects->InsertAtEnd(DO); // Insert at end of linked-list
	else
		g_renderer->debug_objects = DO; // Otherwise start a new linked-list if empty

	DO->type = DO_TRIANGLES;
	DO->numVert = (obbox_in->numFaces * 3); // 3 indicies per face
	DO->numInd = obbox_in->numFaces * 3 * 2; // Also store back to front indicies to render both sides

	DO->vert = new float[DO->numVert*3]; // Three floats per vertex (x.y.z)
	DO->ind = new UINT[DO->numInd];

	DO->matWorld = obbox_in->rbo->matWorld;

	DO->color = color;
	
	// Copy over verticies into flat array and set indicies 0,1,2,....,n
	// Note: obbox indicies are stored in array t->indicies which is shared with all leaf nodes.
	//       The correct indicies start:  t->indices[indices]
	//							  finish: t->indicies[indicies + 3*numFaces - 1]
	for(UINT i = 0; i < (UINT)(obbox_in->numFaces * 3); i ++)
	{
		DO->vert[3*i + 0] =  obbox_in->rbo->vertexBuffer[ obbox_in->t->indices[ obbox_in->indices + i]].x;	// Vertex i.x
		DO->vert[3*i + 1] =  obbox_in->rbo->vertexBuffer[ obbox_in->t->indices[ obbox_in->indices + i]].y;	// Vertex i.y
		DO->vert[3*i + 2] =  obbox_in->rbo->vertexBuffer[ obbox_in->t->indices[ obbox_in->indices + i]].z;	// Vertex i.z
		DO->ind[i] = i;
	}
	// Now render back to front
	for(UINT i = (UINT)(obbox_in->numFaces * 3); i < (UINT)((obbox_in->numFaces * 3) * 2); i ++)
	{
		DO->ind[i] = (UINT)((obbox_in->numFaces * 3) * 2 - i - 1);
	}

	// Initilize debug object for rendering
	DO->InitForRendering(DO_TRIANGLES);

}