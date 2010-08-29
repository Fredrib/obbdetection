// File:		obbox.cpp
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

// This is primary OBB tree data structure to impliment the paper:
// S. Gottschalk et. al. "OBBTree: A Hierarchical Structure for Rapid Iterference Detection"

#include "main.h"
#include "tritri_isectline.h"

/************************************************************************/
/* Name:		obbox													*/
/* Description: Blank Constructor										*/
/************************************************************************/
obbox::obbox()
{
	parent = -1; childNode1 = -1; childNode2 = -1; 
	indices = -1; // will throw a warning if you try and dereference it
	numFaces = 0; rbo = NULL;
	depth = 0;
	t = NULL;
	renderitems = NULL;

}
/************************************************************************/
/* Name:		obbox													*/
/* Description: Constructor, initialize with vertex length n.			*/
/************************************************************************/
obbox::obbox(int numFacesIn, int parentIn, rbobject * rboIn, int depthIn)
{
	t = NULL; 
	parent = parentIn;
	childNode1 = -1; childNode2 = -1;
	indices = -1; // will throw a warning if you try and dereference it
	numFaces = numFacesIn;
	rbo = rboIn;
	renderitems = NULL;
	depth = depthIn;
}
/************************************************************************/
/* Name:		~obbox													*/
/* Description: Destructor, Recursively delete the entire tree.			*/
/************************************************************************/
obbox::~obbox()
{
	if(renderitems) { delete renderitems; renderitems = NULL; }
}
/************************************************************************/
/* Name:		operator =												*/
/* Description: Assignment operator for vector array in obbtree			*/
/*				(when resizing)											*/
/************************************************************************/
obbox & obbox::operator = (const obbox & o)
{
	if (this != &o) // make sure not same object
	{
		// Perform shallow copies (same order as obbox.h if you need to check)
		parent = o.parent; 
		childNode1 = o.childNode1; 
		childNode2 = o.childNode2; 
		rbo = o.rbo;	
		t = o.t;
		orient[0] = o.orient[0];
		orient[1] = o.orient[1];
		orient[2] = o.orient[2];
		boxDimension = o.boxDimension;
		boxCenterObjectCoord = o.boxCenterObjectCoord;
		boxCenterBoxCoord = o.boxCenterBoxCoord;
		indices = o.indices;
		numFaces = o.numFaces; 
		isLeaf = o.isLeaf;
		depth = o.depth;
		renderitems = o.renderitems;
	}
	return *this;  // Return ref for multiple assignment
}

