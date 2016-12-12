#include "Plane.h"

Plane::Plane(const Vector3 &_normal, float distance, bool normalise) {
	if(normalise) {
		float length = _normal.Length();

		this->_normal   = _normal		/ length;
		this->distance = distance	/ length;
	}
	else{
		this->_normal = _normal;
		this->distance = distance;
	}
}

bool Plane::SphereInPlane(const Vector3 &position, float radius) const {
	if(Vector3::Dot(position,_normal)+distance <= -radius) {
		return false;
	}

	return true;	
}

bool Plane::PointInPlane(const Vector3 &position) const {
	float test = Vector3::Dot(position,_normal);
	float test2 = test + distance;

	if(Vector3::Dot(position,_normal)+distance < -0.0001f) {
		return false;
	}

	return true;
}