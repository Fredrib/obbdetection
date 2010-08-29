// MATRIX OPERATIONS - The code from:
// http://www.gamasutra.com/features/19990702/data_structures_02.htm
// Wasn't that good I think.  They used an array of vec3D, which meant indexing was
// [col][row], instead of [row][col] --> I know this would have caused bugs.
// So I wrote my own methods.
// Jonathan Tompson, Summer 2009

#include	"main.h"

mat3D::mat3D() // default constructor makes the identity matrix
{
	//identity mat3D
	elm[0][0] = 1; elm[0][1] = 0; elm[0][2] = 0;
	elm[1][0] = 0; elm[1][1] = 1; elm[1][2] = 0;
	elm[2][0] = 0; elm[2][1] = 0; elm[2][2] = 1;
}

mat3D::mat3D( const vec3D& c0, const vec3D& c1, const vec3D& c2 )
{
	elm[0][0] = c0.x; elm[0][1] = c1.x; elm[0][2] = c2.x;
	elm[1][0] = c0.y; elm[1][1] = c1.y; elm[1][2] = c2.y;
	elm[2][0] = c0.z; elm[2][1] = c1.z; elm[2][2] = c2.z;		
}

//compare
const bool mat3D::operator == ( const mat3D& m ) const
{
	return	elm[0][0]==m.elm[0][0] && elm[0][1]==m.elm[0][1] && elm[0][2]==m.elm[0][2] &&
		elm[1][0]==m.elm[1][0] && elm[1][1]==m.elm[1][1] && elm[1][2]==m.elm[1][2] &&
		elm[2][0]==m.elm[2][0] && elm[2][1]==m.elm[2][1] && elm[2][2]==m.elm[2][2];
}

const bool mat3D::operator != ( const mat3D& m ) const
{
	return !(m == *this);
}

//assign
const mat3D& mat3D::operator = ( const mat3D& m )
{
	elm[0][0]=m.elm[0][0]; elm[0][1]=m.elm[0][1]; elm[0][2]=m.elm[0][2];
	elm[1][0]=m.elm[1][0]; elm[1][1]=m.elm[1][1]; elm[1][2]=m.elm[1][2];
	elm[2][0]=m.elm[2][0]; elm[2][1]=m.elm[2][1]; elm[2][2]=m.elm[2][2];
	return *this;
}

//increment
const mat3D& mat3D::operator +=( const mat3D& m )
{
	elm[0][0]+=m.elm[0][0]; elm[0][1]+=m.elm[0][1]; elm[0][2]+=m.elm[0][2];
	elm[1][0]+=m.elm[1][0]; elm[1][1]+=m.elm[1][1]; elm[1][2]+=m.elm[1][2];
	elm[2][0]+=m.elm[2][0]; elm[2][1]+=m.elm[2][1]; elm[2][2]+=m.elm[2][2];
	return *this;
}

//decrement
const mat3D& mat3D::operator -=( const mat3D& m )
{
	elm[0][0]-=m.elm[0][0]; elm[0][1]-=m.elm[0][1]; elm[0][2]-=m.elm[0][2];
	elm[1][0]-=m.elm[1][0]; elm[1][1]-=m.elm[1][1]; elm[1][2]-=m.elm[1][2];
	elm[2][0]-=m.elm[2][0]; elm[2][1]-=m.elm[2][1]; elm[2][2]-=m.elm[2][2];
	return *this;
}

//self-multiply by a float
const mat3D& mat3D::operator *= ( const float& s )
{
	elm[0][0]*=s; elm[0][1]*=s; elm[0][2]*=s;
	elm[1][0]*=s; elm[1][1]*=s; elm[1][2]*=s;
	elm[2][0]*=s; elm[2][1]*=s; elm[2][2]*=s;
	return *this;
}

