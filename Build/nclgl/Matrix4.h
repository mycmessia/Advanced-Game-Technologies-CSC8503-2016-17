/******************************************************************************
Class:Matrix4
Implements:
Author:Rich Davison
Description:VERY simple 4 by 4 matrix class. Students are encouraged to modify 
this as necessary! Overloading the [] operator to allow acces to the values
array in a neater way might be a good start, as the floats that make the matrix 
up are currently public.

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <iostream>
#include "common.h"
#include "Vector3.h"
#include "Vector4.h"

class Vector3;
class Matrix3;

class Matrix4	{
public:
	Matrix4(void);
	Matrix4(float elements[16]);
	Matrix4(const Matrix3& mat33);
	~Matrix4(void);

	float	values[16];

	//Set all matrix values to zero
	void	ToZero();
	//Sets matrix to identity matrix (1.0 down the diagonal)
	void	ToIdentity();

	//Gets the OpenGL position vector (floats 12,13, and 14)
	Vector3 GetPositionVector() const;
	//Sets the OpenGL position vector (floats 12,13, and 14)
	void	SetPositionVector(const Vector3 in);

	//Gets the scale vector (floats 1,5, and 10)
	Vector3 GetScalingVector() const;
	//Sets the scale vector (floats 1,5, and 10)
	void	SetScalingVector(const Vector3 &in);

	//Creates a rotation matrix that rotates by 'degrees' around the 'axis'
	//Analogous to glRotatef
	static Matrix4 Rotation(float degrees, const Vector3 &axis);

	//Creates a scaling matrix (puts the 'scale' vector down the diagonal)
	//Analogous to glScalef
	static Matrix4 Scale(const Vector3 &scale);

	//Creates a translation matrix (identity, with 'translation' vector at
	//floats 12, 13, and 14. Analogous to glTranslatef
	static Matrix4 Translation(const Vector3 &translation);

	//Creates a perspective matrix, with 'znear' and 'zfar' as the near and 
	//far planes, using 'aspect' and 'fov' as the aspect ratio and vertical
	//field of vision, respectively.
	static Matrix4 Perspective(float znear, float zfar, float aspect, float fov);

	//Creates an orthographic matrix with 'znear' and 'zfar' as the near and 
	//far planes, and so on. Descriptive variable names are a good thing!
	static Matrix4 Orthographic(float znear, float zfar,float right, float left, float top, float bottom);

	//Builds a view matrix suitable for sending straight to the vertex shader.
	//Puts the camera at 'from', with 'lookingAt' centered on the screen, with
	//'up' as the...up axis (pointing towards the top of the screen)
	static Matrix4 BuildViewMatrix(const Vector3 &from, const Vector3 &lookingAt, const Vector3 up = Vector3(0,1,0));

	Matrix4 GetRotation() const;
	Matrix4 GetTransposedRotation() const;

	//Multiplies 'this' matrix by matrix 'a'. Performs the multiplication in 'OpenGL' order (ie, backwards)
	inline Matrix4 operator*(const Matrix4 &a) const{	
		Matrix4 out;
		//Students! You should be able to think up a really easy way of speeding this up...
		for(unsigned int r = 0; r < 4; ++r) {
			for(unsigned int c = 0; c < 4; ++c) {
				out.values[c + (r*4)] = 0.0f;
				for(unsigned int i = 0; i < 4; ++i) {
					out.values[c + (r*4)] += this->values[c+(i*4)] * a.values[(r*4)+i];
				}
			}
		}
		return out;
	}

	inline Vector3 operator*(const Vector3 &v) const {
		Vector3 vec;

		float temp;

		vec.x = v.x*values[0] + v.y*values[4] + v.z*values[8]  + values[12];
		vec.y = v.x*values[1] + v.y*values[5] + v.z*values[9]  + values[13];
		vec.z = v.x*values[2] + v.y*values[6] + v.z*values[10] + values[14];

		temp =  v.x*values[3] + v.y*values[7] + v.z*values[11] + values[15];

		vec.x = vec.x/temp;
		vec.y = vec.y/temp;
		vec.z = vec.z/temp;

		return vec;
	};

		inline Vector4 operator*(const Vector4 &v) const {
		return Vector4(
			v.x*values[0] + v.y*values[4] + v.z*values[8]  +v.w * values[12],
			v.x*values[1] + v.y*values[5] + v.z*values[9]  +v.w * values[13],
			v.x*values[2] + v.y*values[6] + v.z*values[10] +v.w * values[14],
			v.x*values[3] + v.y*values[7] + v.z*values[11] +v.w * values[15]
		);
	};



		inline float  operator[](int index) const {
			return values[index];
		}

		inline float&  operator[](int index) {
			return values[index];
		}

		//Added for GameTech - Code from taken from GLU library (all rights reserved).
		static Matrix4 Inverse(const Matrix4& rhs) {
			Matrix4 inv;
			float det;
			int i;

			inv[0] = rhs[5] * rhs[10] * rhs[15] -
				rhs[5] * rhs[11] * rhs[14] -
				rhs[9] * rhs[6] * rhs[15] +
				rhs[9] * rhs[7] * rhs[14] +
				rhs[13] * rhs[6] * rhs[11] -
				rhs[13] * rhs[7] * rhs[10];

			inv[4] = -rhs[4] * rhs[10] * rhs[15] +
				rhs[4] * rhs[11] * rhs[14] +
				rhs[8] * rhs[6] * rhs[15] -
				rhs[8] * rhs[7] * rhs[14] -
				rhs[12] * rhs[6] * rhs[11] +
				rhs[12] * rhs[7] * rhs[10];

			inv[8] = rhs[4] * rhs[9] * rhs[15] -
				rhs[4] * rhs[11] * rhs[13] -
				rhs[8] * rhs[5] * rhs[15] +
				rhs[8] * rhs[7] * rhs[13] +
				rhs[12] * rhs[5] * rhs[11] -
				rhs[12] * rhs[7] * rhs[9];

			inv[12] = -rhs[4] * rhs[9] * rhs[14] +
				rhs[4] * rhs[10] * rhs[13] +
				rhs[8] * rhs[5] * rhs[14] -
				rhs[8] * rhs[6] * rhs[13] -
				rhs[12] * rhs[5] * rhs[10] +
				rhs[12] * rhs[6] * rhs[9];

			inv[1] = -rhs[1] * rhs[10] * rhs[15] +
				rhs[1] * rhs[11] * rhs[14] +
				rhs[9] * rhs[2] * rhs[15] -
				rhs[9] * rhs[3] * rhs[14] -
				rhs[13] * rhs[2] * rhs[11] +
				rhs[13] * rhs[3] * rhs[10];

			inv[5] = rhs[0] * rhs[10] * rhs[15] -
				rhs[0] * rhs[11] * rhs[14] -
				rhs[8] * rhs[2] * rhs[15] +
				rhs[8] * rhs[3] * rhs[14] +
				rhs[12] * rhs[2] * rhs[11] -
				rhs[12] * rhs[3] * rhs[10];

			inv[9] = -rhs[0] * rhs[9] * rhs[15] +
				rhs[0] * rhs[11] * rhs[13] +
				rhs[8] * rhs[1] * rhs[15] -
				rhs[8] * rhs[3] * rhs[13] -
				rhs[12] * rhs[1] * rhs[11] +
				rhs[12] * rhs[3] * rhs[9];

			inv[13] = rhs[0] * rhs[9] * rhs[14] -
				rhs[0] * rhs[10] * rhs[13] -
				rhs[8] * rhs[1] * rhs[14] +
				rhs[8] * rhs[2] * rhs[13] +
				rhs[12] * rhs[1] * rhs[10] -
				rhs[12] * rhs[2] * rhs[9];

			inv[2] = rhs[1] * rhs[6] * rhs[15] -
				rhs[1] * rhs[7] * rhs[14] -
				rhs[5] * rhs[2] * rhs[15] +
				rhs[5] * rhs[3] * rhs[14] +
				rhs[13] * rhs[2] * rhs[7] -
				rhs[13] * rhs[3] * rhs[6];

			inv[6] = -rhs[0] * rhs[6] * rhs[15] +
				rhs[0] * rhs[7] * rhs[14] +
				rhs[4] * rhs[2] * rhs[15] -
				rhs[4] * rhs[3] * rhs[14] -
				rhs[12] * rhs[2] * rhs[7] +
				rhs[12] * rhs[3] * rhs[6];

			inv[10] = rhs[0] * rhs[5] * rhs[15] -
				rhs[0] * rhs[7] * rhs[13] -
				rhs[4] * rhs[1] * rhs[15] +
				rhs[4] * rhs[3] * rhs[13] +
				rhs[12] * rhs[1] * rhs[7] -
				rhs[12] * rhs[3] * rhs[5];

			inv[14] = -rhs[0] * rhs[5] * rhs[14] +
				rhs[0] * rhs[6] * rhs[13] +
				rhs[4] * rhs[1] * rhs[14] -
				rhs[4] * rhs[2] * rhs[13] -
				rhs[12] * rhs[1] * rhs[6] +
				rhs[12] * rhs[2] * rhs[5];

			inv[3] = -rhs[1] * rhs[6] * rhs[11] +
				rhs[1] * rhs[7] * rhs[10] +
				rhs[5] * rhs[2] * rhs[11] -
				rhs[5] * rhs[3] * rhs[10] -
				rhs[9] * rhs[2] * rhs[7] +
				rhs[9] * rhs[3] * rhs[6];

			inv[7] = rhs[0] * rhs[6] * rhs[11] -
				rhs[0] * rhs[7] * rhs[10] -
				rhs[4] * rhs[2] * rhs[11] +
				rhs[4] * rhs[3] * rhs[10] +
				rhs[8] * rhs[2] * rhs[7] -
				rhs[8] * rhs[3] * rhs[6];

			inv[11] = -rhs[0] * rhs[5] * rhs[11] +
				rhs[0] * rhs[7] * rhs[9] +
				rhs[4] * rhs[1] * rhs[11] -
				rhs[4] * rhs[3] * rhs[9] -
				rhs[8] * rhs[1] * rhs[7] +
				rhs[8] * rhs[3] * rhs[5];

			inv[15] = rhs[0] * rhs[5] * rhs[10] -
				rhs[0] * rhs[6] * rhs[9] -
				rhs[4] * rhs[1] * rhs[10] +
				rhs[4] * rhs[2] * rhs[9] +
				rhs[8] * rhs[1] * rhs[6] -
				rhs[8] * rhs[2] * rhs[5];

			det = rhs[0] * inv[0] + rhs[1] * inv[4] + rhs[2] * inv[8] + rhs[3] * inv[12];

			if (det == 0) {
				inv.ToIdentity();
				return inv;
			}

			det = 1.f / det;

			for (i = 0; i < 16; i++) {
				inv[i] = inv[i] * det;
			}

			return inv;
		}




	//Handy string output for the matrix. Can get a bit messy, but better than nothing!
	inline friend std::ostream& operator<<(std::ostream& o, const Matrix4& m){
		o << "Mat4(";
		o << "\t"	<< m.values[0] << "," << m.values[1] << "," << m.values[2] << "," << m.values [3] << std::endl;
		o << "\t\t" << m.values[4] << "," << m.values[5] << "," << m.values[6] << "," << m.values [7] << std::endl;
		o << "\t\t" << m.values[8] << "," << m.values[9] << "," << m.values[10] << "," << m.values [11] << std::endl;
		o << "\t\t" << m.values[12] << "," << m.values[13] << "," << m.values[14] << "," << m.values [15] << " )" <<std::endl;
		return o;
	}
};

