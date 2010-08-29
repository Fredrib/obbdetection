// File:		obbox.h
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

// This is primary OBB tree data structure to impliment the paper:
// S. Gottschalk et. al. "OBBTree: A Hierarchical Structure for Rapid Iterference Detection"

#ifndef obbox_h
#define obbox_h

#include "main.h"
#include "hull.h"
#include "obbox_renderitems.h"

typedef unsigned int UINT;
typedef unsigned int * PUINT;

class rbobject;
class obbtree;

class obbox 
{
public:
									obbox();
									obbox(int numFacesIn,  int parentIn, rbobject * rboIn, int depthIn);
									~obbox();

	obbox & operator = (const obbox & o); // Assignment operator for vector array in obbtree (when resizing)

	friend class rbobject; // This is lazy I know!
	friend class rbmesh;

	void							BuildOBBTree(int curIndex, vector<int> * obbStack); // build one level of the OBB tree

	// Helper functions to build OBB Tree
	static void						AreaConvexhullTriangle(double * ret, double * p, double * q, double * r);
	static double					AreaConvexhullTriangle(const double3 * p, const double3 * q, const double3 * r); // Also a double3 version
	static void						CentroidConvexhullTriangle(double3 * ret, double * p, double * q, double * r);
	bool							SplitAxes(double3 axis, double splitPoint, int * child1Index, int * child1NumIndicies, int * child2Index, int * child2NumIndicies);
	void							OrderAxes(double3 * boxDimension);

	// Convex Hull Generator --> OLD VERSION OF CODE: MOVED TO:   hull.cpp::SM_chull()
	//	void							GetConvexHull_SMelax(double * vertexSet, int uniqueIndexSetSize, HullDesc * desc, PHullResult * hr);


	// Two versions of the same thing...  Find out later which is fater
	static void						FindUniqueIndexSet_BoolArray(int ** retArray, int * retSize, int * pArray, int numElements);
	static void						FindUniqueIndexSet_HashSet(int ** retArray, int * retSize, int * pArray, int pArray_start, int numElements); // More memory efficient I think

	// Rendering (mostly for debugging)
	void							initConvexHullForRendering(void);
	void							initOBBForRendering(void);
	void							initOBBForRenderingLines(void);
	void							DrawConvexHull(D3DXMATRIX * mat);
	void							DrawOBB(D3DXMATRIX * mat);

	// Collision tests
	static int						TestOBBCollision( obbox * OBB1, obbox * OBB2 );
	static int						TestTriCollision( obbox * OBB1, obbox * OBB2 );

// BAD OOP --> BUT EASIER IF THESE ARE PUBLIC
// private:
	int								parent, childNode1, childNode2;
	rbobject *						rbo;
	obbtree *						t;
	double3							orient[3];				// Vectors form OBB axes (orthonormal eigen basis)
	double3x3						orientMatrix;			// This is is the (U,V,N)->(X,Y,Z) local to world space transform.  Axes run in columns (vertically)
	double3							boxDimension;			// 3 Box dimensions (along axes) -> length / 2
	double3							boxCenterObjectCoord;	// The boxes center in object coordinates
	double3							boxCenterBoxCoord;

	// List of triangle vertex indices (groups of three indices to a triangle)
	// Match indices to verticies through rbo object --> Only for leaf nodes
	int								indices; // This is a starting index in the obbtree "indices" array.  Avoid dynamic memory for every cell.
	int								numFaces; // numindices = numFaces * 3
	bool							isLeaf;
	int								depth;

	// Render items + saved convex hull now moved to obbox_renderitems class to save space in obbox array
	obbox_renderitems *				renderitems;

	/********************************************************************/
	/***** IF YOU ADD MEMBERS, ADD THEM TO THE ASSIGNMENT OPERATOR ******/
	/********************************************************************/
};

bool CheckForDuplicateVertex(int numIndicies, int * indicies, D3DXVECTOR3 * verticies);

#endif