/************************************************************************/
/* Name:		BuildOBBTree											*/
/* Description: Builds one level of the OBB tree						*/
/* Gottschalk et.al. "OBBTree: A Hierarchical..." pg 3 implimented here */
/************************************************************************/
void obbox::BuildOBBTree(int curIndex, vector<int> * obbStack)
{
#ifdef _DEBUG
	if(numFaces < 1)
		throw std::runtime_error("BuildOBBTree: Something went wrong! --> numFaces at current node < 1!!");
#endif

	if(depth+1 <= g_app->GetObbRenderDepth())
		renderitems = new obbox_renderitems;
	else
		renderitems = NULL;

	// Only find convex hull if numFaces > 1
	if(numFaces == 1)
	{
		// This will be a leaf node, just use
		t->cHullIndCount = 3; 
		t->cHullInd[0] = 0; // Point 1
		t->cHullInd[1] = 1; // Point 2
		t->cHullInd[2] = 2; // Point 3

		t->cHullVertCount = 3;
		t->cHullVert[0] = rbo->vertexBuffer[t->indices[indices + 0]].x; // Point 1 = p later on
		t->cHullVert[1] = rbo->vertexBuffer[t->indices[indices + 0]].y;
		t->cHullVert[2] = rbo->vertexBuffer[t->indices[indices + 0]].z;
		t->cHullVert[3] = rbo->vertexBuffer[t->indices[indices + 1]].x; // Point 2 = q later on
		t->cHullVert[4] = rbo->vertexBuffer[t->indices[indices + 1]].y;
		t->cHullVert[5] = rbo->vertexBuffer[t->indices[indices + 1]].z;
		t->cHullVert[6] = rbo->vertexBuffer[t->indices[indices + 2]].x; // Point 3 = r later on
		t->cHullVert[7] = rbo->vertexBuffer[t->indices[indices + 2]].y;
		t->cHullVert[8] = rbo->vertexBuffer[t->indices[indices + 2]].z;

		t->uniqueIndexSetSize = 3;
		t->uniqueIndexSet[0] = t->indices[indices + 0]; // Point 1
		t->uniqueIndexSet[1] = t->indices[indices + 1]; // Point 1
		t->uniqueIndexSet[2] = t->indices[indices + 2]; // Point 1
	}
	else
	{
// 1. Find an index set with non-repeating (i.e. unique) indices --> For convex hull
		FindUniqueIndexSet_HashSet(&t->uniqueIndexSet, &t->uniqueIndexSetSize, t->indices, indices, numFaces*3); // Hash table is O(n)
		// Collect the verticies in an array --> REQUIRED FOR INPUT TO MOST CONVEX HULL GENERATORS (groups of 3 floats)
		// NEED TO ADD SMALL DEVIATION TO EACH VERTEX --> HELPS CONVEX HULL CONVERGE
		if(ADD_VECTOR_PRETURB)
			srand(1000); // Seed random number generator if we're going to use it.
		double randNum = 0; 
		double lowRand = -0.000001;  double rangeRand = 0.000002; // Gives rand [-0.000001, 0.000001)
		for(int i = 0; i < t->uniqueIndexSetSize; i ++)
		{
			if(ADD_VECTOR_PRETURB)
				randNum = static_cast<double>( rand() ) * rangeRand / static_cast<double>( RAND_MAX ) + lowRand ;
			t->vertexSet[i*3 + 0] = rbo->vertexBuffer[t->uniqueIndexSet[i]].x + randNum;
			if(ADD_VECTOR_PRETURB)
				randNum = static_cast<double>( rand() ) * rangeRand / static_cast<double>( RAND_MAX ) + lowRand ;
			t->vertexSet[i*3 + 1] = rbo->vertexBuffer[t->uniqueIndexSet[i]].y + randNum;
			if(ADD_VECTOR_PRETURB)
				randNum = static_cast<double>( rand() ) * rangeRand / static_cast<double>( RAND_MAX ) + lowRand ;
			t->vertexSet[i*3 + 2] = rbo->vertexBuffer[t->uniqueIndexSet[i]].z + randNum;
		}

// 2. a) Find the convex hull of the unique set
/************************************************************************/
/* SOURCES I HAVE TRIED													*/
/************************************************************************/
/* i. Stan Melax's convex hull generator								*/
/*		 --> Bad with degenerate models. but fast.						*/
/************************************************************************/
/* ii. qhull convex hull generator										*/
/*       --> Poorly written library										*/
/************************************************************************/
/* iii. Use CGAL library												*/
/*		 --> Robust but very slow.										*/
/************************************************************************/
/* iv. Just use covarience matrix of model								*/
/*		 --> Defeats the purpose of the whole algorithm					*/
/************************************************************************/
		if(!FIND_CONVEXHULL)
		{
			Copy_Mesh_To_CH(this, t->vertexSet, t->uniqueIndexSetSize*3, &t->cHullVert, &t->cHullVertCount, &t->cHullInd, &t->cHullIndCount );
		}
		else
		{
			// Try using StanHull to create the convex hull
			int retVal = 0;
			if(!CGAL_ONLY)
				retVal = SM_chull(t->vertexSet, t->uniqueIndexSetSize*3, &t->cHullVert, &t->cHullVertCount, &t->cHullInd, &t->cHullIndCount );
			if(!retVal || t->cHullIndCount <= 3) // Sometimes StanHull will return one triangle convex hull!
			{
				// If StanHull Failed try CGAL
				if(!CGAL_chull(t->vertexSet, t->uniqueIndexSetSize*3, &t->cHullVert, &t->cHullVertCount, &t->cHullInd, &t->cHullIndCount) 
				   || t->cHullIndCount <= 3)
				{
					// If CGAL failed OR the model is degenerate (CH is a line segment, etc) just use the origional mesh.
					Copy_Mesh_To_CH(this, t->vertexSet, t->uniqueIndexSetSize*3, &t->cHullVert, &t->cHullVertCount, &t->cHullInd, &t->cHullIndCount );
				}
			}
		}
	}

// 2. b) Initialize convex hull for rendering if we want and save it for later (to save to disk)
	if(depth+1 <= g_app->GetObbRenderDepth())
	{
		// Make a copy of convex hull
		renderitems->cHullVertCount = t->cHullVertCount;
		renderitems->cHullVert = new double[renderitems->cHullVertCount*3];
		for(UINT i = 0; i < renderitems->cHullVertCount*3; i ++)
			renderitems->cHullVert[i] = t->cHullVert[i];
		renderitems->cHullIndCount = t->cHullIndCount;
		renderitems->cHullInd = new UINT[renderitems->cHullIndCount];
		for(UINT i = 0; i < renderitems->cHullIndCount; i ++)
			renderitems->cHullInd[i] = t->cHullInd[i];

		// Initialize the convex hull for rendering
		initConvexHullForRendering();
	}

	// Only use covarience matrix to find OBB if numFaces > 1
	if(t->cHullIndCount == 3) // One face --> MATH CHECKED BY MATLAB
	{
		// Extract the verticies
		double * p = &t->cHullVert[t->cHullInd[0]*3]; // Vertex 0
		double * q = &t->cHullVert[t->cHullInd[1]*3]; // Vertex 1
		double * r = &t->cHullVert[t->cHullInd[2]*3]; // Vertex 2	

		// Use an edge as the first axis (Edge1_p2q)
		double3 Edge1_p2q(q[0] - p[0],q[1] - p[1],q[2] - p[2]);
		t->eigVec[0] = Edge1_p2q;
		t->eigVec[0] = safenormalize(t->eigVec[0]); // Normalize it

		// Use the cross product of two edges as the second axis (Edge1_p2q x Edge2_p2r)
		double3 Edge2_p2r(r[0] - p[0],r[1] - p[1],r[2] - p[2]);
		t->eigVec[1] = cross(Edge1_p2q, Edge2_p2r);		
		t->eigVec[1] = safenormalize(t->eigVec[1]); // Normalize it

		// Use the cross product of those two edges as the third axes
		// According to Right-Hand-Rule (where index is A, middle is B and thumb is A x B), use eigVec[1] as A
		t->eigVec[2] = cross(t->eigVec[1], t->eigVec[0]);
		t->eigVec[2] = safenormalize(t->eigVec[2]); // Normalize it (though should already be normalized)
	}
	else
	{
// 3. Find a) total convex hull surface area, b) the convex hull centroid, c) Covarience Matrix
	// --> Use S. Melax's double classes for better precision (I had trouble with poor precision)
	// Also, do two loops through faces --> Could be done in one, but this is easier to read and O(n) anyway.
		t->A_H = 0; t->A_i = 0;
		t->c_H.x = 0.0; t->c_H.y = 0.0; t->c_H.z = 0.0; 
		t->c_i.x = 0.0; t->c_i.y = 0.0; t->c_i.z = 0.0;
		t->Cov[0][0] = 0.0; t->Cov[0][1] = 0.0; t->Cov[0][2] = 0.0; // Zero covarience matrix 
		t->Cov[1][0] = 0.0; t->Cov[1][1] = 0.0; t->Cov[1][2] = 0.0;
		t->Cov[2][0] = 0.0; t->Cov[2][1] = 0.0; t->Cov[2][2] = 0.0;

		for(UINT i = 0; i < t->cHullIndCount; i += 3)
		{
			double * p = &t->cHullVert[t->cHullInd[i + 0]*3]; // Vertex 0
			double * q = &t->cHullVert[t->cHullInd[i + 1]*3]; // Vertex 1
			double * r = &t->cHullVert[t->cHullInd[i + 2]*3]; // Vertex 2
			AreaConvexhullTriangle(&t->A_i, p, q, r);
			CentroidConvexhullTriangle(&t->c_i, p, q, r);
			t->c_H.x += t->A_i * t->c_i.x;	  t->c_H.y += t->A_i * t->c_i.y;	  t->c_H.z += t->A_i * t->c_i.z;
			t->A_H += t->A_i;
		}
		if(abs(t->A_H-0.0f) < 0.0000001)
			throw std::runtime_error("BuildOBBTree: Convex Hull Surface Area is zero!");
		t->c_H.x = t->c_H.x / t->A_H; t->c_H.y = t->c_H.y / t->A_H; t->c_H.z = t->c_H.z / t->A_H; // Need a weighted average

		// Now find Covarience Matrix --> Could be included into top for loop, but this is easier to debug.
		for(UINT i = 0; i < t->cHullIndCount; i +=3)
		{
			double * p = &t->cHullVert[t->cHullInd[i + 0]*3]; // Vertex 0
			double * q = &t->cHullVert[t->cHullInd[i + 1]*3]; // Vertex 
			double * r = &t->cHullVert[t->cHullInd[i + 2]*3]; // Vertex 2
			AreaConvexhullTriangle(&t->A_i, p, q, r); // (Verified)
			CentroidConvexhullTriangle(&t->c_i, p, q, r); // (Verified)
			t->Cov[0][0] += (t->A_i/12.0) * (9*t->c_i.x*t->c_i.x + p[0]*p[0] + q[0]*q[0] + r[0]*r[0]); // C_{jk}, where j = 0, k = 0		-> Or .x .x
			t->Cov[0][1] += (t->A_i/12.0) * (9*t->c_i.x*t->c_i.y + p[0]*p[1] + q[0]*q[1] + r[0]*r[1]); // C_{jk}, where j = 0, k = 1
			t->Cov[0][2] += (t->A_i/12.0) * (9*t->c_i.x*t->c_i.z + p[0]*p[2] + q[0]*q[2] + r[0]*r[2]); // C_{jk}, where j = 0, k = 2
			t->Cov[1][1] += (t->A_i/12.0) * (9*t->c_i.y*t->c_i.y + p[1]*p[1] + q[1]*q[1] + r[1]*r[1]); // C_{jk}, where j = 1, k = 1
			t->Cov[1][2] += (t->A_i/12.0) * (9*t->c_i.y*t->c_i.z + p[1]*p[2] + q[1]*q[2] + r[1]*r[2]); // C_{jk}, where j = 1, k = 2
			t->Cov[2][2] += (t->A_i/12.0) * (9*t->c_i.z*t->c_i.z + p[2]*p[2] + q[2]*q[2] + r[2]*r[2]); // C_{jk}, where j = 2, k = 2
		}
		t->Cov[0][0] = (t->Cov[0][0]/t->A_H) - t->c_H.x*t->c_H.x; // C_{jk}, where j = 0, k = 0		-> Or .x .x
		t->Cov[0][1] = (t->Cov[0][1]/t->A_H) - t->c_H.x*t->c_H.y; // C_{jk}, where j = 0, k = 1
		t->Cov[0][2] = (t->Cov[0][2]/t->A_H) - t->c_H.x*t->c_H.z; // C_{jk}, where j = 0, k = 2
		t->Cov[1][1] = (t->Cov[1][1]/t->A_H) - t->c_H.y*t->c_H.y; // C_{jk}, where j = 1, k = 1
		t->Cov[1][2] = (t->Cov[1][2]/t->A_H) - t->c_H.y*t->c_H.z; // C_{jk}, where j = 1, k = 2
		t->Cov[2][2] = (t->Cov[2][2]/t->A_H) - t->c_H.z*t->c_H.z; // C_{jk}, where j = 2, k = 2	

		// Add terms missed before --> Build Symmetric matrix
		t->Cov[1][0] = t->Cov[0][1]; 
		t->Cov[2][0] = t->Cov[0][2];
		t->Cov[2][1] = t->Cov[1][2];

// 6. Find eigen vectors of covarience matrix
	// getSymMatEigenvectorsMelax(&t->eigVec[0], Cov); // Wrapper function for S. Melax code --> DOESN'T WORK!
		getSymMatEigenvectors(&t->eigVec[0], t->Cov); // Wrapper function for adapted java lib code

// 7. Normalize eigen vectors to form eigen basis
		t->eigVec[0] = safenormalize(t->eigVec[0]); // We shouldn't need to normalize, but do it anyway.
		t->eigVec[1] = safenormalize(t->eigVec[1]);
		t->eigVec[2] = safenormalize(t->eigVec[2]);
		if(!isBasis(&t->eigVec[0],&t->eigVec[1],&t->eigVec[2]))
			throw std::runtime_error("BuildOBBTree: eigenVectors do not form orthonormal basis!");
		if(!isRotation(&double3x3(t->eigVec[0],t->eigVec[1],t->eigVec[2])))
			throw std::runtime_error("BuildOBBTree: eigenVectors do not form a rotational matrix (with eigenvectors as rows)!");
	}

// 8. Find extremal vertecies along each axis (of the eigen basis) to find box dimensions
	t->minVec0 = 0.0; t->maxVec0 = 0.0; t->minVec1 = 0.0; t->maxVec1 = 0.0; t->minVec2 = 0.0; t->maxVec2 = 0.0;
	t->curVertex.x = 0.0; t->curVertex.y = 0.0; t->curVertex.z = 0.0;
	t->projection = 0.0; t->mean0 = 0.0; t->mean1 = 0.0; t->mean2 = 0.0;

	// Note: Projection of a 3D point onto a line is proj = A.B / ||B||
	// http://www.euclideanspace.com/maths/geometry/elements/line/projections/index.htm 
	// (Helpful for later: http://www.euclideanspace.com/maths/geometry/elements/plane/lineOnPlane/index.htm)

	// Initialize min and max with first vertex
	t->curVertex.x = rbo->vertexBuffer[t->uniqueIndexSet[0]].x;
	t->curVertex.y = rbo->vertexBuffer[t->uniqueIndexSet[0]].y;
	t->curVertex.z = rbo->vertexBuffer[t->uniqueIndexSet[0]].z;
	t->projection = dot(t->curVertex,t->eigVec[0]); t->maxVec0 = t->projection; t->minVec0 = t->projection;
	t->projection = dot(t->curVertex,t->eigVec[1]); t->maxVec1 = t->projection; t->minVec1 = t->projection;
	t->projection = dot(t->curVertex,t->eigVec[2]); t->maxVec2 = t->projection; t->minVec2 = t->projection;

	// Initialize mean point with first vertex
	t->mean0 = t->maxVec0; t->mean1 = t->maxVec1; t->mean2 = t->maxVec2;

	// Go through each vertex and find min and max O(n)
	// While we're looping through, also calculate mean verticies along each axis (used later on)
	for(int i = 1; i < t->uniqueIndexSetSize; i ++)
	{
		// We have box center and vertex in object coords --> take subtraction to find vertex from box center.
		t->curVertex.x = rbo->vertexBuffer[t->uniqueIndexSet[i]].x;
		t->curVertex.y = rbo->vertexBuffer[t->uniqueIndexSet[i]].y;
		t->curVertex.z = rbo->vertexBuffer[t->uniqueIndexSet[i]].z;
		t->projection = dot(t->curVertex,t->eigVec[0]);
		if(t->projection > t->maxVec0) { t->maxVec0 = t->projection; }
		if(t->projection < t->minVec0) { t->minVec0 = t->projection; }
		t->mean0 += t->projection;
		t->projection = dot(t->curVertex,t->eigVec[1]);
		if(t->projection > t->maxVec1) { t->maxVec1 = t->projection; }
		if(t->projection < t->minVec1) { t->minVec1 = t->projection; }
		t->mean1 += t->projection;
		t->projection = dot(t->curVertex,t->eigVec[2]);
		if(t->projection > t->maxVec2) { t->maxVec2 = t->projection; }
		if(t->projection < t->minVec2) { t->minVec2 = t->projection; }
		t->mean2 += t->projection;
	}
	boxDimension.x = (float)((t->maxVec0 - t->minVec0)/2.0);  // Half the length of a box
	boxDimension.y = (float)((t->maxVec1 - t->minVec1)/2.0); 
	boxDimension.z = (float)((t->maxVec2 - t->minVec2)/2.0);
	t->mean0 = t->mean0 / t->uniqueIndexSetSize;
	t->mean1 = t->mean1 / t->uniqueIndexSetSize;
	t->mean2 = t->mean2 / t->uniqueIndexSetSize;

	// Make box lengths non-zero (helps catch collisions for face-face collisions in close proximity
	if(abs(boxDimension.x - 0.0)<EPSILON) { boxDimension.x = 0.0000001; }
	if(abs(boxDimension.y - 0.0)<EPSILON) { boxDimension.y = 0.0000001; }
	if(abs(boxDimension.z - 0.0)<EPSILON) { boxDimension.z = 0.0000001; }
	
// 9. Find the rotation matrix and translation matrix, from what we have found.
	// RELATIVE TO OBJECT COORDINATES, NOT PARENT OBB, AS IN PAPER!!
	// change of basis from (x,y,z)->(u,v,n) has vectors as rows.  (u,v,n)->(x,y,z) has vectors as columns
	// Row's run horizontally, columns run vertically.
	orient[0] = double3(t->eigVec[0].x,t->eigVec[0].y,t->eigVec[0].z);
	orient[1] = double3(t->eigVec[1].x,t->eigVec[1].y,t->eigVec[1].z);
	orient[2] = double3(t->eigVec[2].x,t->eigVec[2].y,t->eigVec[2].z);
	boxCenterBoxCoord = double3((t->maxVec0 + t->minVec0)/2.0,(t->maxVec1 + t->minVec1)/2.0,(t->maxVec2 + t->minVec2)/2.0); // Box center in box coordinates 
	// c = [0.5 * (l1 + u1) * v1] + [0.5 * (l2 + u2) * v2] + [0.5 * (l3 + u3) * v3]
	
	// Transform local to object coordinates, ie do a (U,V,N) to (X,Y,Z) transform with vectors as the columns
	orientMatrix.x = double3(t->eigVec[0].x,t->eigVec[1].x,t->eigVec[2].x);
	orientMatrix.y = double3(t->eigVec[0].y,t->eigVec[1].y,t->eigVec[2].y);
	orientMatrix.z = double3(t->eigVec[0].z,t->eigVec[1].z,t->eigVec[2].z); // Modal Matrix form, eigenvectors are colomn-wise

	// Transform box center from local to object coordinates
	MatrixMult(&boxCenterObjectCoord, &orientMatrix, &boxCenterBoxCoord);

// 9.A) Initialize obb for rendering
	if(depth+1 <= g_app->GetObbRenderDepth())
	{
		if(DRAW_OBB_AS_LINES)
			initOBBForRenderingLines();
		else
			initOBBForRendering();
	}

// 10. Find mean point of verticies along largest axis (to find split point) --> If not possible, use second then third largest axes.
	// Already found mean points during 8
	OrderAxes(&boxDimension); 
	// Results will be:
	// double3	t->axisOrder[3];  // [Largest, Middle, Smallest]			--> NOW USE SCRAP PAD (in obbtree)
	// double	t->splitPoint[3];											--> NOW USE SCRAP PAD (in obbtree)

// 11. Split axes between two children
	if(numFaces > 1 && !ONE_LEVEL_OBBTREE) // ONE_LEVEL_OBBTREE = Stop the recursion to debug convex hull generation
	{
		t->splitOK = SplitAxes(t->axisOrder[0],t->splitPoint[0],t->indices_child1,&t->child1NumIndicies,t->indices_child2,&t->child2NumIndicies);
		if(!t->splitOK)
		{
			t->splitOK = SplitAxes(t->axisOrder[1],t->splitPoint[1],t->indices_child1,&t->child1NumIndicies,t->indices_child2,&t->child2NumIndicies);
			if(!t->splitOK)
			{
				t->splitOK = SplitAxes(t->axisOrder[2],t->splitPoint[2],t->indices_child1,&t->child1NumIndicies,t->indices_child2,&t->child2NumIndicies);
			}
		}
	}
	else
		t->splitOK = false;

	// Case 1: CANNOT RECURSIVELY SUBDIVIDE FURTHER OR WE'RE ON A LEAF NODE
	// "If the group of polygons cannot be partitioned along any axis by this criterion, then the group is considered indivisible"
	if(!t->splitOK) 
	{
		// No work to do on childNodes.
		childNode1 = -1; childNode2 = -1;
		isLeaf = 1;
		// Don't delete the indicies, keep them until closing.
	}
	else
	{
		isLeaf = 0;
	// Case 2: CAN SUBDIVIDE FURTHER.
// 12. Initialize child nodes with new index arrays
		t->GetNext(&childNode1); t->GetNext(&childNode2); // Get the next avaliable node indicies and their pointers
		obbox * p_childNode1 = t->GetAt(childNode1);
		obbox * p_childNode2 = t->GetAt(childNode2);

		p_childNode1->numFaces	= (int)(t->child1NumIndicies/3.0);
		p_childNode1->rbo		= rbo;
		p_childNode1->parent	= curIndex;
		p_childNode1->depth		= depth+1;
		p_childNode1->t			= t;

		p_childNode2->numFaces	= (int)(t->child2NumIndicies/3.0);
		p_childNode2->rbo		= rbo;
		p_childNode2->parent	= curIndex;
		p_childNode2->depth		= depth+1;
		p_childNode2->t			= t;

#ifdef _DEBUG
		if( (p_childNode1->numFaces + p_childNode2->numFaces) != numFaces )
			throw std::runtime_error("BuildOBBTree: Something went wrong.  There are more faces in child nodes than in parent!");
#endif

		// Copy children indices from temporary array into master array
		p_childNode1->indices = indices; // first child starts where parent starts
		for(int i = 0; i < t->child1NumIndicies; i ++)
			t->indices[p_childNode1->indices + i] = t->indices_child1[i];

		p_childNode2->indices = p_childNode1->indices + t->child1NumIndicies; // second child starts where first child finishes
		for(int i = 0; i < t->child2NumIndicies; i ++)
			t->indices[p_childNode2->indices + i] = t->indices_child2[i];

		indices = -1; // Like deleting index array --> will throw runtime error if we try to dereference index = -1

		// Recursively build OBB Tree --> Pass references to stack on higher level
		obbStack->push_back(childNode1);
		obbStack->push_back(childNode2);
	}
}

