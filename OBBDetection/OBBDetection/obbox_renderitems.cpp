/*************************************************************
**				obbox_renderitems class						**
*************************************************************/
// File:		obbox_renderitems.cpp
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

#include	"obbox_renderitems.h"

obbox_renderitems::obbox_renderitems(void)
{
	cHullVert = NULL;
	cHullVertCount = 0;
	cHullInd = NULL;
	cHullIndCount = 0;
	convexHullVertexBuffer = NULL;
	convexHullIndexBuffer = NULL;
	convexHullVertexBufferSize = 0;
	convexHullIndexBufferSize = 0;
	OBBVertexBuffer = NULL;
	OBBIndexBuffer = NULL;
	OBBVertexBufferSize = 0;
	OBBIndexBufferSize = 0;	
}
obbox_renderitems::~obbox_renderitems(void)
{
	ReleaseCOM(convexHullVertexBuffer);
	ReleaseCOM(convexHullIndexBuffer);
	ReleaseCOM(OBBVertexBuffer);
	ReleaseCOM(OBBIndexBuffer);	
	if(cHullVert) { delete [] cHullVert; cHullVert = NULL; }
	if(cHullInd) { delete [] cHullInd; cHullInd = NULL; }
}