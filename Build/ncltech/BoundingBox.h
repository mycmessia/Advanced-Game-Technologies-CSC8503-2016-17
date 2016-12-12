/******************************************************************************
Class: BoundingBox
Implements:
Author: Pieran Marris      <p.marris@newcastle.ac.uk>
Description:
The simplest axis-aligned bounding box implementation you will ever see! 

This serves a single purpose of assisting in the creation of shadow map bounding boxes
for graphics pipeline.

Much better implementations with various different storage methods are out there and I 
highly suggest you go and find them. :)

		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <nclgl\Matrix4.h>
#include <nclgl\Vector3.h>
#include <nclgl\common.h>

struct BoundingBox
{
	Vector3 _min;
	Vector3 _max;


	//Initialize minPoints to max possible value and vice versa to force the first value incorporated to always be used for both min and max points.
	BoundingBox() 
		: _min(FLT_MAX, FLT_MAX, FLT_MAX)
		, _max(-FLT_MAX, -FLT_MAX, -FLT_MAX)
	{}

	//Expand the boundingbox to fit a given point. 
	//  If no points have been set yet, both minPoints and maxPoints will equal the point provided.
	void ExpandToFit(const Vector3& point)
	{
		_min.x = min(_min.x, point.x);
		_min.y = min(_min.y, point.y);
		_min.z = min(_min.z, point.z);
		_max.x = max(_max.x, point.x);
		_max.y = max(_max.y, point.y);
		_max.z = max(_max.z, point.z);
	}

	//Transform the given AABB and returns a new AABB that encapsulates the new rotated bounding box.
	BoundingBox Transform(const Matrix4& mtx)
	{
		BoundingBox bb;
		bb.ExpandToFit(mtx * Vector3(_min.x, _min.y, _min.z));
		bb.ExpandToFit(mtx * Vector3(_max.x, _min.y, _min.z));
		bb.ExpandToFit(mtx * Vector3(_min.x, _max.y, _min.z));
		bb.ExpandToFit(mtx * Vector3(_max.x, _max.y, _min.z));

		bb.ExpandToFit(mtx * Vector3(_min.x, _min.y, _max.z));
		bb.ExpandToFit(mtx * Vector3(_max.x, _min.y, _max.z));
		bb.ExpandToFit(mtx * Vector3(_min.x, _max.y, _max.z));
		bb.ExpandToFit(mtx * Vector3(_max.x, _max.y, _max.z));
		return bb;
	}
};