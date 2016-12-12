/******************************************************************************
Class: CollisionDetectionSAT
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

This wrapper class is used to perform a Seperating Axis Therorem test between
two convex collision shapes, firstly detecting if a collision occured and 
then more throughly building a collision manifold if required.

As part of Tutorial 4/5 we will be covering this in detail and building it 
up ourselves. 

The additional functionality provided here are geometric functions used in the 
aforementioned tutorials to perform collision detection. I will try and detail
them below:
	GetClosestPoint(<point> A, <edges>)
	 - Iterates through all edges returning the the point X which is the closest
	   point along any of the given edges to the provided point A as possible.
	   - Used in SAT detection phase

	PlaneEdgeIntersection(<plane>, <edge>[start, end])
	  - Performs a plane/edge collision test, if an intersection does occur then
	    it will return the point on the line where it intersected the given plane.
		- Used in Sutherland-Hodgman Clipping (below)

	SutherlandHodgmanClipping(<mesh>, <clip_planes>)
	  - Performs sutherland hodgeson clipping algorithm to clip the provided mesh
	    or polygon in regards to each of the provided clipping planes. For more
		information see:
		https://en.wikipedia.org/wiki/Sutherland%E2%80%93Hodgman_algorithm
		Edit: Yes.. there may have been a typo in the notes. It is 'hodgman'
		      not hodgeson, apologies.
		 - Used to build collision manifold around instance and reference
		   faces.

		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "PhysicsObject.h"
#include "CollisionShape.h"
#include "Manifold.h"

struct CollisionData
{
	//The direction of collision from obj1 to obj2
	Vector3		_normal;

	//The amount the objects penetrate eachother (negative overlap distance)
	float		_penetration;

	//The point on obj1 where they overlap
	Vector3		_pointOnPlane;
};

class CollisionDetectionSAT
{
public:
	CollisionDetectionSAT();

	//Start processing new (possible) collision pair
	// - Clear all previous collision data
	void BeginNewPair(
		PhysicsObject* obj1,
		PhysicsObject* obj2,
		CollisionShape* shape1,
		CollisionShape* shape2);

	// Seperating-Axis-Theorem
	// - Returns true if the objects are colliding or false otherwise
	bool AreColliding(CollisionData* out_coldata = NULL);

	// Clipping Method
	// - Uses clipping to construct a manifold describing the surface area
	//   of the collision region
	void GenContactPoints(Manifold* out_manifold);
	
protected:
//<---- SAT ---->
	//Add a new possible colliding axis
	// - This only inserts the axis if it is not a duplicate of pre-existing collision axis
	bool AddPossibleCollisionAxis(Vector3 axis);

	// This will build a list of all possibly colliding axes between the two objects
	void FindAllPossibleCollisionAxes();

	// This will evaluate the given axis working out if the the two objects
	// are indeed colliding in this direction.
	bool CheckCollisionAxis(const Vector3& axis, CollisionData* coldata);
	

	
//<---- UTILS ---->

	// Iterates through all edges returning the the point X which is the closest
	//   point along any of the given edges to the provided point A as possible.
	Vector3 GetClosestPoint(
		const Vector3& pos,
		std::vector<CollisionEdge>& edges);


	// Performs a plane/edge collision test, if an intersection does occur then
	//    it will return the point on the line where it intersected the given plane.
	Vector3 PlaneEdgeIntersection(
		const Plane& plane,
		const Vector3& start,
		const Vector3& end) const;

	//Performs sutherland hodgeson clipping algorithm to clip the provided mesh
	//    or polygon in regards to each of the provided clipping planes.
	void SutherlandHodgmanClipping(
		const std::list<Vector3>& input_polygon,
		int num_clip_planes,
		const Plane* clip_planes,
		std::list<Vector3>* out_polygon,
		bool removeNotClipToPlane) const;

private:
	const PhysicsObject*	m_pObj1;
	const PhysicsObject*	m_pObj2;
	const CollisionShape*	m_pShape1;
	const CollisionShape*	m_pShape2;

	std::vector<Vector3>	m_vPossibleCollisionAxes;

	bool					m_Colliding;
	CollisionData			m_BestColData;
};