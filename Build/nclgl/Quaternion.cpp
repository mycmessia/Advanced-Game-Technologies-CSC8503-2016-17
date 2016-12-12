#include "Quaternion.h"

Quaternion::Quaternion(void)
{
	x = y = z = 0.0f;
	w = 1.0f;
}

Quaternion::Quaternion(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

Quaternion::~Quaternion(void)
{
}

float Quaternion::Dot(const Quaternion &a, const Quaternion &b){
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

void Quaternion::Normalise(){
	float magnitude = sqrt(Dot(*this, *this));

	if (magnitude > 0.0f){
		float t = 1.0f / magnitude;

		x *= t;
		y *= t;
		z *= t;
		w *= t;
	}
}


Quaternion Quaternion::operator *(const Quaternion &b) const{
	Quaternion ans;

	ans.w = (w * b.w) - (x * b.x) - (y * b.y) - (z * b.z);
	ans.x = (x * b.w) + (w * b.x) + (y * b.z) - (z * b.y);
	ans.y = (y * b.w) + (w * b.y) + (z * b.x) - (x * b.z);
	ans.z = (z * b.w) + (w * b.z) + (x * b.y) - (y * b.x);

	return ans;
}

Quaternion Quaternion::operator *(const Vector3 &b) const{
	/*Quaternion ans;

	ans.w = -(x * b.x) - (y * b.y) - (z * b.z);

	ans.x =  (w * b.x) + (y * b.z) - (z * b.y);
	ans.y =  (w * b.y) + (z * b.x) - (x * b.z);
	ans.z =  (w * b.z) + (x * b.y) - (y * b.x);

	return ans;*/

	//This (^) is equiv to q * b, where the below is equiv to b * q (needed for physics)

	Quaternion ans;

	ans.w = -(x * b.x) - (y * b.y) - (z * b.z);

	ans.x = (w * b.x) + (b.y * z) - (b.z * y);
	ans.y = (w * b.y) + (b.z * x) - (b.x * z);
	ans.z = (w * b.z) + (b.x * y) - (b.y * x);

	return ans;
}

Matrix4 Quaternion::ToMatrix4() const{
	Matrix4 mat;

	float yy = y*y;
	float zz = z*z;
	float xy = x*y;
	float zw = z*w;
	float xz = x*z;
	float yw = y*w;
	float xx = x*x;
	float yz = y*z;
	float xw = x*w;

	mat.values[0] = 1 - 2 * yy - 2 * zz;
	mat.values[1] = 2 * xy + 2 * zw;
	mat.values[2] = 2 * xz - 2 * yw;

	mat.values[4] = 2 * xy - 2 * zw;
	mat.values[5] = 1 - 2 * xx - 2 * zz;
	mat.values[6] = 2 * yz + 2 * xw;

	mat.values[8] = 2 * xz + 2 * yw;
	mat.values[9] = 2 * yz - 2 * xw;
	mat.values[10] = 1 - 2 * xx - 2 * yy;

	return mat;
}

Matrix3 Quaternion::ToMatrix3() const{
	Matrix3 mat;

	float yy = y*y;
	float zz = z*z;
	float xy = x*y;
	float zw = z*w;
	float xz = x*z;
	float yw = y*w;
	float xx = x*x;
	float yz = y*z;
	float xw = x*w;

	mat.mat_array[0] = 1 - 2 * yy - 2 * zz;
	mat.mat_array[1] = 2 * xy + 2 * zw;
	mat.mat_array[2] = 2 * xz - 2 * yw;

	mat.mat_array[3] = 2 * xy - 2 * zw;
	mat.mat_array[4] = 1 - 2 * xx - 2 * zz;
	mat.mat_array[5] = 2 * yz + 2 * xw;

	mat.mat_array[6] = 2 * xz + 2 * yw;
	mat.mat_array[7] = 2 * yz - 2 * xw;
	mat.mat_array[8] = 1 - 2 * xx - 2 * yy;

	return mat;
}

Quaternion Quaternion::EulerAnglesToQuaternion(float pitch, float yaw, float roll)	{
	float y2 = (float)DegToRad(yaw / 2.0f);
	float p2 = (float)DegToRad(pitch / 2.0f);
	float r2 = (float)DegToRad(roll / 2.0f);


	float cosy = (float)cos(y2);
	float cosp = (float)cos(p2);
	float cosr = (float)cos(r2);

	float siny = (float)sin(y2);
	float sinp = (float)sin(p2);
	float sinr = (float)sin(r2);

	Quaternion q;


	q.x = cosr * sinp * cosy + sinr * cosp * siny;
	q.y = cosr * cosp * siny - sinr * sinp * cosy;
	q.z = sinr * cosp * cosy - cosr * sinp * siny;
	q.w = cosr * cosp * cosy + sinr * sinp * siny;

	return q;
};

Quaternion Quaternion::AxisAngleToQuaterion(const Vector3& vector, float degrees)	{
	float theta = (float)DegToRad(degrees);
	float result = (float)sin(theta / 2.0f);

	Quaternion q = Quaternion((float)(vector.x * result), (float)(vector.y * result), (float)(vector.z * result), (float)cos(theta / 2.0f));
	q.Normalise();
	return q;
}

void Quaternion::GenerateW()	{
	w = 1.0f - (x*x) - (y*y) - (z*z);
	if (w < 0.0f) {
		w = 0.0f;
	}
	else{
		w = -sqrt(w);
	}
}

Quaternion Quaternion::Conjugate() const
{
	return Quaternion(-x, -y, -z, w);
}

Quaternion Quaternion::FromMatrix(const Matrix4 &m)	{
	Quaternion q;

	q.w = sqrt(max(0.0f, (1.0f + m.values[0] + m.values[5] + m.values[10]))) / 2;
	q.x = sqrt(max(0.0f, (1.0f + m.values[0] - m.values[5] - m.values[10]))) / 2;
	q.y = sqrt(max(0.0f, (1.0f - m.values[0] + m.values[5] - m.values[10]))) / 2;
	q.z = sqrt(max(0.0f, (1.0f - m.values[0] - m.values[5] + m.values[10]))) / 2;

	q.x = (float)_copysign(q.x, m.values[9] - m.values[6]);
	q.y = (float)_copysign(q.y, m.values[2] - m.values[8]);
	q.z = (float)_copysign(q.z, m.values[4] - m.values[1]);

	return q;
}

Quaternion Quaternion::Interpolate(const Quaternion& start, const Quaternion& end, float factor)
{
	//Clamp interpolation between start and end
	factor = min(max(factor, 0.0f), 1.0f);

	// Calc cos theta (Dot product)
	float cos_theta = Quaternion::Dot(start, end);

	// Quaternions can describe any rotation positively or negatively, however to interpolate
	// correctly we need /both/ quaternions to use the same coordinate system
	Quaternion real_end = end;
	if (cos_theta < 0.0f)
	{
		cos_theta = -cos_theta;
		real_end.x = -end.x;
		real_end.y = -end.y;
		real_end.z = -end.z;
		real_end.w = -end.w;
	}

	// Calculate interpolation coefficients
	float theta = acosf(cos_theta);			// extract theta from dot product's cos(theta)
	float inv_sin_theta = sinf(theta);		// compute inverse rotation length 1.0f / sin(theta)

	if (fabs(inv_sin_theta) < 1e-6f) inv_sin_theta = 1e-6f;
	inv_sin_theta = 1.0f / inv_sin_theta;

	float factor_a = sinf((1.0f - factor) * theta) * inv_sin_theta;
	float factor_b = sinf(factor * theta) * inv_sin_theta;


	// Interpolate the two quaternions
	Quaternion out;
	out.x = factor_a * start.x + factor_b * real_end.x;
	out.y = factor_a * start.y + factor_b * real_end.y;
	out.z = factor_a * start.z + factor_b * real_end.z;
	out.w = factor_a * start.w + factor_b * real_end.w;

	return out;
}