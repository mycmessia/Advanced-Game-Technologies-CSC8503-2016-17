/******************************************************************************
Class:CubeRobot
Implements:SceneNode
Author:Rich Davison	<richard.davison4@newcastle.ac.uk>
Description:A cuboid robot, created from a small sub-scenegraph. Root node
(ie 'this') is a transition node, which has a 'body' node as a child, which
in turn has 2 legs, 2 arms, and a head as child nodes. 

The tutorial notes suggest that CreateCube should be called by the student, 
but the constructor will actually call the function if necessary. 

Programs should still call DeleteCube on exit, to prevent memory leaks. 

STUDENT CHALLENGE! How would you re-engineer this class to automatically call
DeleteCube if all CubeRobots have been deleted?

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "SceneNode.h"
#include "OBJMesh.h"
#include "common.h"

class CubeRobot : public SceneNode	{
public:
	CubeRobot(void);
	virtual ~CubeRobot(void){};

	//Overloaded from SceneNode, makes the CubeRobot dance!
	virtual void	Update(float msec);

	//CubeRobot relies on their being a cube Mesh - this creates that Mesh.
	static void	CreateCube() {
		OBJMesh*m = new OBJMesh();
		m->LoadOBJMesh(MESHDIR"cube.obj");
		cube = m; 
	}

	//Deletes the CubeRobot cube Mesh.
	static void DeleteCube(){
		delete cube;
	}

protected:
	static Mesh* cube;
	//These are just handy pointers to the limbs, to make animating them easier
	SceneNode*head;

	SceneNode*leftArm;
	SceneNode*rightArm;

	SceneNode*leftLeg;
	SceneNode*rightLeg;
};

