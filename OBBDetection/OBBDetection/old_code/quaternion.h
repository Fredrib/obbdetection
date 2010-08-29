// VECTOR OPERATIONS - To save time I started with code from:
// http://www.gamasutra.com/features/19990702/data_structures_01.htm
// This is just modified vec3D code --> Not very well tested.
// Jonathan Tompson, Summer 2009

#ifndef QUATERNION_h
#define QUATERNION_h

#include	"main.h"

class mat3D;
class vec3D;
#include "mat3D.h"

class quaternion
{
public:

	float w,x,y,z;	//w,x,y,z coordinates

public:
	quaternion(); // default constructor
	quaternion( const float& a, const float& b, const float& c, const float& d ); 

	float& operator [] ( const long i ); //index a component
	const quaternion operator + ( const quaternion& v ) const; //add
	const quaternion operator - ( const quaternion& v ) const; //subtract
	const quaternion operator - () const; //negate
	const quaternion& operator = ( const quaternion& v ); //assign
	const quaternion& operator += ( const quaternion& v ); //increment
	const quaternion& operator -= ( const quaternion& v ); //decrement
	const bool operator == ( const quaternion& v ) const; //compare
	const bool operator != ( const quaternion& v ) const;
	const quaternion& operator *= ( const float& s ); //self-multiply
	const quaternion& operator /= ( const float& s ); //self-divide
	const quaternion operator * ( const float& s ) const; //post-multiply by a float
	const quaternion operator / (float s) const; //divide

	const quaternion compose( const quaternion& v ) const; //quaternion composition
	const float length() const; //length
	const quaternion unit() const; //unit vec3D
	void normalize(); //make this a unit vec3D
	void rotation(const float &theta,const vec3D &vec); // rotation theta about vector vec
	const bool roughlyEqual( const quaternion& v, const float e ) const; //equal within some tollerence
	mat3D quaternion_to_mat3D(void); // Translate quaternion into rotional matrix

	//pre-multiply by a float --> Best way to do this.
	friend inline const quaternion quaternion::operator * ( const float& s, const quaternion& v )
	{
		return v * s;
	}

};

#endif