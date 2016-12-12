/******************************************************************************
Class:SceneNode
Implements:
Author:Rich Davison	<richard.davison4@newcastle.ac.uk>
Description:A basic, but functional SceneNode class. SceneNodes have a
transformation matrix, and any number of children. They also have a parent, 
forming a tree structure, with their parent above them, and children below.

They also have a seperate scale for their Mesh, for no other reason than
it made the CubeRobot class easier ;) You might find it useful, though...

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////


#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Mesh.h"
#include <vector>

class SceneNode	{
public:
	 SceneNode(Mesh*m = NULL, Vector4 colour = Vector4(1,1,1,1));
	~SceneNode(void);

	void			SetTransform(const Matrix4 &matrix) { transform = matrix;}
	const Matrix4&	GetTransform() const				{ return transform;}
	Matrix4			GetWorldTransform() const			{ return worldTransform;}

	virtual void	Update(float msec);
	virtual void	Draw(const OGLRenderer & r);

	Vector4			GetColour()		const			{return colour;}
	void			SetColour(const Vector4 &c)		{colour = c;}

	Vector3			GetModelScale()		const			{return modelScale;}
	void			SetModelScale(const Vector3 &s)		{modelScale = s;}

	void			AddChild(SceneNode* s);
	bool			RemoveChild(SceneNode* s,bool recursive = true);

	float			GetBoundingRadius() const	{return boundingRadius;}
	void			SetBoundingRadius(float f)	{boundingRadius = f;}

	float			GetCameraDistance() const	{return distanceFromCamera;}
	void			SetCameraDistance(float f)	{distanceFromCamera = f;}

	void			SetMesh(Mesh*m)				{mesh = m;}
	Mesh*			GetMesh()					{return mesh;}

	bool	IsAwake()	{return awake;}
	void	Wake()		{awake = true;}
	void	Sleep()		{awake = false;} 

	std::vector<SceneNode*>::const_iterator GetChildIteratorStart()	{return children.begin();}
	std::vector<SceneNode*>::const_iterator GetChildIteratorEnd()	{return children.end();}

	static bool		CompareByCameraDistance(SceneNode*a,SceneNode*b) ;
	static bool		CompareByZ(SceneNode*a,SceneNode*b) ;

protected:
	Matrix4		worldTransform;
	Matrix4		transform;
	SceneNode*	parent;
	float		distanceFromCamera;
	float		boundingRadius;
	Vector4		colour;
	Vector3		modelScale;
	Mesh*		mesh;
	bool		awake;
	std::vector<SceneNode*>		children;
};