//post-multiply by a mat3D
const mat3D mat3D::operator * ( const mat3D& m ) const
{
	mat3D ret;
	ret.elm[0][0] = elm[0][0]*m.elm[0][0] + elm[0][1]*m.elm[1][0] + elm[0][2]*m.elm[2][0];
	ret.elm[0][1] = elm[0][0]*m.elm[0][1] + elm[0][1]*m.elm[1][1] + elm[0][2]*m.elm[2][1];
	ret.elm[0][2] = elm[0][0]*m.elm[0][2] + elm[0][1]*m.elm[1][2] + elm[0][2]*m.elm[2][2];
	ret.elm[1][0] = elm[1][0]*m.elm[0][0] + elm[1][1]*m.elm[1][0] + elm[1][2]*m.elm[2][0];
	ret.elm[1][1] = elm[1][0]*m.elm[0][1] + elm[1][1]*m.elm[1][1] + elm[1][2]*m.elm[2][1];
	ret.elm[1][2] = elm[1][0]*m.elm[0][2] + elm[1][1]*m.elm[1][2] + elm[1][2]*m.elm[2][2];
	ret.elm[2][0] = elm[2][0]*m.elm[0][0] + elm[2][1]*m.elm[1][0] + elm[2][2]*m.elm[2][0];
	ret.elm[2][1] = elm[2][0]*m.elm[0][1] + elm[2][1]*m.elm[1][1] + elm[2][2]*m.elm[2][1];
	ret.elm[2][2] = elm[2][0]*m.elm[0][2] + elm[2][1]*m.elm[1][2] + elm[2][2]*m.elm[2][2];
	return ret;
}

//add
const mat3D mat3D::operator + ( const mat3D& m ) const
{
	mat3D ret;
	ret.elm[0][0]=elm[0][0]+m.elm[0][0]; ret.elm[0][1]=elm[0][1]+m.elm[0][1]; ret.elm[0][2]=elm[0][2]+m.elm[0][2];
	ret.elm[1][0]=elm[1][0]+m.elm[1][0]; ret.elm[1][1]=elm[1][1]+m.elm[1][1]; ret.elm[1][2]=elm[1][2]+m.elm[1][2];
	ret.elm[2][0]=elm[2][0]+m.elm[2][0]; ret.elm[2][1]=elm[2][1]+m.elm[2][1]; ret.elm[2][2]=elm[2][2]+m.elm[2][2];
	return ret;
}

//subtract
const mat3D mat3D::operator - ( const mat3D& m ) const
{
	mat3D ret;
	ret.elm[0][0]=elm[0][0]-m.elm[0][0]; ret.elm[0][1]=elm[0][1]-m.elm[0][1]; ret.elm[0][2]=elm[0][2]-m.elm[0][2];
	ret.elm[1][0]=elm[1][0]-m.elm[1][0]; ret.elm[1][1]=elm[1][1]-m.elm[1][1]; ret.elm[1][2]=elm[1][2]-m.elm[1][2];
	ret.elm[2][0]=elm[2][0]-m.elm[2][0]; ret.elm[2][1]=elm[2][1]-m.elm[2][1]; ret.elm[2][2]=elm[2][2]-m.elm[2][2];
	return ret;
}

//post-multiply by a float
const mat3D mat3D::operator * ( const float& s ) const
{
	mat3D ret;
	ret.elm[0][0]=elm[0][0]*s; ret.elm[0][1]=elm[0][1]*s; ret.elm[0][2]=elm[0][2]*s;
	ret.elm[1][0]=elm[1][0]*s; ret.elm[1][1]=elm[1][1]*s; ret.elm[1][2]=elm[1][2]*s;
	ret.elm[2][0]=elm[2][0]*s; ret.elm[2][1]=elm[2][1]*s; ret.elm[2][2]=elm[2][2]*s;
	return ret;
}

//post-multiply by a vec3D
const vec3D mat3D::operator * ( const vec3D& v ) const
{
	vec3D ret;
	ret.x = this->elm[0][0] * v.x + this->elm[0][1] * v.y + this->elm[0][2] * v.z;
	ret.y = this->elm[1][0] * v.x + this->elm[1][1] * v.y + this->elm[1][2] * v.z;
	ret.z = this->elm[2][0] * v.x + this->elm[2][1] * v.y + this->elm[2][2] * v.z;
	return ret;
}

