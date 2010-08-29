// File:		CGAL.cpp
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

// This is wrapper header to use CGAL functions for computing 3D convex hull
// Preparing definitions and installing CGAL can be found at: 
// "OBBDetection_references_&_installs\Installs\cgal installation (JONNO).txt"

#include "main.h"

void Copy_Mesh_To_CH(obbox * curobbox, 
					 double * vertexSet, 
					 int vertexSetSize, 
					 double ** cHullVert, 
					 UINT * cHullVertCount, 
					 PUINT * cHullInd, 
					 UINT * cHullIndCount )
{

    // Create Space
    (* cHullVertCount) = (UINT) curobbox->rbo->numVerticies;
	
// EDIT -> SPACE HAS ALREADY BEEN ALLOCATED IN OBBTREE SCRATCH PAD! JUST OVERWRITE THE CURRENT VALUES
	// (*cHullVert) = new double[(*cHullVertCount)*3];	
	// if(* cHullVert) { delete [] (*cHullVert); }

	// Copy the verticies over
	for(UINT i = 0; i < (*cHullVertCount); i ++)
	{
		(*cHullVert)[i*3 + 0] = curobbox->rbo->vertexBuffer[i].x;
		(*cHullVert)[i*3 + 1] = curobbox->rbo->vertexBuffer[i].y;
		(*cHullVert)[i*3 + 2] = curobbox->rbo->vertexBuffer[i].z;
	}


	(*cHullIndCount) = (UINT) curobbox->numFaces*3;

// EDIT -> SPACE HAS ALREADY BEEN ALLOCATED IN OBBTREE SCRATCH PAD! JUST OVERWRITE THE CURRENT VALUES
	// if(* cHullInd) { delete [] (*cHullInd); }
	// (*cHullInd) = new UINT[curobbox->numFaces*3];

	for(UINT i = 0; i < (*cHullIndCount); i ++)
	{
		(*cHullInd)[i] = (UINT)curobbox->t->indices[curobbox->indices+i]; // Offset by starting index in t->indices array
	}
}

bool CGAL_chull(double * vertexSet, 
				int vertexSetSize, 
				double ** cHullVert, 
				UINT * cHullVertCount, 
				PUINT * cHullInd, 
				UINT * cHullIndCount )
{
	// Copy vertex set to a vector
	std::vector<Point_3> points;
	points.reserve(vertexSetSize/3+1);
	for(int i = 0; i < vertexSetSize; i += 3) // 3 values per point (x,y,z)
	{
		points.push_back(Point_3(vertexSet[i],vertexSet[i+1],vertexSet[i+2]));
	}

	// define object to hold convex hull
	CGAL::Object ch_object;

	// compute convex hull
	CGAL::convex_hull_3(points.begin(), points.end(), ch_object);

	// determine what kind of object it is
	if (CGAL::object_cast<Segment_3>(&ch_object) )
	{
// THE FOLOWING SLOWS DOWN THE OUTPUT
//#ifdef _DEBUG
//		OutputDebugString(L"CGAL_chull:Convex hull is a segment\n");
//#endif
		// Convex hull is a segment
		return false;
	}
	else if (CGAL::object_cast<Polyhedron_3>(&ch_object) )
	{
		// Temporary variables
		Point_3					p1, p2, p3;
		Vector_3				normal;
		Facet_iterator			f;
		Halfedge_handle			current;
		Polyhedron_3			ch_Polyhedron_3;

		// Assign data from ch_object into Polyhedron_3
		CGAL::assign(ch_Polyhedron_3, ch_object);

		// Make space for return array
		(* cHullVertCount) = ch_Polyhedron_3.size_of_facets() * 3;	// Three verticies per face
		(* cHullIndCount) = (* cHullVertCount);

		// Max space for Ind and Vert buffers is already allocated.  Just overwrite it.
		int retIndex = 0; 
		for( f = ch_Polyhedron_3.facets_begin(); f != ch_Polyhedron_3.facets_end(); f++ )
		{
			// Get the three triangle points
			current = f->halfedge();
			p1 = current->vertex()->point();
			p2 = current->next()->vertex()->point();
			p3 = current->next()->next()->vertex()->point();

			// Store the triangles in return array
			(* cHullVert)[retIndex*3 + 0] = p1[0];		// p1.x
			(* cHullVert)[retIndex*3 + 1] = p1[1];		// p1.y
			(* cHullVert)[retIndex*3 + 2] = p1[2];		// p1.z
			(* cHullInd)[retIndex] = retIndex;			// p1 index
			retIndex += 1;
			(* cHullVert)[retIndex*3 + 0] = p2[0];		// p2.x
			(* cHullVert)[retIndex*3 + 1] = p2[1];		// p2.y
			(* cHullVert)[retIndex*3 + 2] = p2[2];		// p2.z
			(* cHullInd)[retIndex] = retIndex;			// p2 index
			retIndex += 1;
			(* cHullVert)[retIndex*3 + 0] = p3[0];		// p3.x
			(* cHullVert)[retIndex*3 + 1] = p3[1];		// p3.y
			(* cHullVert)[retIndex*3 + 2] = p3[2];		// p3.z
			(* cHullInd)[retIndex] = retIndex;			// p3 index
			retIndex += 1;
		}
	}
	else
	{
//#ifdef _DEBUG
//		OutputDebugString(L"CGAL_chull: convex_hull_3 failed\n");
//#endif
		return false;
	}

	return true;

}