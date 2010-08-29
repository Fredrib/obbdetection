/*************************************************************
**				OBBDetection Collision Class				**
*************************************************************/
// File:		collision.h
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

#include	"main.h"

#ifndef collision_h
#define collision_h

enum CollisionType
{
	COL_UNDEFINED     = 0,
	EDGE_EDGE         = 1,
	EDGE_FACE         = 2,
};

class collision
{
public:
					collision(void);
					~collision(void);
					collision(obbox * obbox_a_in, obbox * obbox_b_in);
	collision &		operator = (const collision & o); // Assignment operator for vector array in physics (when resizing)

	// Pointer to the colliding objects.
	obbox *			obbox_a;
	obbox *			obbox_b;
	CollisionType	Type;

	/********************************************************************/
	/***** IF YOU ADD MEMBERS, ADD THEM TO THE ASSIGNMENT OPERATOR ******/
	/********************************************************************/

};

#endif