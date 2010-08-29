// MATRIX OPERATIONS - The code from:
// http://www.gamasutra.com/features/19990702/data_structures_02.htm
// Wasn't that good I think.  They used an array of vec3D, which meant indexing was
// [col][row], instead of [row][col] --> I know this would have caused bugs.
// So I wrote my own methods.
// Jonathan Tompson, Summer 2009

#ifndef MAT3D_h
#define MAT3D_h

#include	"main.h"

class quaternion;

class mat3D
{
public:

	float elm[3][3]; // val[0][0] is top left hand side, [2][2] is bottom right.
	// Note: 1D array has less overhead but this is easier to read
public:

	mat3D(); // default constructor makes the identity matrix
	mat3D( const vec3D& c0, const vec3D& c1, const vec3D& c2 );

	const bool operator == ( const mat3D& m ) const; //compare
	const bool operator != ( const mat3D& m ) const;
	const mat3D& operator = ( const mat3D& m ); //assign
	const mat3D& operator +=( const mat3D& m ); //increment
	const mat3D& operator -=( const mat3D& m ); //decrement
	const mat3D& operator *= ( const float& s ); //self-multiply by a float
	const mat3D operator * ( const mat3D& m ) const; //post-multiply by a mat3D
	const mat3D operator + ( const mat3D& m ) const; //add
	const mat3D operator - ( const mat3D& m ) const; //subtract
	const mat3D operator * ( const float& s ) const; //post-multiply by a float
	// pre-multiply by a float
	const vec3D operator * ( const vec3D& v ) const; //post-multiply by a vec3D
	mat3D transpose() const; //transpose
	const float determinant() const; //float determinant
	const mat3D inverse() const;  //mat3D inverse
	quaternion mat3D_to_quaternion(void); //MATRIX TO QUATERNION

	//pre-multiply by a float --> Best way to do this.
	friend inline const mat3D operator * ( const float& s, const mat3D& m )
	{
		return m * s;
	}
};

#endif