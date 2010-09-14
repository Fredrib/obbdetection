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

#include	"main.h"
#include	"obbtree.h"

/************************************************************************/
/* Name:		physics													*/
/* Description:	Default Constructor: Set ODE solver variables			*/
/************************************************************************/
physics::physics(void)
{
	// Rigid body simulator (RK4)
	y = new float[STATESIZE * NO_RBOBJECTS];
	ydot = new float[STATESIZE * NO_RBOBJECTS];
	yfinal = new float[STATESIZE * NO_RBOBJECTS];
	ygraphics = new float[STATESIZE * NO_RBOBJECTS];
	k1 = new float[STATESIZE * NO_RBOBJECTS]; 
	k2 = new float[STATESIZE * NO_RBOBJECTS];
	k3 = new float[STATESIZE * NO_RBOBJECTS]; 
	k4 = new float[STATESIZE * NO_RBOBJECTS];
	ytemp = new float[STATESIZE * NO_RBOBJECTS];

	// Collision detection
	AABBXaxis = new int[NO_RBOBJECTS];
	AABBYaxis = new int[NO_RBOBJECTS];
	AABBZaxis = new int[NO_RBOBJECTS];

	// initialize AABB overlap array: Size is (NO_RBOBJECTS * NO_RBOBJECTS) --> 2n wasted!  But easier to index
	AABBOverlapStatus = new AABBOverlap[NO_RBOBJECTS * NO_RBOBJECTS];

	capacity_OBBCollisions = COLLISION_VECTOR_SIZE;
	size_OBBCollisions = 0;
	OBBCollisions = new collision[capacity_OBBCollisions];
}
/************************************************************************/
/* Name:		~physics												*/
/* Description:	Default Destructor										*/
/************************************************************************/
physics::~physics(void)
{
	if(y) { delete [] y; y = NULL; }
	if(ydot) { delete [] ydot; ydot = NULL; }
	if(yfinal) { delete [] yfinal; yfinal = NULL; }
	if(ygraphics) { delete [] ygraphics; ygraphics = NULL; }
	if(k1) { delete [] k1; k1 = NULL; }
	if(k2) { delete [] k2; k2 = NULL; }
	if(k3) { delete [] k3; k3 = NULL; }
	if(k4) { delete [] k4; k4 = NULL; }
	if(ytemp) { delete [] ytemp; ytemp = NULL; }
	if(AABBXaxis) { delete [] AABBXaxis; AABBXaxis = NULL; }
	if(AABBYaxis) { delete [] AABBYaxis; AABBYaxis = NULL; }
	if(AABBZaxis) { delete [] AABBZaxis; AABBZaxis = NULL; }
	if(AABBOverlapStatus) { delete [] AABBOverlapStatus; AABBOverlapStatus = NULL; }
	if(OBBCollisions) { delete [] OBBCollisions; OBBCollisions = NULL; }

}
/************************************************************************/
/* Name:		calcStep												*/
/* Description:	Main Entry point for physics solver						*/
/************************************************************************/
void physics::calcStep() // PHYSICS_STEP
{
	if(!g_app->m_pausePhysics && !g_app->m_EnterObbDebugMode )
	{
		// Make y = yfinal
		for(int i = 0; i < (STATESIZE * NO_RBOBJECTS); i++)
			y[i] = yfinal[i];

		do
		{
			// Take a physics step and store it in yfinal
			dydt_func dydt = &physics::dydt;
			ode(y, yfinal, STATESIZE * NO_RBOBJECTS, g_app->m_currentTimePhysics, 
				g_app->m_currentTimePhysics + PHYSICS_STEP, &physics::dydt);

			// Update rigid-body-objects for collision detection
			rbobject::StateToObjects(yfinal);

			// Do a coarse collision test --> THIS CAN BE FASTER. USE INSERTION SORT SWAPPING TO KEEP TRACK OF OVERLAPS (AS PER BARAFF & WITKIN)
			CoarseCollisionDetection();

			// Do a coarse collision test to fill up the collision array.  The sweep and prune is O(n).
			// --> But then it checks a big boolian array in O(n^2).  Consider another implimentation!
			int numOverlaps = 0;
			for(int i = 0; i < (NO_RBOBJECTS-1); i ++)
			{
				for(int j = i+1; j < (NO_RBOBJECTS); j ++)
				{
					AABBOverlap * curOverlap = GetOverlapStatus(AABBOverlapStatus, i, j);
					if(curOverlap->xAxisOverlap && curOverlap->yAxisOverlap && curOverlap->zAxisOverlap)
					{
						// Objects potentially overlap
						numOverlaps ++;

						if(!g_app->m_EnterObbDebugMode)
						{
							//  Get the collision pairs
							if(g_app->CheckObbDebugMode())
							{
								ObbDebugObjectA = g_rbobjects[i];
								ObbDebugObjectB = g_rbobjects[j];
								obbtree::TestOBBTreeCollisionDebug( ObbDebugObjectA, ObbDebugObjectB );
							}
							else
								obbtree::TestOBBTreeCollision( g_rbobjects[i], g_rbobjects[j]);
						}
					}
				}
			}

#ifdef _DEBUG
			if(!g_app->m_EnterObbDebugMode)
			{
				// Render obbox leaf collisions
				if(PAUSE_ON_OBBLEAF_COLLISION && !Empty_OBBCollisions())
				{
					g_app->m_pausePhysics = true;

/************************************/
/********** START TEMP CODE *********/
/************************************/

					collision * cur_collision = GetAt_OBBCollisions(0);
					obbtree::TestOBBTreeCollision( cur_collision->obbox_a->rbo, cur_collision->obbox_b->rbo );
					

/************************************/
/*********** END TEMP CODE **********/
/************************************/


				}
				if(RENDER_OBBLEAF_COLLISION_BOXES && !Empty_OBBCollisions())
				{
					// Render the boxes that are colliding.
					for(UINT i = 0; i <  Size_OBBCollisions(); i ++)
					{
						debug_object::AddObboxDebugObject(GetAt_OBBCollisions(i)->obbox_a, BLACK);
						debug_object::AddObboxDebugObject(GetAt_OBBCollisions(i)->obbox_b, BLACK);
					}
				}
				if(RENDER_OBBLEAF_COLLISION_TRIANGLES && !Empty_OBBCollisions())
				{
					// Render the triangles that are colliding.
					for(UINT i = 0; i <  Size_OBBCollisions(); i ++)
					{
						debug_object::AddObboxTriDebugObject(GetAt_OBBCollisions(i)->obbox_a, BLACK);
						debug_object::AddObboxTriDebugObject(GetAt_OBBCollisions(i)->obbox_b, BLACK);
					}
				}
			}
#endif
			// Recover from collisions
		} 
		while ( false /* While there are collisions to process */ );
	}

	if(g_app->m_EnterObbDebugMode)
	{
		if(obbtree::obbox_pairs.empty()) // If we're done cycling through the obb tree mark as done and take another step
		{
			g_app->m_EnterObbDebugMode = false; // Allow physics system to proceed as normal
			delete g_renderer->debug_objects; // Recursively delete the list
			g_renderer->debug_objects = NULL;
		}
		else
		{
			if(g_app->m_ObbDebugModeNext)
			{
				g_app->m_ObbDebugModeNext = false;	
				obbtree::TestOBBTreeCollisionDebug( ObbDebugObjectA, ObbDebugObjectB );
			}
		}
	}

	// DEBUG: rotate the source light
	g_light[0].rotateLightY(PHYSICS_STEP * 0.8f);
}
/************************************************************************/
/* Name:		InterpolateFrame										*/
/* Description:	Linearly interpolate graphics state between steps		*/
/************************************************************************/
void physics::InterpolateFrame(float alphaTime)
{

	for(int i = 0; i < (STATESIZE * NO_RBOBJECTS); i++)
	{
		if(!g_app->m_pausePhysics)
			ygraphics[i] = yfinal[i]*alphaTime + y[i]*(1.0f - alphaTime);
		else
			ygraphics[i] = yfinal[i];
	}
	rbobject::StateToObjects(ygraphics);
	for(int i = 0; i < (NO_RBOBJECTS); i++)
	{
		g_rbobjects[i]->dirtyScaleMatrix = true;
		g_rbobjects[i]->dirtyRotMatrix = true;
		g_rbobjects[i]->dirtyTransMatrix = true;
	}
}
/************************************************************************/
/* Name:		InitPhysics												*/
/* Description:	Initialization of array and any other bookkeeping		*/
/************************************************************************/
void physics::InitPhysics()
{
	rbobject::ObjectsToState(y);
	rbobject::ObjectsToState(yfinal);
	InitAABBs();
	
}
/************************************************************************/
/* Name:		GetOverlapStatus										*/
/* Description:	Get correct index in AABBOverlapStatus array. Makes		*/
/*				higher level code easier to read.						*/
/************************************************************************/
AABBOverlap * GetOverlapStatus(AABBOverlap * AABBOverlapStatus, int a, int b)
{
#ifdef _DEBUG
	if(a>=NO_RBOBJECTS || b>=NO_RBOBJECTS)
		throw std::runtime_error("GetOverlapStatus: object a or object b are out of bounds!");
	if(a == b)
		throw std::runtime_error("GetOverlapStatus: object a = object b!");
#endif
	if(a < b)
		return & AABBOverlapStatus[a * NO_RBOBJECTS + b];
	else
		return & AABBOverlapStatus[b * NO_RBOBJECTS + a];
}
void SetOverlapStatusXaxis(AABBOverlap * AABBOverlapStatus, int a, int b, bool setval)
{
#ifdef _DEBUG
	if(a>=NO_RBOBJECTS || b>=NO_RBOBJECTS)
		throw std::runtime_error("GetOverlapStatus: object a or object b are out of bounds!");
	if(a == b)
		throw std::runtime_error("GetOverlapStatus: object a = object b!");
#endif
	if(a < b)
		AABBOverlapStatus[a * NO_RBOBJECTS + b].xAxisOverlap = setval;
	else
		AABBOverlapStatus[b * NO_RBOBJECTS + a].xAxisOverlap = setval;
}
void SetOverlapStatusYaxis(AABBOverlap * AABBOverlapStatus, int a, int b, bool setval)
{
#ifdef _DEBUG
	if(a>=NO_RBOBJECTS || b>=NO_RBOBJECTS)
		throw std::runtime_error("GetOverlapStatus: object a or object b are out of bounds!");
	if(a == b)
		throw std::runtime_error("GetOverlapStatus: object a = object b!");
#endif
	if(a < b)
		AABBOverlapStatus[a * NO_RBOBJECTS + b].yAxisOverlap = setval;
	else
		AABBOverlapStatus[b * NO_RBOBJECTS + a].yAxisOverlap = setval;
}
void SetOverlapStatusZaxis(AABBOverlap * AABBOverlapStatus, int a, int b, bool setval)
{
#ifdef _DEBUG
	if(a>=NO_RBOBJECTS || b>=NO_RBOBJECTS)
		throw std::runtime_error("GetOverlapStatus: object a or object b are out of bounds!");
	if(a == b)
		throw std::runtime_error("GetOverlapStatus: object a = object b!");
#endif
	if(a < b)
		AABBOverlapStatus[a * NO_RBOBJECTS + b].zAxisOverlap = setval;
	else
		AABBOverlapStatus[b * NO_RBOBJECTS + a].zAxisOverlap = setval;
}
/************************************************************************/
/* Name:		InitAABBs												*/
/* Description:	Initialize axis aligned bounding boxes and perform first*/
/*				Sort and sweep, expected O(n^2).						*/
/************************************************************************/
void physics::InitAABBs(void)
{
	// Reset all object overlap statuses.  This array wastes space --> 2x larger than necessary.  But faster indexing this way.
	for(int i = 0; i < (NO_RBOBJECTS * NO_RBOBJECTS - 1); i++)
	{
		AABBOverlapStatus[i].xAxisOverlap = false;
		AABBOverlapStatus[i].yAxisOverlap = false;
		AABBOverlapStatus[i].zAxisOverlap = false;
	}

	// Initialize axis lists as just the RBOBJECTS (arbitrary order)
	for(int i = 0; i < (NO_RBOBJECTS); i++)
	{
		AABBX
			axis[i] = i;  AABBYaxis[i] = i;  AABBZaxis[i] = i;
	}

	// Perform a first time sort and sweep --> Expected slow due to insertion sort on unsorted list.
	CoarseCollisionDetection();
}
/************************************************************************/
/* Name:		InsertionSortAABBs										*/
/* Description:	Takes a list of rbobjects indices, and sorts them using*/
/*				the value derived from the selection function.			*/
/************************************************************************/
void physics::InsertionSortAABBs( int * pArray, int arraySize, select_func MinSel )
{
	if(arraySize<1 || pArray == NULL)
		throw std::runtime_error("InsertionSortAABBs: Array is empty, nothing to sort!");

	int rbobjectToSort; float curVal, valueToSort;
	for(int i=1; i<arraySize;i++) {  // Place the next value
		rbobjectToSort = pArray[i];
		valueToSort = CALL_MEMBER_FN(*this,MinSel)(g_rbobjects, rbobjectToSort);
		for(int j=0; j<=i;j++) {
			curVal = CALL_MEMBER_FN(*this,MinSel)(g_rbobjects, pArray[j]);
			if(curVal > valueToSort) {
				// push the other value forward to insert
				for(int m = i; m>j; m--) {
					pArray[m] = pArray[m-1];
				}
				pArray[j] = rbobjectToSort;
				break;
			}
		}
	}
}
/************************************************************************/
/* Name:		SweepAxisList											*/
/* Description:	Takes a list of rbobjects, performs a sweep determining */
/*				all overlaps, and sets the appropriate values in the	*/
/*				overlap list (using a set_func). pArray must already be	*/
/*				ordered by minimum AABB vertex.							*/
/************************************************************************/
void physics::SweepAxisList( int * pArray, int arraySize, select_func MinSel, select_func MaxSel, set_func Set )
{
	int curArrayIndex = 0; list<int>::iterator listIter;
	AABBActiveList.clear();
	while(curArrayIndex < arraySize)
	{
		// Check the current object against objects on the active list
		for(listIter = AABBActiveList.begin(); listIter != AABBActiveList.end(); /* nothing */ )
		{
			// If the new object start is after the active list end, then: REMOVE INDEX FROM ACTIVE LIST
			if(CALL_MEMBER_FN(*this,MaxSel)(g_rbobjects, *listIter) < CALL_MEMBER_FN(*this,MinSel)(g_rbobjects, pArray[curArrayIndex]))
			{
				(*Set)(AABBOverlapStatus, *listIter, pArray[curArrayIndex], false);
				listIter = AABBActiveList.erase(listIter);
			}
			else // OTHERWISE THERE IS OVERLAP BETWEEN THESE OBJECTS, so set overlap status
			{
				(*Set)(AABBOverlapStatus, *listIter, pArray[curArrayIndex], true);
				listIter++;
			}
		}

		// Put the current rbobject index on the active list
		AABBActiveList.push_back(pArray[curArrayIndex]);
		
		curArrayIndex ++;
	}
}
/************************************************************************/
/* Name:		CoarseCollisionDetection								*/
/* Description:	Use Axis aligned bounding boxes to find all sets of		*/
/*				potential collisions. Expected O(n+k+c)					*/
/************************************************************************/
void physics::CoarseCollisionDetection(void)
{
	// Update Bounding boxes
	for(int i = 0; i < (NO_RBOBJECTS); i++)
		g_rbobjects[i]->UpdateBoundingBox();

	// Do an insertion sort on each axis list to order objects, by minimum BB vertex
	// Insertion sort is O(n) when almost sorted, therefore best for slowly moving objects
	InsertionSortAABBs(AABBXaxis, NO_RBOBJECTS, (select_func) &physics::xAxisMinSel);
	InsertionSortAABBs(AABBYaxis, NO_RBOBJECTS, (select_func) &physics::yAxisMinSel);
	InsertionSortAABBs(AABBZaxis, NO_RBOBJECTS, (select_func) &physics::zAxisMinSel);

	// Now Find all overlaps by doing sweep of each axis lists.
	SweepAxisList(AABBXaxis, NO_RBOBJECTS, (select_func) &physics::xAxisMinSel, (select_func) &physics::xAxisMaxSel, (set_func) &SetOverlapStatusXaxis);
	SweepAxisList(AABBYaxis, NO_RBOBJECTS, (select_func) &physics::yAxisMinSel, (select_func) &physics::yAxisMaxSel, (set_func) &SetOverlapStatusYaxis);
	SweepAxisList(AABBZaxis, NO_RBOBJECTS, (select_func) &physics::zAxisMinSel, (select_func) &physics::zAxisMaxSel, (set_func) &SetOverlapStatusZaxis);
}
/************************************************************************/
/* Name:		ode														*/
/* Description:	RK4 integrator ODE core, take state y0 and progress 	*/
/*			forward from time t0 to t1, and return in yend. 			*/
/************************************************************************/
// http://en.wikipedia.org/wiki/Runge–Kutta_methods
void physics::ode(float y0[], float yend[], int len, float t0, float t1, dydt_func dydt)
{
	float dt = t1 - t0; // time step

	// CALCULATE k1
	CALL_MEMBER_FN(*this,dydt)(t0, y0, k1);		// (derivative of y0 at t0 stored into k1)
	// Take step with 0.5*dt*k1 and CALCULATE k2
	for(int i = 0; i < len; i ++) ytemp[i] = y0[i] + 0.5f*dt*k1[i];
	CALL_MEMBER_FN(*this,dydt)(t0 + 0.5f*dt, ytemp, k2);
	// Take step with 0.5*dt*k2 and CALCULATE k2
	for(int i = 0; i < len; i ++) ytemp[i] = y0[i] + 0.5f*dt*k2[i];
	CALL_MEMBER_FN(*this,dydt)(t0 + 0.5f*dt, ytemp, k3);
	// Take step with dt*k3 and CALCULATE k4
	for(int i = 0; i < len; i ++) ytemp[i] = y0[i] + dt*k3[i];
	CALL_MEMBER_FN(*this,dydt)(t0 + dt, ytemp, k4);

	// Calculate final value using weighted averaging of 4 slopes
	for(int i = 0; i < len; i ++)
		yend[i] = y0[i] + (dt*(k1[i] + 2.0f*k2[i] + 2.0f*k3[i] + k4[i]))/6.0f;
}
/************************************************************************/
/* Name:		dydt													*/
/* Description:	Given a state y[t], dydt simply calculates the			*/
/*			derivative at the time t, and returns the result in ydot	*/
/************************************************************************/
void physics::dydt(float t, float y[], float ydot[])
{
	rbobject::StateToObjects(y); // Store state in y[] into rbobjects
	for(int i = 0; i < (NO_RBOBJECTS); i++)
	{
		ComputeForceAndTorque(t, g_rbobjects[i]);
		ddtStateToArray(g_rbobjects[i], &ydot[i * STATESIZE]);
	}
}
/************************************************************************/
/* Name:		ComputeForceAndTorque									*/
/* Description:	Compute time dependant force and torque					*/
/************************************************************************/
void physics::ComputeForceAndTorque(float t, rbobject * rb)
{
	// Zero force and add gravity
	if(!rb->immovable)
	{
		rb->force = GRAVITY;
		rb->torque.x = 0.0f; rb->torque.y = 0.0f; rb->torque.z = 0.0f;
	}
	else
	{
		rb->force.x = 0.0f; rb->force.y = 0.0f; rb->force.z = 0.0f;
		rb->torque.x = 0.0f; rb->torque.y = 0.0f; rb->torque.z = 0.0f;
	}
}
/************************************************************************/
/* Name:		ddtStateToArray											*/
/* Description:	Given a rbobject, copy derivative into array ydot		*/
/************************************************************************/
void physics::ddtStateToArray(rbobject * rb, float * ydot)
{
	// dx(t)/dt = vel(t)
	*ydot++ = rb->v[0]; *ydot++ = rb->v[1]; *ydot++ = rb->v[2];
	// qdot(t) = 0.5 * w(t) q(t)
	D3DXQUATERNION qomega(rb->omega.x, rb->omega.y, rb->omega.z, 0.0f);
	D3DXQUATERNION qdot = 0.5 * (qomega * rb->q); // returns quaternion product [0,rb->omega]q
	*ydot++ = qdot.w; *ydot++ = qdot.x; *ydot++ = qdot.y; *ydot++ = qdot.z; 
	// dP(t)/dt = F(t)
	*ydot++ = rb->force.x; *ydot++ = rb->force.y; *ydot++ = rb->force.z;
	// dL(t)/dt = torque(t)
	*ydot++ = rb->torque.x; *ydot++ = rb->torque.y; *ydot++ = rb->torque.z;
}
/************************************************************************/
/* Name:		Resize_OBBCollisions									*/
/* Description: double the size of the array (amortized O(2))			*/
/************************************************************************/
void physics::Resize_OBBCollisions(void)
{
#ifdef _DEBUG
	//wchar_t OutputText[128]; swprintf(OutputText,127,L"Resizing OBBCollisions to %d\n",2*capacity_OBBCollisions);
	//OutputDebugString(OutputText);	
	__asm int 3; // debug breakpoint --> IF BREAKING HERE, CONSIDER STARTING WITH A NEW ARRAY
#endif

	// double vector size for amortized time of O(2)
	collision * new_OBBCollisions = new collision[capacity_OBBCollisions*2];
	// Copy old tree data into new structure
	for(UINT curIndex = 0; curIndex < size_OBBCollisions; curIndex ++)
	{
		new_OBBCollisions[curIndex] = OBBCollisions[curIndex];
	}
	// Delete the old allocated memory block and swap with new one
	delete [] OBBCollisions;
	OBBCollisions = new_OBBCollisions;
	capacity_OBBCollisions = capacity_OBBCollisions*2;
}
/************************************************************************/
/* Name:		GetNext_OBBCollisions									*/
/* Description: Add an element to the back of the array and return its  */
/*		address or index, resize if necessary.							*/
/************************************************************************/
void physics::GetNext_OBBCollisions(collision ** element )
{
	while(size_OBBCollisions >= capacity_OBBCollisions)
		this->Resize_OBBCollisions();
	*element = &OBBCollisions[size_OBBCollisions]; // Element after the last
	size_OBBCollisions = size_OBBCollisions + 1; // Increase size now
}
void physics::GetNext_OBBCollisions(UINT * element)
{
	while(size_OBBCollisions >= capacity_OBBCollisions)
		this->Resize_OBBCollisions();
	*element = size_OBBCollisions; // Element after the last
	size_OBBCollisions = size_OBBCollisions + 1; // Increase size now
}
/************************************************************************/
/* Name:		GetAt													*/
/* Description: Just dereference the array								*/
/************************************************************************/
collision * physics::GetAt_OBBCollisions(UINT curIndex)
{
	if(curIndex > (size_OBBCollisions-1) || curIndex < 0)
		throw std::runtime_error("physics::GetAt_OBBCollisions: curIndex is out of bounds");
	return &OBBCollisions[curIndex];
}
/************************************************************************/
/* Name:		GetAt													*/
/* Description: Just dereference the array								*/
/************************************************************************/
bool physics::Empty_OBBCollisions()
{
	return (size_OBBCollisions == 0);
}
/************************************************************************/
/* Name:		Size_OBBCollisions										*/
/* Description: Just return the number of stored values					*/
/************************************************************************/
UINT physics::Size_OBBCollisions()
{
	return size_OBBCollisions;
}
