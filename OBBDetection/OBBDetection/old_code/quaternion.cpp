// VECTOR OPERATIONS - To save time I started with code from:
// http://www.gamasutra.com/features/19990702/data_structures_01.htm
// This is just modified vec3D code --> Not very well tested.
// Jonathan Tompson, Summer 2009

#include	"main.h"

quaternion::quaternion() // default constructor
{
	w=0; x=0; y=0; z=0;	
}

quaternion::quaternion( const float& a, const float& b, const float& c, const float& d )
{
	w=a; x=b; y=c; z=d;
}

//index a component (idea for this operation taken from gamasutra)
//NOTE: returning a reference allows you to assign the indexed element
float& quaternion::operator [] ( const long i )
{
	return *((&x) + i);
}

//add
const quaternion quaternion::operator + ( const quaternion& v ) const
{
	return quaternion(w + v.w, x + v.x, y + v.y, z + v.z);
}

//subtract
const quaternion quaternion::operator - ( const quaternion& v ) const
{
	return quaternion(w - v.w, x - v.x, y - v.y, z - v.z);
}

//negate
const quaternion quaternion::operator - () const
{
	return quaternion( -w, -x, -y, -z );
}

//assign
const quaternion& quaternion::operator = ( const quaternion& v )
{
	w = v.w; x = v.x; y = v.y; z = v.z;
	return *this;
}

//increment
const quaternion& quaternion::operator += ( const quaternion& v )
{
	w+=v.w; x+=v.x; y+=v.y; z+=v.z;
	return *this;
}

//decrement
const quaternion& quaternion::operator -= ( const quaternion& v )
{
	w-=v.w; x-=v.x; y-=v.y; z-=v.z;
	return *this;
}

//compare
const bool quaternion::operator == ( const quaternion& v ) const
{
	return (v.w==w && v.x==x && v.y==y && v.z==z);
}

const bool quaternion::operator != ( const quaternion& v ) const
{
	return !(v == *this);
}

//self-multiply
const quaternion& quaternion::operator *= ( const float& s )
{
	w*=s; x*=s; y*=s; z*=s;
	return *this;
}

//self-divide
const quaternion& quaternion::operator /= ( const float& s )
{
	const float r = 1 / s;
	w *= r; x *= r; y *= r; z *= r;
	return *this;
}

//post-multiply by a float
const quaternion quaternion::operator * ( const float& s ) const
{
	return quaternion( w*s, x*s, y*s, z*s );
}

//divide
const quaternion quaternion::operator / (float s) const
{
	s = 1/s;
	return quaternion( s*w, s*x, s*y, s*z );
}

//quaternion compose
//[w',v'] = [w1*w2 - v1.v2, w1*v2 + w2*v1 - v1xv2]
const quaternion quaternion::compose( const quaternion& v ) const
{
	quaternion ret;
	ret.w = w*v.w - (x*v.x + y*v.y + z*v.z);
	ret.x = w*v.x + x*v.w + y*v.z - z*v.y;
	ret.y = w*v.y + y*v.w + z*v.x - x*v.z;
	ret.z = w*v.z + z*v.w + x*v.y - y*v.x;
	return ret;
}


//length
const float quaternion::length() const
{
	return (float)sqrt( (float)(w*w + x*x + y*y + z*z));
}

//unit vec3D
const quaternion quaternion::unit() const
{
	float mag = length();
	if(mag == 0.0)
		return (*this);
	else
		return (*this) / mag;
}

//make this a unit vec3D
void quaternion::normalize()
{
	float mag = length();
	if(mag == 0.0)
		return;
	else
		(*this) /= mag;
}

// rotation theta about vector vec
void quaternion::rotation(const float &theta,const vec3D &vec)
{
	vec3D vec_unit = vec.unit();
	w = cos(theta / 2.0f);
	vec_unit *= sin(theta / 2.0f);
	x = vec_unit.x;
	y = vec_unit.y;
	z = vec_unit.z;
}

//equal within some tollerence
const bool quaternion::roughlyEqual( const quaternion& v, const float e ) const
{
	return abs(w-v.w)<e && abs(x-v.x)<e && abs(y-v.y)<e && abs(z-v.z)<e;
}

// Translate quaternion into rotional matrix
// From: "Physically Based Modelling: Principles and Practice", Baraff and Witkin
mat3D quaternion::quaternion_to_mat3D(void)
{
	mat3D ret;
	ret.elm[0][0]= 1.0f - 2*(y*y) - 2*(z*z); 
	ret.elm[0][1]= 2*(x*y) - 2*(w*z); 
	ret.elm[0][2]= 2*(x*z) + 2*(w*y);
	ret.elm[1][0]= 2*(x*y) + 2*(w*z); 
	ret.elm[1][1]= 1.0f - 2*(x*x) - 2*(z*z); 
	ret.elm[1][2]= 2*(y*z) - 2*(w*x);
	ret.elm[2][0]= 2*(x*z) - 2*(w*y); 
	ret.elm[2][1]= 2*(y*z) + 2*(w*x); 
	ret.elm[2][2]= 1.0f - 2*(x*x) - 2*(y*y);
	return ret;
}