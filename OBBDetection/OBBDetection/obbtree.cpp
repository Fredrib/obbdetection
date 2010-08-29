// File:		obbtree.cpp
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

// This is header obbject to store OBB tree data:
// S. Gottschalk et. al. "OBBTree: A Hierarchical Structure for Rapid Iterference Detection"

#include "main.h"
#include "obbtree.h"

obbtree::obbtree(DWORD numFaces, int numVerticies)
{
	tree = new obbox[numFaces*2]; // 2n - 1 cells in the tree --> This is a vector that can increase in size.
	// this->indicies is a scratch pad we will use to shift indicies around amongst sub obbox nodes temporarily
	indices = new int[numFaces*3]; // 3 indicies per face
    indices_child1 = new int[numFaces*3]; // Temporary memory space to use when splitting up indicies between children
	indices_child2 = new int[numFaces*3];
	uniqueIndexSet = new int[numFaces*3]; // Temporary memory space to use when removing repeated indicies
	vertexSet = new double[numVerticies*3]; // Temporary memory space to hold input to convex hull generator
	cHullVert = new double[numVerticies*3]; // Temporary memory space to hold convex hull
	cHullInd = new UINT[numVerticies]; // Temporary memory space to hold convex hull indicies
	uniqueIndexSetSize = 0;
	vertexSetSize = 0;
	cHullVertCount = 0;
	cHullIndCount = 0;
	size = 0;
	capacity = numFaces*2;
	numIndices = numFaces*3;
	curOBBRender = 0;
	scaleSet = 0;
}
obbtree::~obbtree()
{
	if(tree) { delete [] tree; tree = NULL; }
	if(indices) { delete [] indices; indices = NULL; }
	if(indices_child1) { delete [] indices_child1; indices_child1 = NULL; }
	if(indices_child2) { delete [] indices_child2; indices_child2 = NULL; }
	if(uniqueIndexSet) { delete [] uniqueIndexSet; uniqueIndexSet = NULL; }
	if(vertexSet) { delete [] vertexSet; vertexSet = NULL; }
	if(cHullVert) { delete [] cHullVert; cHullVert = NULL; }
	if(cHullInd) { delete [] cHullInd; cHullInd = NULL; }
}
/************************************************************************/
/* Name:		Resize													*/
/* Description: double the size of the vertex array (amortized O(2))	*/
/************************************************************************/
void obbtree::Resize(void)
{
#ifdef _DEBUG
	wchar_t OutputText[128]; swprintf(OutputText,127,L"Resizing obbtree to %d\n",2*capacity);
	OutputDebugString(OutputText);	
	__asm int 3; // debug breakpoint
#endif

	// double vector size for amortized time of O(2)
	obbox * newtree = new obbox[capacity*2];
	// Copy old tree data into new structure
	for(int curIndex = 0; curIndex < size; curIndex ++)
	{
		newtree[curIndex] = tree[curIndex];
	}
	// Delete the old allocated memory block and swap with new one
	delete [] tree;
	tree = newtree;
	capacity = capacity*2;
}
/************************************************************************/
/* Name:		PushBack												*/
/* Description: Add to the back of the vector, resize if necessary		*/
/************************************************************************/
void obbtree::PushBack(const obbox & item)
{
	while(size >= capacity)
		this->Resize();
	tree[size] = item;
	size = size + 1;
}
/************************************************************************/
/* Name:		GetNext													*/
/* Description: Add an element to the back of the array and return its  */
/*		address or index, resize if necessary.							*/
/************************************************************************/
void obbtree::GetNext(obbox ** element )
{
	while(size >= capacity)
		this->Resize();
	*element = &tree[size]; // Element after the last
	size = size + 1; // Increase size now
}
void obbtree::GetNext(int * element)
{
	while(size >= capacity)
		this->Resize();
	*element = (size); // Element after the last
	size = size + 1; // Increase size now
}
/************************************************************************/
/* Name:		GetAt													*/
/* Description: Just dereference the array								*/
/************************************************************************/
obbox * obbtree::GetAt(int curIndex)
{
	if(curIndex > (size-1) || curIndex < 0)
		throw std::runtime_error("GetBox:: curIndex is out of bounds");
	return &tree[curIndex];
}
/************************************************************************/
/* Name:		GetObbToRender											*/
/* Description: Just dereference the array and gets the current obb we	*/
/*              want to render											*/
/************************************************************************/
obbox * obbtree::GetObbToRender()
{
	return this->GetAt(this->curOBBRender);
}
/************************************************************************/
/* Name:		MoveUp / MoveLeft / MoveRight							*/
/* Description: Functions used to traverse the curOBBRender pointer		*/
/*				along the OBBTree.										*/
/************************************************************************/
void obbtree::MoveUp()
{
	obbox * currObb = GetObbToRender();
	if(currObb->parent != -1)
		curOBBRender = currObb->parent;
}
void obbtree::MoveLeft()
{
	obbox * currObb = GetObbToRender();
	if(!currObb->isLeaf && (currObb->depth+1) < (g_app->GetObbRenderDepth()))
		curOBBRender = currObb->childNode1;
}
void obbtree::MoveRight()
{
	obbox * currObb = GetObbToRender();
	if(!currObb->isLeaf && (currObb->depth+1) < (g_app->GetObbRenderDepth()))
		curOBBRender = currObb->childNode2;
	else
		int temp = 1;
}
/************************************************************************/
/* Name:		InitOBBTree												*/
/* Description: Create OBB Tree from vertex and mesh data in rbobject	*/
/************************************************************************/
void obbtree::InitOBBTree(rbobject * rbo, LPCTSTR Filename )
{	
	if(!g_app->CheckRebuildObb())
	{
		if(readObbFromDisk(rbo, Filename)) // Try reading the OBB from Disk if we're not forcing a rebuild
			return; // If we sucessfully read from disk then just return
	}
	
	// Otherwise build OBBTree from scratch...
	// Initalize root data structure & copy the indices to the root --> Could use memcpy, but this is safer
	obbox * curObbox;
	GetNext(&curObbox); // Get the next allocated block and increase size by 1

	curObbox->numFaces = rbo->numFaces;
	curObbox->indices = 0; // First index starts at zero 
	// this->indicies is a scratch pad we will use to shift indicies around amongst sub obbox nodes temporarily
	for(DWORD i = 0; i < ( rbo->numFaces * 3); i ++)
		this->indices[i] = rbo->indexBuffer[i];
	curObbox->parent = -1;								// root has no parent
	curObbox->depth = 0;								// depth starts at zero
	curObbox->isLeaf = false;							// Assume not true for now (could be single triangle though)
	curObbox->rbo = rbo;								// Pointer back to rbo (to match verticies with indicies)
	curObbox->t = this;									// Pointer back to tree header object
	
	// Start a flattened recursive procedure using a hand-built stack (LIFO)
	// --> overhead of recursive function calls was very high --> Stack overflow problems for very large models (>1M triangles).
	vector<int> obbStack;
	obbStack.reserve( 2*rbo->numFaces ); // # leaves is n, n=2^(h-1), h is height, h=log2(n)+1
	obbStack.push_back(0); // Add the root (index is always 0)

	int curObboxNum = 0;
	while(!obbStack.empty()) // Recursive procedure
	{
		// Include a progress check
		g_hud->RenderProgressText(curObboxNum+1,2*rbo->numFaces-1);
#ifdef _DEBUG
		wchar_t OutputText[128]; swprintf(OutputText,127,L"Processing OBBox %d of %d\n",curObboxNum+1,2*rbo->numFaces-1);
		OutputDebugString(OutputText);
#endif

		int curIndex = obbStack.back(); // Get the next index and remove it from the stack
		obbStack.pop_back();
		curObbox = this->GetAt(curIndex); // Get the obbox at the current index

		curObbox->BuildOBBTree(curIndex, &obbStack); // Build the current node --> Will add child nodes to the stack if necesary
		
		curObboxNum += 1;
	}

	// We're done with some scrap variables.  Delete them to save space for runtime.
	if(uniqueIndexSet) { delete [] uniqueIndexSet; uniqueIndexSet = NULL; }
	if(vertexSet) { delete [] vertexSet; vertexSet = NULL; }
	if(cHullVert) { delete [] cHullVert; cHullVert = NULL; }
	if(cHullInd) { delete [] cHullInd; cHullInd = NULL; }
	if(indices_child1) { delete [] indices_child1; indices_child1 = NULL; }
	if(indices_child2) { delete [] indices_child2; indices_child2 = NULL; }

	// Now save OBB tree to Disk at the specified filename
	writeObbToDisk(Filename);

}

