/*************************************************************
**				OBBDetection Collision Class				**
*************************************************************/
// File:		collision.cpp
// Author:		Jonathan Tompson
// e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

#include	"main.h"

/************************************************************************/
/* Name:		collision												*/
/* Description:	Default constructor.									*/
/************************************************************************/
collision::collision(void)
{
	Type = COL_UNDEFINED;
	obbox_a = NULL;
	obbox_b = NULL;
}
/************************************************************************/
/* Name:		collision												*/
/* Description:	Default constructor.									*/
/************************************************************************/
collision::collision(obbox * obbox_a_in, obbox * obbox_b_in)
{
	Type = COL_UNDEFINED;
	obbox_a = obbox_a_in;
	obbox_b = obbox_b_in;
}
/************************************************************************/
/* Name:		~collision												*/
/* Description:	Default destructor.										*/
/************************************************************************/
collision::~collision(void)
{
	// Nothing to do.
}
/************************************************************************/
/* Name:		operator =												*/
/* Description: Assignment operator for vector array in physics			*/
/*				(when resizing)											*/
/************************************************************************/
collision & collision::operator = (const collision & o)
{
	if (this != &o) // make sure not same object
	{
		// Perform shallow copies (same order as collision.h if you need to check)
		obbox_a = o.obbox_a;
		obbox_b = o.obbox_b;
		Type = o.Type;
	}
	return *this;  // Return ref for multiple assignment
}