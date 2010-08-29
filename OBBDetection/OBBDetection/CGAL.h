// File:		CGAL.h
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

// This is wrapper header to use CGAL functions for computing 3D convex hull
// Preparing definitions and installing CGAL can be found at: 
// "OBBDetection_references_&_installs\Installs\cgal installation (JONNO).txt"

#ifndef CGAL_h
#define CGAL_h

#include "main.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/algorithm.h>
#include <CGAL/Convex_hull_traits_3.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/Polyhedron_3.h>
#include <vector>

// Some typedefs to make life easier
typedef CGAL::Exact_predicates_inexact_constructions_kernel		K;
typedef CGAL::Convex_hull_traits_3<K>							Traits;
typedef Traits::Polyhedron_3									Polyhedron_3;
typedef Traits::Vector_3										Vector_3;
typedef Polyhedron_3::Facet_iterator							Facet_iterator;
typedef K::Segment_3											Segment_3;
typedef K::Point_3												Point_3;
typedef Polyhedron_3::Halfedge_handle							Halfedge_handle;

// MY FUNCTION DEFINITIONS
bool CGAL_chull(double * vertexSet, int vertexSetSize, double ** cHullVert, UINT * cHullVertCount, PUINT * cHullInd, UINT * cHullIndCount );

void Copy_Mesh_To_CH(obbox * curobbox, double * vertexSet, int vertexSetSize, double ** cHullVert, UINT * cHullVertCount, PUINT * cHullInd, UINT * cHullIndCount );

#endif