/************************************************************************/
/* Name:		GetConvexHull_SMelax									*/
/* Description: Use Stan Melax's convex hull generator					*/
/************************************************************************/
/*		OLD VERSION OF THE CODE. MOVED TO:   hull.cpp::SM_chull()		*/
/************************************************************************/
//void obbox::GetConvexHull_SMelax(double * vertexSet, int uniqueIndexSetSize, HullDesc * desc, PHullResult * hr)
//{
//	// --> SETUP THE INPUTS
//	desc->SetHullFlag(QF_TRIANGLES);			// Turn on TRIANGLE output
//	//desc.SetHullFlag(QF_SKIN_WIDTH);			// NO SKIN DEPTH BETWEEN VERTICIES AND MESH
//	desc->mVcount = uniqueIndexSetSize;			// number of vertices in the input point cloud
//	desc->mVertexStride = sizeof(double)*3;		// the stride of each vertex, in bytes.
//	desc->mMaxVertices = uniqueIndexSetSize;	// maximum number of vertices to be considered for the hull!
//	desc->mMaxFaces = uniqueIndexSetSize*15;	// Arbitrarily large
//	desc->mNormalEpsilon  = 0.0000001;		   // Very small number for testing duplicate points
//	desc->mSkinWidth = 0.0;						// No skin width
//	desc->mVertices = vertexSet;
//
//	if(!ComputeHull(desc->mVcount,desc->mVertices,*hr,desc->mMaxVertices,desc->mSkinWidth)) // THIS IS KINDA SLOW --> NOT MY FUNCTION. TRY QHULL, CGAL, OR T.CHAN'S 
//		throw std::runtime_error("BuildOBBTree: stanhull library failed to create a convex hull");
//	convexHullVertexBuffer =  NULL; convexHullIndexBuffer = NULL;
//}
/************************************************************************/
/* Name:		SplitAxes												*/
/* Description: Partition faces on two sides of plane formed by axes	*/
/*				and split point											*/
/* Gottschalk et. al. "OBBTree: A Hierarchical..." pg 4					*/
/************************************************************************/
bool obbox::SplitAxes(double3 axis, double splitPoint, int * child1Index, int * child1NumIndicies, int * child2Index, int * child2NumIndicies)
{
	// Parameterized plane: F(x) : (x-p1).n = 0, where x is a vector, p1 is a point on the plane, n is the unit normal to the plane
	double3 planePoint = axis * splitPoint;	// p1
	axis = safenormalize(axis);				// n (eigen vectors should have already been normalized)
	// Go through each trangle and partition it on either side of the plane
	*child1NumIndicies = 0; *child2NumIndicies = 0;
	double3 curCenter;
	double p[3]; double q[3]; double r[3];
	for(int i = 0; i < numFaces; i ++)
	{
		p[0] = (double)rbo->vertexBuffer[t->indices[indices + i*3 + 0]].x;	// Vertex 0.x
		p[1] = (double)rbo->vertexBuffer[t->indices[indices + i*3 + 0]].y;	// Vertex 0.y
		p[2] = (double)rbo->vertexBuffer[t->indices[indices + i*3 + 0]].z;	// Vertex 0.z
		q[0] = (double)rbo->vertexBuffer[t->indices[indices + i*3 + 1]].x;	// Vertex 1.x
		q[1] = (double)rbo->vertexBuffer[t->indices[indices + i*3 + 1]].y;	// Vertex 1.y
		q[2] = (double)rbo->vertexBuffer[t->indices[indices + i*3 + 1]].z;	// Vertex 1.z
		r[0] = (double)rbo->vertexBuffer[t->indices[indices + i*3 + 2]].x;	// Vertex 2.x
		r[1] = (double)rbo->vertexBuffer[t->indices[indices + i*3 + 2]].y;	// Vertex 2.y
		r[2] = (double)rbo->vertexBuffer[t->indices[indices + i*3 + 2]].z;	// Vertex 2.z
		CentroidConvexhullTriangle(&curCenter, p, q, r);
		double dotprod = dot(axis,curCenter - planePoint);
		if(abs(dotprod - 0.0)<EPSILON) // If the dot product is too small then we cannot partition (a point lies on the plane)
			return false;
		if(dotprod > 0.0)
		{
			child1Index[*child1NumIndicies + 0] = t->indices[indices + i*3 + 0]; // Vertex 0
			child1Index[*child1NumIndicies + 1] = t->indices[indices + i*3 + 1]; // Vertex 1
			child1Index[*child1NumIndicies + 2] = t->indices[indices + i*3 + 2]; // Vertex 2
			*child1NumIndicies += 3;
		}
		else
		{
			child2Index[*child2NumIndicies + 0] = t->indices[indices + i*3 + 0]; // Vertex 0
			child2Index[*child2NumIndicies + 1] = t->indices[indices + i*3 + 1]; // Vertex 1
			child2Index[*child2NumIndicies + 2] = t->indices[indices + i*3 + 2]; // Vertex 2
			*child2NumIndicies += 3;
		}
	}
	if(*child1NumIndicies<3 || *child2NumIndicies<3) // There isn't at least one triangle in each child...
		return false; // We didn't split anything --> All indicies went on one side!
	else
		return true;
}
/************************************************************************/
/* Name:		AreaConvexhullTriangle									*/
/* Description: Return the area of a single convex hull triangle		*/
/* Gottschalk et. al. "OBBTree: A Hierarchical..." pg 3					*/
/************************************************************************/
void obbox::AreaConvexhullTriangle(double * ret, double * p, double * q, double * r)
{
	double3 temp1(q[0] - p[0],q[1] - p[1],q[2] - p[2]);
	double3 temp2(r[0] - p[0],r[1] - p[1],r[2] - p[2]);
	double3 temp3 = cross(temp1, temp2);
	*ret = 0.5f * sqrt(temp3.x*temp3.x + temp3.y*temp3.y + temp3.z*temp3.z);
} 
double obbox::AreaConvexhullTriangle(const double3 * p, const double3 * q, const double3 * r)
{
	// Temp variables here might slow down function calls... But only done on startup!
	double3 temp1((p->x - q->x),(p->y - q->y),(p->z - q->z));
	double3 temp2((p->x - r->x),(p->y - r->y),(p->z - r->z));
	double3 temp3 = cross(temp1, temp2);
	return 0.5f * sqrt(temp3.x*temp3.x + temp3.y*temp3.y + temp3.z*temp3.z);
}
/************************************************************************/
/* Name:		CentroidConvexhullTriangle								*/
/* Description: Return the area of a single convex hull triangle		*/
/* Gottschalk et. al. "OBBTree: A Hierarchical..." pg 3					*/
/************************************************************************/
void obbox::CentroidConvexhullTriangle(double3 * ret, double * p, double * q, double * r)
{
	ret->x = (p[0] + q[0] + r[0]) / 3.0;
	ret->y = (p[1] + q[1] + r[1]) / 3.0;
	ret->z = (p[2] + q[2] + r[2]) / 3.0;
}
/************************************************************************/
/* Name:		FindUniqueIndexSet										*/
/* Description: Return an array of non-repeating indices - O(2n)		*/
/*		To methods listed (both order n), add values to a hash set OR   */
/*		use a boolian flag array.  Another approach could be a			*/
/*		divide-and-conquer or recursive sub-divisons and then merging??	*/
/************************************************************************/
// Solution 1 --> Use large boolian array
void obbox::FindUniqueIndexSet_BoolArray(int ** retArray, int * retSize, int * pArray, int numElements)
{
	// I used to do this using a hash_set --> But it turns out this brute force approach
	// is easier and about an order of magnitude faster.  At least this is faster than the obvious O(n^2) algorithm

	// Find largest index in the array
	int max_index = -1;
	for(int i = 0; i < numElements; i ++)
	{
		if(pArray[i] > max_index)
			max_index = pArray[i];
	}

	bool * bool_array = new bool[max_index+1]; // Very space inefficient
	if(bool_array == NULL)
		throw std::runtime_error("FindUniqueIndexSet: Not enough space for new");
	
	// Inialize all elements in the boolian array to false
	ZeroMemory(bool_array, sizeof(bool)*(max_index+1)); 

	// Mark each element in the array that exists
	*retSize = 0;
	for(int i = 0; i < numElements; i ++)
	{
		if(bool_array[pArray[i]] == false)
		{
			*retSize += 1;
			bool_array[pArray[i]] = true;
		}
	}

	*retArray = new int[*retSize];
	// Go through the boolian array and see what elements we have, and add them to the output.
	int index = 0; // Could do this with a list<int>...
	for(int i = 0; i < (max_index+1); i ++)
	{
		if(bool_array[i])
		{
			(*retArray)[index] = i;
			index ++;
		}
	}
}
void obbox::FindUniqueIndexSet_HashSet(int ** retArray, int * retSize, int * pArray, int pArray_start, int numElements)
// Solution 2 --> Use custom hash Table
// Looks clean here, but is in fact slower I think...  test later
{
	hashset_int uniqueSet(numElements, 4, 2); // numBuckets, starting bucket size, maxLoad
	for(int i = pArray_start; i < (pArray_start+numElements); i ++) // Add all elements to the hash set.
		uniqueSet.AddKey(&pArray[i]);
	uniqueSet.ReturnArray(retSize, retArray);
}
/************************************************************************/
/* Name:		initConvexHullForRendering								*/
/* Description: Sets up a D3D Mesh to render the convexHull Wireframe	*/
/************************************************************************/
void obbox::initConvexHullForRendering(void)
{
	renderitems->convexHullVertexBufferSize = renderitems->cHullVertCount; 
	renderitems->convexHullIndexBufferSize = 2*renderitems->cHullIndCount; // Render both sides

	// Obtain a pointer to a new vertex buffer.
	HR(g_app->m_pD3DDev->CreateVertexBuffer(renderitems->convexHullVertexBufferSize * sizeof(VertexPos), 
		D3DUSAGE_WRITEONLY,	0, D3DPOOL_MANAGED, &renderitems->convexHullVertexBuffer, 0),
		L"initConvexHullForRendering: CreateVertexBuffer failed: ");

	// Now lock it to obtain a pointer to its internal data, and write the grid's vertex data.
	VertexPos* v = 0;
	HR(renderitems->convexHullVertexBuffer->Lock(0, 0, (void**)&v, 0), L"initConvexHullForRendering: Failed to lock vertex buffer: ");
	for(UINT i = 0; i < renderitems->convexHullVertexBufferSize; ++i)
		v[i] = D3DXVECTOR3((float)renderitems->cHullVert[i*3 + 0],(float)renderitems->cHullVert[i*3 + 1],(float)renderitems->cHullVert[i*3 + 2]);
	HR(renderitems->convexHullVertexBuffer->Unlock(), L"initConvexHullForRendering: Failed to unlock vertex buffer: ");


	// Obtain a pointer to a new index buffer.
	// TWICE THE SIZE TO RENDER NEGATIVE AND POSITIVE NORMAL FACE SIDE.
	HR(g_app->m_pD3DDev->CreateIndexBuffer(2*renderitems->cHullIndCount*sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &renderitems->convexHullIndexBuffer, 0),
		L"initConvexHullForRendering: CreateIndexBuffer failed: ");
	
	// Now lock it to obtain a pointer to its internal data, and write the grid's index data.
	WORD* k = 0;
	HR(renderitems->convexHullIndexBuffer->Lock(0, 0, (void**)&k, 0), L"initConvexHullForRendering: Failed to lock index buffer: ");
	for(UINT i = 0; i < renderitems->cHullIndCount; i = i+3) // positive side
	{
		k[i+0] = (WORD)renderitems->cHullInd[i+0]; // vertex 1
		k[i+1] = (WORD)renderitems->cHullInd[i+1]; // vertex 1
		k[i+2] = (WORD)renderitems->cHullInd[i+2]; // vertex 1
	}
	UINT j = 0;
	for(UINT i = renderitems->cHullIndCount; i < 2*renderitems->cHullIndCount; i += 3) // positive side
	{
		k[i+0] = (WORD)renderitems->cHullInd[j+2]; // vertex 1
		k[i+1] = (WORD)renderitems->cHullInd[j+1]; // vertex 1
		k[i+2] = (WORD)renderitems->cHullInd[j+0]; // vertex 1
		j += 3;
	}
	HR(renderitems->convexHullIndexBuffer->Unlock(), L"initConvexHullForRendering: Failed to unlock index buffer: ");
}
/************************************************************************/
/* Name:		initOBBForRendering										*/
/* Description: Sets up D3D Mesh to render the OBB Wireframe recursively*/
/************************************************************************/
void obbox::initOBBForRendering()
{
	renderitems->OBBVertexBufferSize = 8; renderitems->OBBIndexBufferSize = 72; // 8 verticies in a box, (both sides of each triangle) 72 indicies to describe 12 triangles on 6 faces

	// Obtain a pointer to a new vertex buffer.
	HR(g_app->m_pD3DDev->CreateVertexBuffer(renderitems->OBBVertexBufferSize * sizeof(VertexPos), 
		D3DUSAGE_WRITEONLY,	0, D3DPOOL_MANAGED, &renderitems->OBBVertexBuffer, 0),
		L"initConvexHullForRendering: CreateVertexBuffer failed: ");

	// Now lock it to obtain a pointer to its internal data, and write the grid's vertex data.
	VertexPos* v = 0;
	HR(renderitems->OBBVertexBuffer->Lock(0, 0, (void**)&v, 0), L"initConvexHullForRendering: Failed to lock vertex buffer: ");

	// WORK OUT BOX CORNERS FOR VERTEX BUFFER IN OBJECT COORDINATES

	double3 boxCoord; double3 objCoord; // Some temporary variables
	boxCoord.x = (boxCenterBoxCoord.x - boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y - boxDimension.y); 
	boxCoord.z = (boxCenterBoxCoord.z - boxDimension.z);
	MatrixMult(&objCoord, &orientMatrix, &boxCoord); // Translate box corner to object coordinates
	v[0] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	boxCoord.x = (boxCenterBoxCoord.x + boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y - boxDimension.y); 
	boxCoord.z = (boxCenterBoxCoord.z - boxDimension.z);
	MatrixMult(&objCoord, &orientMatrix, &boxCoord);
	v[1] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	boxCoord.x = (boxCenterBoxCoord.x - boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y - boxDimension.y); 
	boxCoord.z = (boxCenterBoxCoord.z + boxDimension.z);	
	MatrixMult(&objCoord, &orientMatrix, &boxCoord);
	v[2] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	boxCoord.x = (boxCenterBoxCoord.x + boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y - boxDimension.y); 
	boxCoord.z = (boxCenterBoxCoord.z + boxDimension.z);
	MatrixMult(&objCoord, &orientMatrix, &boxCoord);
	v[3] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	boxCoord.x = (boxCenterBoxCoord.x - boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y + boxDimension.y); 
	boxCoord.z = (boxCenterBoxCoord.z - boxDimension.z);
	MatrixMult(&objCoord, &orientMatrix, &boxCoord);
	v[4] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	boxCoord.x = (boxCenterBoxCoord.x + boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y + boxDimension.y); 
	boxCoord.z = (boxCenterBoxCoord.z - boxDimension.z);
	MatrixMult(&objCoord, &orientMatrix, &boxCoord);
	v[5] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	boxCoord.x = (boxCenterBoxCoord.x - boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y + boxDimension.y); 
	boxCoord.z = (boxCenterBoxCoord.z + boxDimension.z);
	MatrixMult(&objCoord, &orientMatrix, &boxCoord);
	v[6] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	boxCoord.x = (boxCenterBoxCoord.x + boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y + boxDimension.y); 
	boxCoord.z = (boxCenterBoxCoord.z + boxDimension.z);
	MatrixMult(&objCoord, &orientMatrix, &boxCoord);
	v[7] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	HR(renderitems->OBBVertexBuffer->Unlock(), L"initConvexHullForRendering: Failed to unlock vertex buffer: ");

	// Obtain a pointer to a new index buffer.
	HR(g_app->m_pD3DDev->CreateIndexBuffer(renderitems->OBBIndexBufferSize*sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &renderitems->OBBIndexBuffer, 0),
		L"initConvexHullForRendering: CreateIndexBuffer failed: ");
	
	// Now lock it to obtain a pointer to its internal data, and write the grid's index data.
	WORD* k = 0;
	HR(renderitems->OBBIndexBuffer->Lock(0, 0, (void**)&k, 0), L"initConvexHullForRendering: Failed to lock index buffer: ");
	k[0] = 2;  k[1] = 1;  k[2] = 3;		// face 1 triange 1
	k[3] = 2;  k[4] = 0;  k[5] = 1;		// face 1 triange 2
	k[6] = 0;  k[7] = 4;  k[8] = 5;		// face 2 triange 3
	k[9] = 0;  k[10] = 5; k[11] = 1;	// face 2 triange 4
	k[12] = 1; k[13] = 5; k[14] = 7;	// face 3 triange 5
	k[15] = 1; k[16] = 7; k[17] = 3;	// face 3 triange 6
	k[18] = 3; k[19] = 7; k[20] = 6;	// face 4 triange 7
	k[21] = 3; k[22] = 6; k[23] = 2;	// face 4 triange 8
	k[24] = 2; k[25] = 6; k[26] = 4;	// face 5 triange 9
	k[27] = 2; k[28] = 4; k[29] = 0;	// face 5 triange 10
	k[30] = 4; k[31] = 5; k[32] = 7;	// face 6 triange 11
	k[33] = 4; k[34] = 7; k[35] = 6;	// face 6 triange 12
	k[36] = 3; k[37] = 1; k[38] = 2;	// face 1 triange 1		- opposite side
	k[39] = 1; k[40] = 0; k[41] = 2;	// face 1 triange 2		- opposite side
	k[42] = 5; k[43] = 4; k[44] = 0;	// face 2 triange 3		- opposite side
	k[45] = 1; k[46] = 5; k[47] = 0;	// face 2 triange 4		- opposite side
	k[48] = 7; k[49] = 5; k[50] = 1;	// face 3 triange 5		- opposite side
	k[51] = 3; k[52] = 7; k[53] = 1;	// face 3 triange 6		- opposite side
	k[54] = 6; k[55] = 7; k[56] = 3;	// face 4 triange 7		- opposite side
	k[57] = 2; k[58] = 6; k[59] = 3;	// face 4 triange 8		- opposite side
	k[60] = 4; k[61] = 6; k[62] = 2;	// face 5 triange 9		- opposite side
	k[63] = 0; k[64] = 4; k[65] = 2;	// face 5 triange 10	- opposite side
	k[66] = 7; k[67] = 5; k[68] = 4;	// face 6 triange 11	- opposite side
	k[69] = 6; k[70] = 7; k[71] = 4;	// face 6 triange 12	- opposite side
	HR(renderitems->OBBIndexBuffer->Unlock(), L"initConvexHullForRendering: Failed to unlock index buffer: ");
}
/************************************************************************/
/* Name:		initOBBForRenderingLines								*/
/* Description: Sets up D3D Mesh to render the OBB Wireframe			*/
/************************************************************************/
void obbox::initOBBForRenderingLines()
{
	renderitems->OBBVertexBufferSize = 8; renderitems->OBBIndexBufferSize = 24; // 8 verticies in a box, (both sides of each triangle) 24 indicies to describe 12 edges (both sides)

	// Obtain a pointer to a new vertex buffer.
	HR(g_app->m_pD3DDev->CreateVertexBuffer(renderitems->OBBVertexBufferSize * sizeof(VertexPos), 
		D3DUSAGE_WRITEONLY,	0, D3DPOOL_MANAGED, &renderitems->OBBVertexBuffer, 0),
		L"initOBBForRenderingLines: CreateVertexBuffer failed: ");

	// Now lock it to obtain a pointer to its internal data, and write the grid's vertex data.
	VertexPos* v = 0;
	HR(renderitems->OBBVertexBuffer->Lock(0, 0, (void**)&v, 0), L"initOBBForRenderingLines: Failed to lock vertex buffer: ");

	// WORK OUT BOX CORNERS FOR VERTEX BUFFER IN OBJECT COORDINATES
	double3 boxCoord; double3 objCoord; // Some temporary variables
	boxCoord.x = (boxCenterBoxCoord.x - boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y - boxDimension.y); 
	boxCoord.z = (boxCenterBoxCoord.z - boxDimension.z);
	MatrixMult(&objCoord, &orientMatrix, &boxCoord); // Translate box corner to object coordinates
	v[0] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	boxCoord.x = (boxCenterBoxCoord.x + boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y - boxDimension.y); 
	boxCoord.z = (boxCenterBoxCoord.z - boxDimension.z);
	MatrixMult(&objCoord, &orientMatrix, &boxCoord);
	v[1] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	boxCoord.x = (boxCenterBoxCoord.x - boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y - boxDimension.y);
	boxCoord.z = (boxCenterBoxCoord.z + boxDimension.z);	
	MatrixMult(&objCoord, &orientMatrix, &boxCoord);
	v[2] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	boxCoord.x = (boxCenterBoxCoord.x + boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y - boxDimension.y); 
	boxCoord.z = (boxCenterBoxCoord.z + boxDimension.z);
	MatrixMult(&objCoord, &orientMatrix, &boxCoord);
	v[3] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	boxCoord.x = (boxCenterBoxCoord.x - boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y + boxDimension.y); 
	boxCoord.z = (boxCenterBoxCoord.z - boxDimension.z);
	MatrixMult(&objCoord, &orientMatrix, &boxCoord);
	v[4] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	boxCoord.x = (boxCenterBoxCoord.x + boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y + boxDimension.y); 
	boxCoord.z = (boxCenterBoxCoord.z - boxDimension.z);
	MatrixMult(&objCoord, &orientMatrix, &boxCoord);
	v[5] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	boxCoord.x = (boxCenterBoxCoord.x - boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y + boxDimension.y); 
	boxCoord.z = (boxCenterBoxCoord.z + boxDimension.z);
	MatrixMult(&objCoord, &orientMatrix, &boxCoord);
	v[6] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	boxCoord.x = (boxCenterBoxCoord.x + boxDimension.x); 
	boxCoord.y = (boxCenterBoxCoord.y + boxDimension.y); 
	boxCoord.z = (boxCenterBoxCoord.z + boxDimension.z);
	MatrixMult(&objCoord, &orientMatrix, &boxCoord);
	v[7] = D3DXVECTOR3((float)objCoord.x, (float)objCoord.y, (float)objCoord.z);

	HR(renderitems->OBBVertexBuffer->Unlock(), L"initOBBForRenderingLines: Failed to unlock vertex buffer: ");

	// Obtain a pointer to a new index buffer.
	HR(g_app->m_pD3DDev->CreateIndexBuffer(renderitems->OBBIndexBufferSize*sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &renderitems->OBBIndexBuffer, 0),
		L"initOBBForRenderingLines: CreateIndexBuffer failed: ");
	
	// Now lock it to obtain a pointer to its internal data, and write the grid's index data.
	WORD* k = 0;
	HR(renderitems->OBBIndexBuffer->Lock(0, 0, (void**)&k, 0), L"initOBBForRenderingLines: Failed to lock index buffer: ");
	k[0]  = 0; k[1]  = 1; // Line 1
	k[2]  = 1; k[3]  = 5; // Line 2
	k[4]  = 5; k[5]  = 4; // Line 3
	k[6]  = 4; k[7]  = 0; // Line 4
	k[8]  = 0; k[9]  = 2; // Line 5
	k[10] = 1; k[11] = 3; // Line 6
	k[12] = 5; k[13] = 7; // Line 7
	k[14] = 4; k[15] = 6; // Line 8
	k[16] = 2; k[17] = 3; // Line 9
	k[18] = 3; k[19] = 7; // Line 10
	k[20] = 7; k[21] = 6; // Line 11
	k[22] = 2; k[23] = 6; // Line 12

	HR(renderitems->OBBIndexBuffer->Unlock(), L"initOBBForRenderingLines: Failed to unlock index buffer: ");
}
/************************************************************************/
/* Name:		CheckForDuplicateVertex									*/
/* Description: O(n^2) function just for debugging.  Checks that there  */
/*		aren't duplicate verticies in the index array.					*/
/************************************************************************/
bool CheckForDuplicateVertex(int numIndicies, int * indicies, D3DXVECTOR3 * verticies)
{
	for(int i = 0 ; i < (numIndicies-1); i ++)
	{
		for(int j = i+1; j < numIndicies; j++)
		{
			if(verticies[indicies[i]] == verticies[indicies[j]])
				return true;
		}
	}
	return false;
}
/************************************************************************/
/* Name:		DrawConvexHull											*/
/* Description: Draw the convex hull layers								*/
/************************************************************************/
void obbox::DrawConvexHull(D3DXMATRIX * mat)
{
	if(depth+1 > g_app->GetObbRenderDepth())
		return;
	if(renderitems && renderitems->convexHullVertexBuffer ) // If the vertex buffer exists
		g_renderer->DrawWireframe(renderitems->convexHullVertexBuffer, renderitems->convexHullVertexBufferSize, 
								  renderitems->convexHullIndexBuffer, renderitems->convexHullIndexBufferSize, mat);
	else
		throw std::runtime_error("obbox::DrawConvexHull(): Error - convexHullVertexBuffer not initialized");
}
/************************************************************************/
/* Name:		DrawOBB													*/
/* Description: Draw the convex hull layers								*/
/************************************************************************/
void obbox::DrawOBB(D3DXMATRIX * mat)
{
	if(depth+1 > g_app->GetObbRenderDepth())
		return;
	if(renderitems && renderitems->OBBVertexBuffer) // If the vertex buffer exists
	{
		if(DRAW_OBB_AS_LINES)
			g_renderer->DrawWireframeLines(renderitems->OBBVertexBuffer, renderitems->OBBVertexBufferSize, 
										   renderitems->OBBIndexBuffer, renderitems->OBBIndexBufferSize, mat);
		else
			g_renderer->DrawWireframe(renderitems->OBBVertexBuffer, renderitems->OBBVertexBufferSize, 
									  renderitems->OBBIndexBuffer, renderitems->OBBIndexBufferSize, mat);
	}
	else
		throw std::runtime_error("obbox::DrawConvexHull(): Error - OBBVertexBuffer not initialized");
}
/************************************************************************/
/* Name:		OrderAxes												*/
/* Description: Quick and dirty sorting of axes based on their box      */
/*			dimension lengths.											*/
/************************************************************************/
void obbox::OrderAxes(double3 * boxDimension)
{
	// Lazy way to sort box axes (O(n^2)), but only 3 values --> No point doing faster sort! Results will be:
	// double3	t->axisOrder[3];  // [Largest, Middle, Smallest]			--> NOW USE SCRAP PAD (in obbtree)
	// double	t->splitPoint[3];											--> NOW USE SCRAP PAD (in obbtree)
	if(boxDimension->x > boxDimension->y && boxDimension->x > boxDimension->z) // x largest
	{
		t->axisOrder[0] = t->eigVec[0]; t->splitPoint[0] = t->mean0;
		if(boxDimension->y > boxDimension->z) // y second largest
		{
			t->axisOrder[1] = t->eigVec[1]; t->splitPoint[1] = t->mean1; t->axisOrder[2] = t->eigVec[2]; t->splitPoint[2] = t->mean2;
		}
		else // z second largest
		{
			t->axisOrder[1] = t->eigVec[2]; t->splitPoint[1] = t->mean2; t->axisOrder[2] = t->eigVec[1]; t->splitPoint[2] = t->mean1;
		}
	}
	else if(boxDimension->y > boxDimension->z) // y largest
	{
		t->axisOrder[0] = t->eigVec[1];  t->splitPoint[0] = t->mean1;
		if(boxDimension->x > boxDimension->z) // x second largest
		{
			t->axisOrder[1] = t->eigVec[0]; t->splitPoint[1] = t->mean0; t->axisOrder[2] = t->eigVec[2]; t->splitPoint[2] = t->mean2;
		}
		else // z second largest
		{
			t->axisOrder[1] = t->eigVec[2]; t->splitPoint[1] = t->mean2;t->axisOrder[2] = t->eigVec[0]; t->splitPoint[2] = t->mean0;
		}
	} else // z largest
	{
		t->axisOrder[0] = t->eigVec[2]; t->splitPoint[0] = t->mean2;
		if(boxDimension->x > boxDimension->y) // x second largest
		{
			t->axisOrder[1] = t->eigVec[0]; t->splitPoint[1] = t->mean0; t->axisOrder[2] = t->eigVec[1]; t->splitPoint[2] = t->mean1;
		}
		else // y second largest
		{
			t->axisOrder[1] = t->eigVec[1]; t->splitPoint[1] = t->mean1; t->axisOrder[2] = t->eigVec[0]; t->splitPoint[2] = t->mean0;
		}
	}
}
/************************************************************************/
/* Name:		TestOBBCollision										*/
/* Description: Test two OBB elements for collision using separating	*/
/*				axis test. Code borrowed and modified from page 104 of  */
/*				real time collision detection book.						*/
/************************************************************************/
int obbox::TestOBBCollision( obbox * a, obbox * b )
{
	// The following code is based on Christer Ericson's book, Real-time collision detection (pages 103-105)
    //http://books.google.com/books?id=WGpL6Sk9qNAC&lpg=PA101&ots=Pl4MjF4ciO&dq=Real-Time%20Collision%20Detection%20oriented%20bounding%20boxes&pg=PA101#v=onepage&q=Real-Time%20Collision%20Detection%20oriented%20bounding%20boxes&f=false
	// NOTE 1: RBO World matrix is defined as follows: WorldMatrix = M_scale * M_rot * M_translation
	//         OBBOX bounds need to be modified in the same way.  Also, M_rot needs to be M_rot^-1 = M_rot^t
    // NOTE 2: rbobject scale is only a scalar.  Using a vector (scaling in x, y, z directions separatly)
	//         would require updating scaling factor for all boxes --> expensive.
	// NOTE 3: the COLUMNS of orientMat are the axes of the OBB. ie, v1 = <x.x,y.x,z.x>, V2 = <x.y,y.y,z.y>, V2 = <x.z,y.z,z.z>

	if(a->isLeaf && b->isLeaf)
		return TestTriCollision(a, b);

	// There is some matlab matlab code to verfify Matrix and Vector multiplication functions --> See: "MatlabTests.m"

//#ifdef _DEBUG
//	if(obbtree::debug_check_count == 68)
//		int tempvar = 68;  // Put breakpoint here if you want to debug a particular collision step
//#endif

	// Compute rotation matrix expressing b in a's coordinate frame.
	double3x3 R, R_aOBB_world, R_bOBB_world;
	// Rotation composite from OBBOX to world coordinates
	// --> Both R_aOBB and R_world are [U,V,N] -> [X,Y,Z] transforms.  Therefore columns represent frame axes.
	MatrixMult_ATran_B(& R_aOBB_world, & a->rbo->R, & a->orientMatrix);  // Aug 23rd 2010 --> Correct!
	MatrixMult_ATran_B(& R_bOBB_world, & b->rbo->R, & b->orientMatrix);

	// Move OBB center from object coords into world coords
	double3 t_aOBB, t_bOBB;
	MatrixMult_ATran_B( & t_aOBB, & a->rbo->R, & (a->boxCenterObjectCoord * a->rbo->scale) ); //boxCenterObjectCoord needs to be scaled then rotated first
	MatrixMult_ATran_B( & t_bOBB, & b->rbo->R, & (b->boxCenterObjectCoord * b->rbo->scale) );

	// Add rbo position and OBB position in world coordinates
	double3 t_a_world, t_b_world;
	Add_D3DXVECTOR3_double3( & t_a_world, & a->rbo->x, & t_aOBB );
	Add_D3DXVECTOR3_double3( & t_b_world, & b->rbo->x, & t_bOBB );

	// From here on is CHRISTER ERICSON'S ALGORITHM
	// Need A^Transpose * B, The following function does this in one step (saves compuation)
	MatrixMult_ATran_B( & R, & R_aOBB_world, & R_bOBB_world );

	// Calculate displacement vector in world coordinates
	double3 t_world = t_b_world - t_a_world; // Vector from a to b

	// Bring translation vector into OBpB A's coordinates
	double3 t;
	MatrixMult_ATran_B( & t, & R_aOBB_world, & t_world );

	// Scale box dimensions
	double a_OBBDim[3];
	for(int i = 0; i < 3; i ++) {a_OBBDim[i] = a->boxDimension[i] * a->rbo->scale; }
	double b_OBBDim[3];
	for(int i = 0; i < 3; i ++) {b_OBBDim[i] = b->boxDimension[i] * b->rbo->scale; }

	// Compute common subexpressions. Add in an epsilon term to counteract arithmetic 
	// errors when two edges are parallel and their cross product is (near) null.
	double3x3 AbsR;
	for (int i = 0; i < 3; i ++ )
		for (int j = 0; j < 3; j ++ )
			AbsR[i][j] = fabs(R[i][j]) + 0.000000000000000001;

	double ra, rb; // temporary values.
	// Test axes L = A0, L = A1, L = A2
	for (int i = 0; i < 3; i ++ )
	{
		ra = a_OBBDim[i];
		rb = (b_OBBDim[0] * AbsR[i][0]) + (b_OBBDim[1] * AbsR[i][1]) + (b_OBBDim[2] * AbsR[i][2]);
		if (fabs(t[i]) > (ra + rb)) return 0;
	}

	// Test axes L = B0, L = B1, L = B2
	for (int i = 0; i < 3; i ++ )
	{
		ra = (a_OBBDim[0] * AbsR[0][i]) + (a_OBBDim[1] * AbsR[1][i]) + (a_OBBDim[2] * AbsR[2][i]);
		rb = b_OBBDim[i];
		if (fabs(t[0]*R[0][i] + t[1]*R[1][i] + t[2]*R[2][i]) > (ra + rb)) return 0;
	}

	// NOTE: MANY ALGORITHMS STOP HERE SINCE THIS COVERS ~85% OF CASES (see pg 106 of real time collision detection book)

	// Test axis L = A0 x B0
	ra = a_OBBDim[1] * AbsR[2][0] + a_OBBDim[2] * AbsR[1][0];
	rb = b_OBBDim[1] * AbsR[0][2] + b_OBBDim[2] * AbsR[0][1];
	if (fabs(t[2] * R[1][0] - t[1] * R[2][0]) > (ra + rb)) return 0;

	// Test axis L = A0 x B1
	ra = a_OBBDim[1] * AbsR[2][1] + a_OBBDim[2] * AbsR[1][1];
	rb = b_OBBDim[0] * AbsR[0][2] + b_OBBDim[2] * AbsR[0][0];
	if (fabs(t[2] * R[1][1] - t[1] * R[2][1]) > (ra + rb)) return 0;

	// Test axis L = A0 x B2
	ra = a_OBBDim[1] * AbsR[2][2] + a_OBBDim[2] * AbsR[1][2];
	rb = b_OBBDim[0] * AbsR[0][1] + b_OBBDim[1] * AbsR[0][0];
	if (fabs(t[2] * R[1][2] - t[1] * R[2][2]) > (ra + rb)) return 0;

	// Test axis L = A1 x B0
	ra = a_OBBDim[0] * AbsR[2][0] + a_OBBDim[2] * AbsR[0][0];
	rb = b_OBBDim[1] * AbsR[1][2] + b_OBBDim[2] * AbsR[1][1];
	if (fabs(t[0] * R[2][0] - t[2] * R[0][0]) > (ra + rb)) return 0;

	// Test axis L = A1 x B1
	ra = a_OBBDim[0] * AbsR[2][1] + a_OBBDim[2] * AbsR[0][1];
	rb = b_OBBDim[0] * AbsR[1][2] + b_OBBDim[2] * AbsR[1][0];
	if (fabs(t[0] * R[2][1] - t[2] * R[0][1]) > (ra + rb)) return 0;

	// Test axis L = A1 x B2
	ra = a_OBBDim[0] * AbsR[2][2] + a_OBBDim[2] * AbsR[0][2];
	rb = b_OBBDim[0] * AbsR[1][1] + b_OBBDim[1] * AbsR[1][0];
	if (fabs(t[0] * R[2][2] - t[2] * R[0][2]) > (ra + rb)) return 0;

	// Test axis L = A2 x B0
	ra = a_OBBDim[0] * AbsR[1][0] + a_OBBDim[1] * AbsR[0][0];
	rb = b_OBBDim[1] * AbsR[2][2] + b_OBBDim[2] * AbsR[2][1];
	if (fabs(t[1] * R[0][0] - t[0] * R[1][0]) > (ra + rb)) return 0;

	// Test axis L = A2 x B1
	ra = a_OBBDim[0] * AbsR[1][1] + a_OBBDim[1] * AbsR[0][1];
	rb = b_OBBDim[0] * AbsR[2][2] + b_OBBDim[2] * AbsR[2][0];
	if (fabs(t[1] * R[0][1] - t[0] * R[1][1]) > (ra + rb)) return 0;

	// Test axis L = A2 x B2
	ra = a_OBBDim[0] * AbsR[1][2] + a_OBBDim[1] * AbsR[0][2];
	rb = b_OBBDim[0] * AbsR[2][1] + b_OBBDim[1] * AbsR[2][0];
	if (fabs(t[1] * R[0][2] - t[0] * R[1][2]) > (ra + rb)) return 0;

	// Since no separating axis is found, the OBBs must be intersecting
	return 1;

}