//transpose
mat3D mat3D::transpose() const
{
	mat3D ret;
	ret.elm[0][0]=elm[0][0]; ret.elm[0][1]=elm[1][0]; ret.elm[0][2]=elm[2][0];
	ret.elm[1][0]=elm[0][1]; ret.elm[1][1]=elm[1][1]; ret.elm[1][2]=elm[2][1];
	ret.elm[2][0]=elm[0][2]; ret.elm[2][1]=elm[1][2]; ret.elm[2][2]=elm[2][2];
	return ret;
}

//float determinant
const float mat3D::determinant() const
{
	return (elm[0][0]*elm[1][1]*elm[2][2] + elm[0][1]*elm[1][2]*elm[2][0] + elm[0][2]*elm[1][0]*elm[2][1] -
		elm[2][0]*elm[1][1]*elm[0][2] - elm[2][1]*elm[1][2]*elm[0][0] - elm[2][2]*elm[1][0]*elm[0][1]);
}

//mat3D inverse
// test using this --> http://www.maths.surrey.ac.uk/explore/emmaspages/java/Matrix8.html
const mat3D mat3D::inverse() const 
{
	float det = this->determinant();
	mat3D ret;
	ret.elm[0][0]=elm[2][2]*elm[1][1]-elm[2][1]*elm[1][2]; 
	ret.elm[0][1]=-(elm[2][2]*elm[0][1]-elm[2][1]*elm[0][2]); 
	ret.elm[0][2]=elm[1][2]*elm[0][1]-elm[1][1]*elm[0][2];
	ret.elm[1][0]=-(elm[2][2]*elm[1][0]-elm[2][0]*elm[1][2]); 
	ret.elm[1][1]=elm[2][2]*elm[0][0]-elm[2][0]*elm[0][2]; 
	ret.elm[1][2]=-(elm[1][2]*elm[0][0]-elm[1][0]*elm[0][2]);
	ret.elm[2][0]=elm[2][1]*elm[1][0]-elm[2][0]*elm[1][1]; 
	ret.elm[2][1]=-(elm[2][1]*elm[0][0]-elm[2][0]*elm[0][1]); 
	ret.elm[2][2]=elm[1][1]*elm[0][0]-elm[1][0]*elm[0][1];
	return (  ret * (1.0f/det));
}

//MATRIX TO QUATERNION
// Code taken from "Physically Based Modelling: Principles and Practice", Baraff and Witkin
quaternion mat3D::mat3D_to_quaternion(void)
{
	quaternion	ret;
	float		tr, s;
	tr = elm[0][0] + elm[1][1] + elm[2][2];
	if(tr >=0)
	{
		s = sqrt(tr + 1.0f);
		ret.w = 0.5f * s;
		s = 0.5f / s;
		ret.x = (elm[2][1] - elm[1][2]) * s;
		ret.y = (elm[0][2] - elm[2][0]) * s;
		ret.z = (elm[1][0] - elm[0][1]) * s;
	}
	else
	{
		int i = 0;
		if(elm[1][1] > elm[0][0])
			i = 1;
		if(elm[2][2] > elm[i][i])
			i = 2;
		switch (i)
		{
		case 0:
			s = sqrt((elm[0][0] - (elm[1][1]+elm[2][2])) + 1.0f);
			ret.x = 0.5f * s;
			s = 0.5f / s;
			ret.y = (elm[0][1] + elm[1][0]) * s;
			ret.z = (elm[2][0] + elm[0][2]) * s;
			ret.w = (elm[2][1] - elm[1][2]) * s;
			break;
		case 1:
			s = sqrt((elm[1][1] - (elm[2][2]+elm[0][0])) + 1.0f);
			ret.y = 0.5f * s;
			s = 0.5f / s;
			ret.z = (elm[1][2] + elm[2][1]) * s;
			ret.x = (elm[0][1] + elm[1][0]) * s;
			ret.w = (elm[0][2] - elm[2][0]) * s;
			break;
		case 2:
			s = sqrt((elm[2][2] - (elm[0][0]+elm[1][1])) + 1.0f);
			ret.z = 0.5f * s;
			s = 0.5f / s;
			ret.x = (elm[2][0] + elm[0][2]) * s;
			ret.y = (elm[1][2] + elm[2][1]) * s;
			ret.w = (elm[1][0] - elm[0][1]) * s;
			break;

		}
	}
	return ret;
}