/************************************************************************/
/* Name:		writeObbToDisk											*/
/* Description: Write OBB stream to disk								*/
/************************************************************************/
void obbtree::writeObbToDisk( LPCTSTR Filename )
{

	// Get the filename strings we need
	std::wstring obbFilename = Filename; obbFilename.append(L"_OBB.bin");
	std::wstring obbFilename_ind = Filename; obbFilename_ind.append(L"_OBB_index.bin");
	std::wstring obbFilename_CH_ind = Filename; obbFilename_CH_ind.append(L"_OBB_CH_ind.bin");
	std::wstring obbFilename_CH_vert = Filename; obbFilename_CH_vert.append(L"_OBB_CH_vert.bin");
	std::wstring obbFilename_RI = Filename; obbFilename_RI.append(L"_OBB_RI.bin");

	// See if the file exists
	ifstream fin(obbFilename.c_str(), ios::binary);					// 1. OBB Tree data (with obbox objects)
	if(fin.is_open() && !g_app->CheckRebuildObb()) // If we can open the file and we're not trying to rebuild then just return.
		return;
	fin.close();

	// THERE ARE 5 STREAMS PER OBJECT:
	ofstream fout(obbFilename.c_str(), ios::binary);					// 1. OBB Tree data (with obbox objects)
	ofstream fout_ind(obbFilename_ind.c_str(), ios::binary);			// 2. Index data (int *) that goes along with obbtree
	ofstream fout_CH_ind(obbFilename_CH_ind.c_str(), ios::binary);		// 3. Convex hull indices data (UINT *) that goes along with obbtree
	ofstream fout_CH_vert(obbFilename_CH_vert.c_str(), ios::binary);	// 4. Convex hull vertex data (double *) that goes along with obbtree
	ofstream fout_RI(obbFilename_RI.c_str(), ios::binary);				// 4. Convex hull render items data

	fout.seekp(0); fout_ind.seekp(0); // Go to the start of the files (default)
	fout_CH_ind.seekp(0); fout_CH_vert.seekp(0); fout_RI.seekp(0);

	fout.write((char *)(tree), capacity*sizeof(obbox)); // Save the whole array
	fout.write((char *)(&size), 1*sizeof(int)); // Also save the size of the array (not necessarily same as capacity)
	fout_ind.write((char *)(indices), numIndices*sizeof(int)); // 3 indicies per triangle (or face)

	// Now iterate through the tree array, writing dynamically allocated memory to disk
	// This is slow, but is only done for convex hull rendering (so not many blocks)
	for(int i = 0; i < size; i ++)
	{
		obbox * curObbox = GetAt(i);
		if(curObbox->renderitems)
		{
			fout_RI.write((char *)(curObbox->renderitems), sizeof(obbox_renderitems));
			if(curObbox->renderitems->cHullVert)
			{
				fout_CH_vert.write((char *)(curObbox->renderitems->cHullVert), (curObbox->renderitems->cHullVertCount * 3)*sizeof(double)); // 3 points per vertex
			}
			if(curObbox->renderitems->cHullInd)
			{
				fout_CH_ind.write((char *)(curObbox->renderitems->cHullInd), (curObbox->renderitems->cHullIndCount)*sizeof(UINT));
			}
		}
	}

	fout.close();
	fout_ind.close();	
	fout_CH_ind.close();
	fout_CH_vert.close();
	fout_RI.close();

}