/************************************************************************/
/* Name:		TestTriCollision										*/
/* Description: Test two OBB leaf nodes with triangle primatives for    */
/*				overlap.  Just a wrapper function for Tomas Moller's 	*/
/*				function.												*/
/************************************************************************/
int obbox::TestTriCollision( obbox * a, obbox * b )
{
#ifdef _DEBUG
	if(!a->isLeaf || !b->isLeaf)
		throw std::runtime_error("TestTriCollision: obbox are not leaf nodes!");
#endif

	float V0[3], V1[3], V2[3], U0[3], U1[3], U2[3], TEMP[3];

	for(int i = 0; i < a->numFaces; i ++) // O(n^2) loop, but OBB algorithm should make numFaces ~= 1 (or at least small)
	{
		for(int j = 0; j < b->numFaces; j ++)
		{
			// V0
			TEMP[0] = (float) a->rbo->vertexBuffer[a->t->indices[a->indices + i*3 + 0]].x;
			TEMP[1] = (float) a->rbo->vertexBuffer[a->t->indices[a->indices + i*3 + 0]].y;
			TEMP[2] = (float) a->rbo->vertexBuffer[a->t->indices[a->indices + i*3 + 0]].z;
			MatrixAffineMult_ATran_B( V0, & a->rbo->matWorld, TEMP);

			// V1
			TEMP[0] = (float) a->rbo->vertexBuffer[a->t->indices[a->indices + i*3 + 1]].x;
			TEMP[1] = (float) a->rbo->vertexBuffer[a->t->indices[a->indices + i*3 + 1]].y;
			TEMP[2] = (float) a->rbo->vertexBuffer[a->t->indices[a->indices + i*3 + 1]].z;
			MatrixAffineMult_ATran_B( V1, & a->rbo->matWorld, TEMP);

			// V2
			TEMP[0] = (float) a->rbo->vertexBuffer[a->t->indices[a->indices + i*3 + 2]].x;
			TEMP[1] = (float) a->rbo->vertexBuffer[a->t->indices[a->indices + i*3 + 2]].y;
			TEMP[2] = (float) a->rbo->vertexBuffer[a->t->indices[a->indices + i*3 + 2]].z;
			MatrixAffineMult_ATran_B( V2, & a->rbo->matWorld, TEMP);

			// U0
			TEMP[0] = (float) b->rbo->vertexBuffer[b->t->indices[b->indices + j*3 + 0]].x;
			TEMP[1] = (float) b->rbo->vertexBuffer[b->t->indices[b->indices + j*3 + 0]].y;
			TEMP[2] = (float) b->rbo->vertexBuffer[b->t->indices[b->indices + j*3 + 0]].z;
			MatrixAffineMult_ATran_B( U0, & b->rbo->matWorld, TEMP);

			// U1
			TEMP[0] = (float) b->rbo->vertexBuffer[b->t->indices[b->indices + j*3 + 1]].x;
			TEMP[1] = (float) b->rbo->vertexBuffer[b->t->indices[b->indices + j*3 + 1]].y;
			TEMP[2] = (float) b->rbo->vertexBuffer[b->t->indices[b->indices + j*3 + 1]].z;
			MatrixAffineMult_ATran_B( U1, & b->rbo->matWorld, TEMP);

			// U2
			TEMP[0] = (float) b->rbo->vertexBuffer[b->t->indices[b->indices + j*3 + 2]].x;
			TEMP[1] = (float) b->rbo->vertexBuffer[b->t->indices[b->indices + j*3 + 2]].y;
			TEMP[2] = (float) b->rbo->vertexBuffer[b->t->indices[b->indices + j*3 + 2]].z;
			MatrixAffineMult_ATran_B( U2, & b->rbo->matWorld, TEMP);

			// Just send verticies off to Tomas Moller's triangle-triangle intersect code.
			if(tri_tri_intersect(V0, V1, V2, U0, U1, U2))
				return 1;
		}
	}

	return 0;
}