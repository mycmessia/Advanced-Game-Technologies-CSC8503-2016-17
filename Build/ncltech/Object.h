/******************************************************************************
Class: Object
Implements:
Author: Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description: 

This is the base class for all of the objects in your game/scenes. It
will automatically be managed by any Scene instance which it is added to,
firing OnRenderObject() and OnUpdateObject(float dt) each frame.

It can also optionally be attached to a PhysicsObject component which will
automatically update the object's world transform based off it's physical
position/orientation each frame.



		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <nclgl\Matrix4.h>
#include "PhysicsObject.h"
#include <vector>

class Scene;
class PhysicsEngine;

class Object
{

	friend class Scene;			//Can set the private variables m_WorldTransform and m_pScene 
	friend class PhysicsEngine;	//Can notionally set private variable m_pPhysicsObject to NULL if it was deleted elsewhere

public:
	Object(const std::string& name = "");
	virtual ~Object();


//<---------- PHYSICS ------------>
	//This function creates a new physics node for the object in question.
	// - MUST be called before setting any parameters with Physics()
	void CreatePhysicsNode();

	//Returns true if this object has a physicsObject attached
	bool HasPhysics() { return (m_pPhysicsObject != NULL); }

	//Gets a pointer to this objects physicsObject (or NULL if none set)
	PhysicsObject*		Physics() { return m_pPhysicsObject; }


//<--------- SCENE TREE ---------->
	//Get a list of all child scene-tree objects
	std::vector<Object*>& GetChildren()		{ return m_vpChildren; }

	//Recursively search this and all children for an object with the given name (returns NULL if none found)
	Object*				FindGameObject(const std::string& name);

	//Add a child object to this scene node
	void				AddChildObject(Object* child);



//---------- SOUND (?) ------------>







//<------- Object Parameters ------>
	//Get the name of this object (if set)
	const std::string&	GetName()			{ return m_Name; }

	//Returns if this object should be treated as opaque or transparent
	//	- Should be overrided if the object's transparency is not based on it's colour
	virtual bool	IsOpaque()					{ return m_Colour.w >= 0.999f; }

	//Gets/Sets object colour
	void			SetColour(const Vector4& colour)	{ m_Colour = colour; }
	const Vector4&	GetColour()							{ return m_Colour; }

	//Gets/Sets object bounding radius
	//	- Used solely for graphics frustum culling, and should be set for each object
	void			SetBoundingRadius(float radius)		{ m_BoundingRadius = radius; }
	float			GetBoundingRadius()					{ return m_BoundingRadius; }

	//Gets/Sets the local transform
	//	- This will be multiplied by the transform provided by the physicsObject to form
	//	  the world transform used for rendering. If no physicsObject exists, this contains
	//	  the position of the object relevant to it's parent.
	void			SetLocalTransform(const Matrix4& transform)			{ m_LocalTransform = transform; }
	const Matrix4&  GetLocalTransform()									{ return m_LocalTransform; }

	//Get the final world transform of the object, this is recalculated each render pass
	// - This is premultiplied by the objects parent (if any), it's physics transform (if any) and local transform in that order.
	const Matrix4&  GetWorldTransform()					{ return m_WorldTransform; }


	//Get the frustum flags used to identify which RenderList's this object is currently visible within
	uint&			GetFrustumCullFlags()				{ return m_FrustumCullFlags; }

	//Get the screen picker unique ID used to identify where the object exists (if it exsists) within the screen picker array
	uint&			GetScreenPickerIdx()				{ return m_ScreenPickerIdx; }

public:
	//<------ OVERRIDABLE FUNCTIONALITY ---->
	//Called when the object should be renderered
	//	- This can be called multiple times per screen render, or none at all if the object is not in any visible view frustum
	//	  so please don't put any game logic in here.
	virtual void OnRenderObject() {};

	//Called when the object should be updated
	//	- This is called once per frame regardless of screen-visibility and should handle all object logic/AI
	virtual void OnUpdateObject(float dt)		{};	


//<----- MOUSE INTERACTIVITY --------->
//	- To Enable mouse interactivity the object must call "ScreenPicker::Instance()->RegisterObject(this)"
//    in the constructor (or as required). It renders the object to the screen via OnRenderObject() so
//    will only recieve mouse interactivity callbacks if it renders something to the screen.

	//Called when the mouse cursor hovers over the obejct
	virtual void OnMouseEnter(float dt)			{};

	//Called when the mouse cursor stops hovering over the object
	virtual void OnMouseLeave(float dt)			{};

	//Called when the user clicks on the object
	virtual void OnMouseDown(float dt, const Vector3& worldPos)									{};

	//Called each frame the user has clicked (but not released) the object
	virtual void OnMouseMove(float dt, const Vector3& worldPos, const Vector3& worldChange)		{};

	//Called when the user releases the mouse after previously clicking
	virtual void OnMouseUp(float dt, const Vector3& worldPos)									{};

protected:
	//Scene Tree 
	std::string					m_Name;
	Scene*						m_pScene;
	Object*						m_pParent;
	std::vector<Object*>		m_vpChildren;

	//Physics
	PhysicsObject*				m_pPhysicsObject;

	//Sound(?)




	//Object Parameters
	Vector4						m_Colour;
	float						m_BoundingRadius;	
	Matrix4						m_LocalTransform;
	Matrix4						m_WorldTransform;

	//Misc Parameters
	uint						m_FrustumCullFlags;
	uint						m_ScreenPickerIdx;  
};