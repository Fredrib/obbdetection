// VECTOR OPERATIONS - To save time I started with code from:
// http://www.gamasutra.com/features/19990702/data_structures_01.htm
// Jonathan Tompson, Summer 2009

#ifndef VEC3D_h
#define VEC3D_h

#include	"main.h"

class vec3D
{
public:

	float x,y,z;	//x,y,z coordinates

public:

	vec3D(); // default constructor
	vec3D( const float& a, const float& b, const float& c ); 

	float& operator [] ( const long i ); //index a component
	const vec3D operator + ( const vec3D& v ) const; //add
	const vec3D operator - ( const vec3D& v ) const; //subtract
	const vec3D operator - () const; //negate
	const vec3D& operator = ( const vec3D& v ); //assign
	const vec3D& operator += ( const vec3D& v ); //increment
	const vec3D& operator -= ( const vec3D& v ); //decrement
	const bool operator == ( const vec3D& v ) const; //compare
	const bool operator != ( const vec3D& v ) const; 
	const vec3D& operator *= ( const float& s ); //self-multiply
	const vec3D& operator /= ( const float& s ); //self-divide
	const vec3D operator * ( const float& s ) const; //post-multiply by a float
	const vec3D operator / (float s) const; //divide

	const float dot( const vec3D& v ) const; //dot product
	const vec3D cross( const vec3D& v ) const; //cross product
	const float length() const; //length
	const vec3D unit() const; //unit vec3D
	void normalize(); //make this a unit vec3D
	const bool roughlyEqual( const vec3D& v, const float e ) const; //equal within some tollerence

	//pre-multiply by a float --> Best way to do this.
	friend inline const vec3D operator * ( const float& s, const vec3D& v )
	{
		return v * s;
	}
};

#endif