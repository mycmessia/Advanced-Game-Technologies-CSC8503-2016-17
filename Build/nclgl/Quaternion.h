/******************************************************************************
Class:Quaternion
Implements:
Author:Rich Davison
Description:VERY simple Quaternion class. Students are encouraged to modify 
this as necessary...if they dare.

Quaternions aren't really discussed much in the graphics module, but the 
MD5Mesh class uses them internally, as MD5 files store their rotations
as quaternions.

I hate Quaternions.

Pieran Edit: It's not much, though the bug where multiplication was inversed
             has now been fixed, and I have added another function to linearly
			 interpolate between two quaternions (slerp) which may come in
			 handy for networked dead reckoning =]


-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <iostream>
#include "common.h"
#include "Matrix4.h"
#include "Matrix3.h"

class Matrix4;

class Quaternion	{
public:
	Quaternion(void);
	Quaternion(float x, float y, float z, float w);

	~Quaternion(void);

	float x;
	float y;
	float z;
	float w;

	void	Normalise();

	Matrix4 ToMatrix4() const;
	Matrix3 ToMatrix3() const;

	Quaternion	Conjugate() const;
	void		GenerateW();	//builds 4th component when loading in shortened, 3 component quaternions - great for network compression ;)

	static Quaternion EulerAnglesToQuaternion(float pitch, float yaw, float roll);
	static Quaternion AxisAngleToQuaterion(const Vector3& vector, float degrees);

	static void RotatePointByQuaternion(const Quaternion &q, Vector3 &point);

	static Quaternion FromMatrix(const Matrix4 &m);
	static Quaternion FromMatrix(const Matrix3 &m);

	static float Dot(const Quaternion &a, const Quaternion &b);

	Quaternion operator *(const Quaternion &a) const;
	Quaternion operator *(const Vector3 &a) const;

	Quaternion operator+(const Quaternion &a) const {
		return Quaternion(x + a.x, y + a.y, z + a.z, w + a.w);
	}

	Quaternion Interpolate(const Quaternion& pStart, const Quaternion& pEnd, float pFactor);

	inline friend std::ostream& operator<<(std::ostream& o, const Quaternion& q){
		o << "Quat(" << q.x << "," << q.y << "," << q.z <<  "," << q.w << ")" << std::endl;
		return o;
	}
};