/************************************************************************/
/* Name:		readObbFromDisk											*/
/* Description: Read OBB stream from disk								*/
/************************************************************************/
bool obbtree::readObbFromDisk( rbobject * rbo, LPCTSTR Filename )
{
	// Get the filename strings we need
	std::wstring obbFilename = Filename; obbFilename.append(L"_OBB.bin");
	std::wstring obbFilename_ind = Filename; obbFilename_ind.append(L"_OBB_index.bin");
	std::wstring obbFilename_CH_ind = Filename; obbFilename_CH_ind.append(L"_OBB_CH_ind.bin");
	std::wstring obbFilename_CH_vert = Filename; obbFilename_CH_vert.append(L"_OBB_CH_vert.bin");
	std::wstring obbFilename_RI = Filename; obbFilename_RI.append(L"_OBB_RI.bin");

	// THERE ARE 5 STREAMS: 
	ifstream fin(obbFilename.c_str(), ios::binary);					// 1. OBB Tree data (with obbox objects)
	if(!fin.is_open()) // If we can't open the file then just return
		return false;
	ifstream fin_ind(obbFilename_ind.c_str(), ios::binary);			// 2. Index data (int *) that goes along with obbtree
	if(!fin_ind.is_open())
		return false;
	ifstream fin_CH_ind(obbFilename_CH_ind.c_str(), ios::binary);	// 3. Convex hull indices data (UINT *) that goes along with obbtree
	if(!fin_CH_ind.is_open())
		return false;
	ifstream fin_CH_vert(obbFilename_CH_vert.c_str(), ios::binary);	// 4. Convex hull vertex data (double *) that goes along with obbtree
	if(!fin_CH_vert.is_open())
		return false;
	ifstream fin_RI(obbFilename_RI.c_str(), ios::binary);	// 4. Convex hull vertex data (double *) that goes along with obbtree
	if(!fin_RI.is_open())
		return false;
	fin.seekg(0); fin_ind.seekg(0); // Go to the start of the files (default)
	fin_CH_ind.seekg(0); fin_CH_vert.seekg(0); fin_RI.seekg(0);

	// GET THE TREE ARRAY:
	//		1. get filesize and check it makes sense
	struct stat results;
	stat(toNarrowString(obbFilename.c_str(),-1).c_str(), &results); // results.st_size is in bytes
	float num_obbox_elements = ((float)results.st_size-(float)sizeof(int))/(float)sizeof(obbox); // There is a single integer value at the end of the file

	//      2. Check that the number of elements is a whole number of obbox elements
	if(fabs(num_obbox_elements - (int)num_obbox_elements) >= 0.000001f)
		throw std::runtime_error("readObbFromDisk::Loaded _OBB file size is not a whole number of obbox elements.  Consider rebuilding obb tree");

	//		3. Read in the whole array at once (space has already been allocated)
	if((int)num_obbox_elements != capacity)
		throw std::runtime_error("readObbFromDisk::Loaded _OBB file is not the correct size.  Consider rebuilding obb tree");
	fin.read((char *)(tree),results.st_size-sizeof(int));

	//		4. Read in the size of the array (not necessaryily the capacity)
	fin.read((char *)(&size),sizeof(int));

	// GET THE INDEX ARRAY:
	//		1. get filesize and check it makes sense
	stat(toNarrowString(obbFilename_ind.c_str(),-1).c_str(), &results); // results.st_size is in bytes
	float num_index_elements = (float)results.st_size/(float)sizeof(int);

	//      2. Check that the number of elements is a whole number of int elements
	if(fabs(num_index_elements - (int)num_index_elements) >= 0.000001f)
		throw std::runtime_error("readObbFromDisk::Loaded _OBB_index file size is not a whole number of integer elements.  Consider rebuilding obb tree");

	//		3. Read in the whole array at once (space has already been allocated)
	if((int)num_index_elements != numIndices)
		throw std::runtime_error("readObbFromDisk::Loaded _OBB_index file is not the correct size.  Consider rebuilding obb tree");
	fin_ind.read((char *)(indices),results.st_size);

	// Now iterate through the tree array, reading dynamically allocated buffers from disk and fixing rbo pointer
	for(int i = 0; i < size; i ++)
	{
		obbox * curObbox = GetAt(i);
		curObbox->rbo = rbo;
		curObbox->t = rbo->OBBTree;
		if(curObbox->depth+1 <= g_app->GetObbRenderDepth())
		{
			if(curObbox->renderitems)
			{
				curObbox->renderitems = new obbox_renderitems;
				fin_RI.read((char *)(curObbox->renderitems), sizeof(obbox_renderitems));
				if(curObbox->renderitems->cHullVert)
				{
					curObbox->renderitems->cHullVert = new double[curObbox->renderitems->cHullVertCount *3];
					fin_CH_vert.read((char *)(curObbox->renderitems->cHullVert), (curObbox->renderitems->cHullVertCount*3)*sizeof(double)); // 3 indicies per triangle (or face)
				}

				if(curObbox->renderitems->cHullInd) // pointer is not null, but currently it is incorrect
				{
					curObbox->renderitems->cHullInd = new UINT[curObbox->renderitems->cHullIndCount];
					fin_CH_ind.read((char *)(curObbox->renderitems->cHullInd), (curObbox->renderitems->cHullIndCount)*sizeof(UINT)); // 3 indicies per triangle (or face)
				}
			}
			// Try initializing Convex Hull and OBB for rendering if obbox is within depth
			if(curObbox->depth+1 <= g_app->GetObbRenderDepth())
			{
				if(curObbox->renderitems) // If we have convex hull data and we want to render it
					curObbox->initConvexHullForRendering(); 
				else
					throw std::runtime_error("Convex hull data on disk doesn't cover desired render depth. Consider Re-building.");
				if(DRAW_OBB_AS_LINES)
					curObbox->initOBBForRenderingLines();
				else
					curObbox->initOBBForRendering();	
			}
		}
		else
		{
			curObbox->renderitems = NULL;
		}
	}

	fin.close();
	fin_ind.close();	
	fin_CH_ind.close();
	fin_CH_vert.close();	

	return true;
}
/************************************************************************/
/* Name:		TestOBBTreeCollision									*/
/* Description: Test two OBB trees for collision using separating		*/
/*				axis tests.	return number of collisions added to vector	*/
/************************************************************************/
void obbtree::TestOBBTreeCollision(rbobject * a, rbobject * b)
{
	if(!obbox_pairs.empty())
		throw std::runtime_error("TestOBBTreeCollision: obbox_pairs should be empty!");

	if(a == NULL || b == NULL)
		throw std::runtime_error("TestOBBTreeCollision: Input is null");

	// We will process obbox entries according to the rule: DESCEND LARGEST AREA
	// Note: DESCEND LARGEST LENGTH might work just as well (and not require 6 multiplications to work out).
	// The pairs to process will be thrown on a stack (LIFO) stored in obbox_pairs (static to obbtree class).

	// Put the roots on the stack.
	obbox_pairs.push_back(pair<int,int>(0,0));
	collision * curCollision = NULL;
	obbox * curNodeA = NULL;
	obbox * curNodeB = NULL;

	while(!obbox_pairs.empty())
	{
		pair_int curPair = obbox_pairs.back(); // Get the next index and remove it from the stack
		obbox_pairs.pop_back();
		curNodeA = a->OBBTree->GetAt(curPair.first);
		curNodeB = b->OBBTree->GetAt(curPair.second);

		// Test collision at the current level --> Either stop here or add more pairs to the stack.
		if(obbox::TestOBBCollision(curNodeA, curNodeB))
		{
			// If both are leaf nodes
			if(curNodeA->isLeaf && curNodeB->isLeaf)
			{
				// Add the nodes to the return collisions array.
				g_physics->GetNext_OBBCollisions(&curCollision);
				curCollision->obbox_a = curNodeA;
				curCollision->obbox_b = curNodeB;
			}
			// If only a is a leaf
			else if (curNodeA->isLeaf)
			{
				// Descend b's children
				obbox_pairs.push_back( pair_int( curPair.first, curNodeB->childNode1 ) );
				obbox_pairs.push_back( pair_int( curPair.first, curNodeB->childNode2 ) );
			}
			// If only b is a leaf
			else if (curNodeB->isLeaf)
			{
				// Descend a's children
				obbox_pairs.push_back( pair_int( curNodeA->childNode1 , curPair.second ) );
				obbox_pairs.push_back( pair_int( curNodeA->childNode2 , curPair.second ) );
			}
			// Descend either a or b
			else
			{
				// Descend the box of largest area --> Optimal stratergy for fewest iterations
				// Not necessarily fastest since 6 mults are needed.
				// TAKES INTO ACCOUNT CONSTANT SCALING FROM OBJECT TO WORLD COORDINATES.
				if(curNodeA->boxDimension.x*curNodeA->boxDimension.y*curNodeA->boxDimension.z*(curNodeA->rbo->scale*curNodeA->rbo->scale*curNodeA->rbo->scale) > 
				   curNodeB->boxDimension.x*curNodeB->boxDimension.y*curNodeB->boxDimension.z*(curNodeA->rbo->scale*curNodeB->rbo->scale*curNodeB->rbo->scale) )
				{
					obbox_pairs.push_back( pair_int( curNodeA->childNode1 , curPair.second ) );
					obbox_pairs.push_back( pair_int( curNodeA->childNode2 , curPair.second ) );
				}
				else
				{
					obbox_pairs.push_back( pair_int( curPair.first, curNodeB->childNode1 ) );
					obbox_pairs.push_back( pair_int( curPair.first, curNodeB->childNode2 ) );
				}
			}
		}
	}
}
/************************************************************************/
/* Name:		TestOBBTreeCollisionDebug								*/
/* Description: Test one obbox at a time, rendering each result.		*/
/************************************************************************/
void obbtree::TestOBBTreeCollisionDebug(rbobject * a, rbobject * b)
{
#ifdef _DEBUG
	obbtree::debug_check_count += 1;
#endif

	if(a == NULL || b == NULL)
		throw std::runtime_error("TestOBBTreeCollision: Input is null");
	a->UpdateMatricies(); // Update Rotation and Translation matricies (should have been done already, but O(1) anyway)
	b->UpdateMatricies(); // Update Rotation and Translation matricies (should have been done already, but O(1) anyway)

	g_app->m_EnterObbDebugMode = true;
	delete g_renderer->debug_objects; // Recursively clear the rest of the blue or red OBBes
	g_renderer->debug_objects = NULL;

	if(obbox_pairs.empty()) // First time we're running through --> Start with root
		// Put the roots on the stack.
		obbox_pairs.push_back(pair<int,int>(0,0));

	// We will process obbox entries according to the rule: DESCEND LARGEST AREA
	// Note: DESCEND LARGEST LENGTH might work just as well (and not require 6 multiplications to work out).
	// The pairs to process will be thrown on a stack (LIFO) stored in obbox_pairs (static to obbtree class).

	collision * curCollision = NULL;
	obbox * curNodeA = NULL;
	obbox * curNodeB = NULL;
	pair_int curPair = obbox_pairs.back(); // Get the next index and remove it from the stack
	obbox_pairs.pop_back();
	curNodeA = a->OBBTree->GetAt(curPair.first);
	curNodeB = b->OBBTree->GetAt(curPair.second);

	int colResult = obbox::TestOBBCollision(curNodeA, curNodeB);

	// Test collision at the current level --> Either stop here or add more pairs to the stack.
	if(colResult)
	{
		// If both are leaf nodes
		if(curNodeA->isLeaf && curNodeB->isLeaf)
		{
			debug_object::AddObboxTriDebugObject(curNodeA, RED); // Make a render object of the OBB leaf triangles
			debug_object::AddObboxTriDebugObject(curNodeB, RED);

			// Add the nodes to the return collisions array.
			g_physics->GetNext_OBBCollisions(&curCollision);
			curCollision->obbox_a = curNodeA;
			curCollision->obbox_b = curNodeB;
		}
		
		else
		{
			debug_object::AddObboxDebugObject(curNodeA, RED); // Make a render object of the OBB boxes
			debug_object::AddObboxDebugObject(curNodeB, RED);

			// If only a is a leaf
			if (curNodeA->isLeaf)
			{
				// Descend b's children
				obbox_pairs.push_back( pair_int( curPair.first, curNodeB->childNode1 ) );
				obbox_pairs.push_back( pair_int( curPair.first, curNodeB->childNode2 ) );
			}
			// If only b is a leaf
			else if (curNodeB->isLeaf)
			{
				// Descend a's children
				obbox_pairs.push_back( pair_int( curNodeA->childNode1 , curPair.second ) );
				obbox_pairs.push_back( pair_int( curNodeA->childNode2 , curPair.second ) );
			}
			// Descend either a or b
			else
			{
				// Descend the box of largest area --> Optimal stratergy for fewest iterations
				// Not necessarily fastest since 6 mults are needed.
				// TAKES INTO ACCOUNT CONSTANT SCALING FROM OBJECT TO WORLD COORDINATES.
				if(curNodeA->boxDimension.x*curNodeA->boxDimension.y*curNodeA->boxDimension.z*(curNodeA->rbo->scale*curNodeA->rbo->scale*curNodeA->rbo->scale) > 
					curNodeB->boxDimension.x*curNodeB->boxDimension.y*curNodeB->boxDimension.z*(curNodeA->rbo->scale*curNodeB->rbo->scale*curNodeB->rbo->scale) )
				{
					obbox_pairs.push_back( pair_int( curNodeA->childNode1 , curPair.second ) );
					obbox_pairs.push_back( pair_int( curNodeA->childNode2 , curPair.second ) );
				}
				else
				{
					obbox_pairs.push_back( pair_int( curPair.first, curNodeB->childNode1 ) );
					obbox_pairs.push_back( pair_int( curPair.first, curNodeB->childNode2 ) );
				}
			}
		}
	}
	else // Not in contact
	{
		if(curNodeA->isLeaf && curNodeB->isLeaf)
		{
			debug_object::AddObboxTriDebugObject(curNodeA, BLUE);
			debug_object::AddObboxTriDebugObject(curNodeB, BLUE);
		}
		else
		{
			debug_object::AddObboxDebugObject(curNodeA, BLUE);
			debug_object::AddObboxDebugObject(curNodeB, BLUE);
		}
	}
}
vector< pair_int > obbtree::obbox_pairs;
int obbtree::debug_check_count = 0;