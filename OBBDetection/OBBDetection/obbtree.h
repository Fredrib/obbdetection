// File:		obbtree.h
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

// This is header obbject to store OBB tree data:
// S. Gottschalk et. al. "OBBTree: A Hierarchical Structure for Rapid Iterference Detection"

#ifndef obbtree_h
#define obbtree_h

#include "main.h"
#include "hull.h"
#include "rbobjects.h"
#include "obbox.h"
#include "d3dformats.h"
#include "collision.h"

typedef pair< int, int >	pair_int;

class obbtree 
{
public:

					obbtree(DWORD numFaces, int numVerticies);
					~obbtree();

	friend class	rbobject; // This is lazy I know!
	friend class	rbmesh;
	friend class	obbox;
	friend class	debug_object;
	friend void		Copy_Mesh_To_CH(obbox * curobbox, double * vertexSet, int vertexSetSize, double ** cHullVert, UINT * cHullVertCount, PUINT * cHullInd, UINT * cHullIndCount );

	void			InitOBBTree(rbobject * rbo, LPCTSTR obbDataFilename );	// Recursive wrapper, to initialize root and start recursion
	void			PushBack(const obbox & item); // Add an element to the array by copying data in item. resize if necessary.
	void			Resize(void); // double array size for amortized cost of O(2)
	void			GetNext(obbox ** element); // Return a pointer to the next element and increase size by one. resize if necessary..
	void			GetNext(int * element); // Return an index to the next element and increase size by one. resize if necessary.
	obbox *			GetAt(int curIndex); // Get a pointer to the obbox at the desired index
	obbox *			GetObbToRender(); // Get a pointer to the obbox we're going to render

	// Collision Tests and support functions
	static void		TestOBBTreeCollision( rbobject * a, rbobject * b);
	static void		TestOBBTreeCollisionDebug(rbobject * a, rbobject * b); // Puts root node on stack and enters debug mode
	static void		TestOBBTreeCollisionDebug(); // Tests remaining pairs in obbtree::obbox_pairs in debug mode

	// Temporary space to use in collision tests
	static vector< pair_int > obbox_pairs;
	static int      debug_check_count; // How many collision tests we've done. Helps in debugging.

	void			MoveUp();
	void			MoveLeft();
	void			MoveRight();
 
	// File IO of OBB Tree
	void			writeObbToDisk( LPCTSTR Filename );
	bool			readObbFromDisk( rbobject * rbo, LPCTSTR Filename );

private:
	// Now have large contiguous memory arrays to store tree and indicies for each node.
	// --> The whole tree was dynamic which was VERY slow to generate and load from disk.
	obbox *		tree; // 2n - 1 cells in the tree --> This is a vector that can increase in size.
	int *		indices; // 3 indicies per face. Now use one global pool rather than dynamic memory for every node.
	int *		indices_child1; // Temporary memory space to use when splitting up indicies between children
	int *		indices_child2;
	int			size;
	int			capacity;
	int			numIndices;
	int			curOBBRender; // Current obbox and CH to render
	bool		scaleSet; // A flag to make sure the obbox scale is only set once.

	// Scratch variables for use in building obbox elements
	// --> Avoids multiple allocations on stack since obbox::BuildOBBTree is called many times.
	// Some speed up seen in practice (but not too much)
	double		A_H, A_i;
	double3		c_H, c_i;
	double3x3	Cov;
	double3		eigVec[3];
	double		minVec0, maxVec0, minVec1, maxVec1, minVec2, maxVec2;
	double3		curVertex; 
	double		projection, mean0, mean1, mean2;
	double3		axisOrder[3];
	double		splitPoint[3];
	int			child1NumIndicies, child2NumIndicies;
	bool		splitOK;
	int *		uniqueIndexSet; // Temporary space to use when removing repeated indicies
	int			uniqueIndexSetSize;
	double *	vertexSet; // Temporary space to hold input to convex hull generator
	int			vertexSetSize;

	double *	cHullVert; // Temporary memory space to hold convex hull
	UINT		cHullVertCount;
	PUINT		cHullInd; // Temporary memory space to hold convex hull indicies
	UINT		cHullIndCount;
};

#endif