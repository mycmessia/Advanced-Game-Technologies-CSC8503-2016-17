/******************************************************************************
Class: CollisionShape
Implements:
Author: Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description: 

A Generic template for all the functionality needed to represent a convex collision shape. 

This will be the only thing in the physics engine that defines the geometric shape of the
attached PhysicsObject. It provides a means for computing the interia tensor (rotational mass)
and a means to calculate collisions with other unknown collision shapes via CollisionDetectionSAT.

        (\_/)
        ( '_')
     /""""""""""""\=========     -----D
    /"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Hull.h"

#include <nclgl\Vector3.h>
#include <nclgl\Plane.h>
#include <vector>
#include <list>

class PhysicsObject;

struct CollisionEdge
{
	CollisionEdge(const Vector3& a, const Vector3& b) 
		: _v0(a), _v1(b) {}

	Vector3 _v0;
	Vector3 _v1;
};

class CollisionShape
{
public:
	CollisionShape()	{}
	virtual ~CollisionShape()	{}

	// Constructs an inverse inertia matrix of the given collision volume. This is the equivilant of the inverse mass of an object for rotation,
	//   a good source for non-inverse inertia matricies can be found here: https://en.wikipedia.org/wiki/List_of_moments_of_inertia
	virtual Matrix3 BuildInverseInertia(float invMass) const = 0;

	// Draws this collision shape to the debug renderer
	virtual void DebugDraw(const PhysicsObject* currentObject) const = 0;



//<----- USED BY COLLISION DETECTION ----->
	// Get all possible collision axes
	//	- This is a list of all the face normals ignoring any duplicates and parallel vectors.
	virtual void GetCollisionAxes(
		const PhysicsObject* currentObject,
		std::vector<Vector3>* out_axes) const = 0;

	// Get all shape Edges
	//	- Returns a list of all edges AB that form the convex hull of the collision shape. These are
	//    used to check edge/edge collisions aswell as finding the closest point to a sphere. */
	virtual void GetEdges(
		const PhysicsObject* currentObject,
		std::vector<CollisionEdge>* out_edges) const = 0;


	// Get the min/max vertices along a given axis
	virtual void GetMinMaxVertexOnAxis(
		const PhysicsObject* currentObject,
		const Vector3& axis, Vector3* out_min,
		Vector3* out_max) const = 0;


	// Get all data needed to build manifold
	//	- Computes the face that is closest to parallel to that of the given axis,
	//    returning the face (as a list of vertices), face normal and the planes
	//    of all adjacent faces in order to clip against.
	virtual void GetIncidentReferencePolygon(const PhysicsObject* currentObject,
		const Vector3& axis,
		std::list<Vector3>* out_face,
		Vector3* out_normal,
		std::vector<Plane>* out_adjacent_planes) const = 0;

};