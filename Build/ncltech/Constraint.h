/******************************************************************************
Class: Constraint
Implements: 
Author: Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description:
A generic template class to represent a physical constraint. 

A rigid body has 6 degrees of freedom: 3 positional and 3 rotational. A
constraint in this sense is anything which acts to constrain the movement of that
rigid body - usually relative to another existing physical object. In commercial engines,
this seems to most commonly be a defined set of constraints between two objects.
Though there is no reason it cant be more, although if the solver 100% accurate, I guess
it would be the same as using multiple constraints between 2 objects... although that is
a big ask =]

If you are interested, I have listed the pages outlining the constraints available on a
couple of the main commerical physics engines below. Hopefully there is some on there
which you may want to implement yourselves.

	Havok: http://www.codelooker.com/dfilea/112105615910HavocPhysicsEngine/constraintsuserguide.pdf
	(Definitely recomend this one! - even has nice tips to help increase stability within chained constraints)

	Bullet: http://bulletphysics.org/mediawiki-1.5.8/index.php/Constraints
	PhysX: http://docs.nvidia.com/gameworks/content/gameworkslibrary/physx/guide/Manual/Joints.html


		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "PhysicsObject.h"
#include <nclgl\Vector3.h>

class Constraint
{
public:
	Constraint() {}


	// Apply Velocity Impulse to object(s) in order to satisfy given constraint
	//  - Called by PhysicsEngine upon resolving constraints
	virtual void ApplyImpulse() = 0;
	

	// Optional: Pre-solver step will be triggered before any calls to ApplyImpulse
	//			 and only ever be called once per physics timestep
	//  - If you need to precompute any data/velocity forces prior to them changing
	//    through this (or other constraints) then you can do it here.
	virtual void PreSolverStep(float dt) {}


	// Visually Debug Constraint 
	virtual void DebugDraw() const {}
};