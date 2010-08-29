// VECTOR OPERATIONS - To save time I started with code from:
// http://www.gamasutra.com/features/19990702/data_structures_01.htm
// Jonathan Tompson, Summer 2009

#include	"main.h"

vec3D::vec3D() // default constructor
{
	x=0; y=0; z=0;	
}

vec3D::vec3D( const float& a, const float& b, const float& c )
{
	x=a; y=b; z=c;
}

//index a component (idea for this operation taken from gamasutra)
//NOTE: returning a reference allows you to assign the indexed element

float& vec3D::operator [] ( const long i )
{
	return *((&x) + i);
}

//add
const vec3D vec3D::operator + ( const vec3D& v ) const
{
	return vec3D(x + v.x, y + v.y, z + v.z);
}

//subtract
const vec3D vec3D::operator - ( const vec3D& v ) const
{
	return vec3D(x - v.x, y - v.y, z - v.z);
}

//negate
const vec3D vec3D::operator - () const
{
	return vec3D( -x, -y, -z );
}

//assign
const vec3D& vec3D::operator = ( const vec3D& v )
{
	x = v.x; y = v.y; z = v.z;
	return *this;
}

//increment
const vec3D& vec3D::operator += ( const vec3D& v )
{
	x+=v.x; y+=v.y; z+=v.z;
	return *this;
}

//decrement
const vec3D& vec3D::operator -= ( const vec3D& v )
{
	x-=v.x; y-=v.y; z-=v.z;
	return *this;
}

//compare
const bool vec3D::operator == ( const vec3D& v ) const
{
	return (v.x==x && v.y==y && v.z==z);
}

const bool vec3D::operator != ( const vec3D& v ) const
{
	return !(v == *this);
}

//self-multiply
const vec3D& vec3D::operator *= ( const float& s )
{
	x*=s; y*=s; z*=s;
	return *this;
}

//self-divide
const vec3D& vec3D::operator /= ( const float& s )
{
	const float r = 1 / s;
	x *= r; y *= r; z *= r;
	return *this;
}

//post-multiply by a float
const vec3D vec3D::operator * ( const float& s ) const
{
	return vec3D( x*s, y*s, z*s );
}

//divide
const vec3D vec3D::operator / (float s) const
{
	s = 1/s;
	return vec3D( s*x, s*y, s*z );
}

//dot product
const float vec3D::dot( const vec3D& v ) const
{
	return x*v.x + y*v.y + z*v.z;
}

//cross product
const vec3D vec3D::cross( const vec3D& v ) const
{
	//Davis, Snider, "Introduction to vec3D Analysis", p. 44
	return vec3D( y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x );
}

//length
const float vec3D::length() const
{
	return (float)sqrt( (float)this->dot(*this) );
}

//unit vec3D
const vec3D vec3D::unit() const
{
	float mag = length();
	if(mag == 0.0)
		return (*this);
	else
		return (*this) / mag;
}

//make this a unit vec3D
void vec3D::normalize()
{
	float mag = length();
	if(mag == 0.0)
		return;
	else
		(*this) /= mag;
}

//equal within some tollerence
const bool vec3D::roughlyEqual( const vec3D& v, const float e ) const
{
	return abs(x-v.x)<e && abs(y-v.y)<e && abs(z-v.z)<e;
}