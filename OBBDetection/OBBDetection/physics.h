/*************************************************************
**				OBBDetection Physics engine					**
**				Summer 2009									**
*************************************************************/
// File:		physics.h
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com
// Collision detection: Bounding box culling with fine detection for non-convex meshes.
// Collision reaction: Impulse based, frictionless collisions, with minimum impulse responce.
// ODE solver: Choice of Euler integrator or Runge Kutta 4th order integrator

#ifndef physics_h
#define physics_h

#include	"main.h"

// Now using "collision" class
//// Custom Data structures
//struct contact {
//	rbobject							* a,	// body containing vertex
//										* b;	// body containing face / vertex
//	D3DXVECTOR3							p,		// World space vertex location --> position of contact
//										n,		// Normal of face, or cross product of edges
//										ea,		// Edge direction for A
//										eb;		// Edge direction for B
//	bool								vf;		// Vertex face
//};

struct AABBOverlap {
	bool								xAxisOverlap,   // xAxisOverlaps
										yAxisOverlap,
										zAxisOverlap;
};
AABBOverlap *							GetOverlapStatus(AABBOverlap * AABBOverlapStatus, int a, int b); // O(1)
void									SetOverlapStatusXaxis(AABBOverlap * AABBOverlapStatus, int a, int b, bool setval);
void									SetOverlapStatusYaxis(AABBOverlap * AABBOverlapStatus, int a, int b, bool setval);
void									SetOverlapStatusZaxis(AABBOverlap * AABBOverlapStatus, int a, int b, bool setval);

class physics;

// FUNCTION POINTERS
typedef void (physics::*dydt_func)(float t, float y[], float ydot[]); // Function pointer which takes the ODE step
typedef float (physics::*select_func)(rbobject ** rb_list, int index); // Function pointer used in InsertionSortAABBs.  Grabs an rbobject from global list and grabs correct vertex info.
typedef float (*set_func)(AABBOverlap * AABBOverlapStatus, int a, int b, bool value); // Function pointer used in InsertionSortAABBs.  Sets the appropriate overlap status in AABBOverlap array.
#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

class physics
{
public:
										physics(void);				// Default Constructor
										~physics(void);				// Default Destructor

	// Rigid Body Object functions
	void								calcStep();					// Main entry point
	void								ode(float y0[], float yend[], int len, float t0, float t1, dydt_func dydt);
	void								dydt(float t, float y[], float ydot[]);
	void								ComputeForceAndTorque(float t, rbobject * rb);
	void								ddtStateToArray(rbobject * rb, float * ydot);
	void								InitPhysics(void);
	void								InterpolateFrame(float alphaTime);

	// Coarse Collision Detection
	void								InitAABBs(void); // Set up data structures and perform intial sort and sweep, likely O(n^2)
	void								CoarseCollisionDetection(void); // Perform per-step sort and sweep, probably O(n)
	void								InsertionSortAABBs( int * pArray, int arraySize, select_func MinSel );			 // Sort for coarse collision det.
	void								SweepAxisList( int * pArray, int arraySize, select_func MinSel, select_func MaxSel, set_func Set ); // Sweep for coarse collision det.
	inline float						xAxisMinSel(rbobject ** rb_list, int index) { return rb_list[index]->m_minAABB.x; }
	inline float						yAxisMinSel(rbobject ** rb_list, int index) { return rb_list[index]->m_minAABB.y; }
	inline float						zAxisMinSel(rbobject ** rb_list, int index) { return rb_list[index]->m_minAABB.z; }
	inline float						xAxisMaxSel(rbobject ** rb_list, int index) { return rb_list[index]->m_maxAABB.x; }
	inline float						yAxisMaxSel(rbobject ** rb_list, int index) { return rb_list[index]->m_maxAABB.y; }
	inline float						zAxisMaxSel(rbobject ** rb_list, int index) { return rb_list[index]->m_maxAABB.z; }
	
	// OBB Collisions Vector modifiers / selectors
	void								Resize_OBBCollisions(void);
	void								GetNext_OBBCollisions(collision ** element );
	void								GetNext_OBBCollisions(UINT * element);
	collision *							GetAt_OBBCollisions(UINT curIndex);
	bool								Empty_OBBCollisions(void);
	UINT								Size_OBBCollisions(void);
	
//	void						ODE(float y0, float y1, int num_RBO, float t0, float t1, dydt_func dydt);
	// These used to be private, but it's just easier this way
	float *								y;							// STATE: y[]
	float *								yfinal;						// STATE: yfinal[]
	float *								ydot;						// STATE: dydt[]
	float *								ygraphics;					// STATE: ygraphics[]
private:
	// Derivatives for RK4 integrator and a temporary variable
	float *								k1, * k2, * k3, * k4, * ytemp;	

	// Coarse Collision detection
	int	*								AABBXaxis;   // Sorted indices of g_rbobjects --> xaxis values
	int	*								AABBYaxis;
	int *								AABBZaxis;
	list<int>							AABBActiveList; // Linked list of active values
	AABBOverlap	*						AABBOverlapStatus; // Holds the overlap status for all rigid body object pairs --> Triangular array

	// Fine Collision Detection
	// A manually managed vector of OBB collisions: aim is to make it big enough to cover complex scene.  Will grow automatically if necessary.
	collision *							OBBCollisions; 
	UINT								size_OBBCollisions;
	UINT								capacity_OBBCollisions;
	rbobject *							ObbDebugObjectA; // Pointer to the objects currently being rendered in OBBDebugMode
	rbobject *							ObbDebugObjectB;
};

#endif