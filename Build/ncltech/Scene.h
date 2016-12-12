/******************************************************************************
Class: Scene
Implements: 
Author: Pieran Marris <p.marris@newcastle.ac.uk>
Description: 

The Scene class is an extrapolation of the Scene Management tutorial 
from Graphics for Games module. It contains a SceneTree of Objects which are automatically
Culled, Rendered and Updated as needed during runtime.

With the addition of the SceneManager class, multiple scenes can cohexist within the same
program meaning the same Scene could be initialied/cleaned up multiple times. The standard procedure
for a Scene lifespan follows:-
	1. Constructor()		 [Program Start]
	2. OnInitializeScene()	 [Scene Focus]
	3. OnCleanupScene()		 [Scene Lose Focus]
	4. Deconsructor()		 [Program End]

Once an object is added to the scene via AddGameObject(), the object is managed by the Scene. 
This means that it will automatically call delete on any objects you have added when the scene 
becomes innactive (lose focus). To override this you will need to override the OnCleanupScene method
and handle cleanup of Objects yourself.


		(\_/)								
		( '_')								
	 /""""""""""""\=========     -----D		
	/"""""""""""""""""""""""\			
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <nclgl/OGLRenderer.h>
#include <nclgl/Camera.h>
#include <nclgl/Shader.h>
#include <nclgl/Frustum.h>

#include "TSingleton.h"
#include "Object.h"
#include "RenderList.h"


class Scene
{
public:
	Scene(const std::string& friendly_name); //Called once at program start - all scene initialization should be done in 'OnInitialize'
	~Scene();

	// Called when scene is being activated, and will begin being rendered/updated. 
	//	 - Initialize objects/physics here
	virtual void OnInitializeScene()	{}		

	// Called when scene is being swapped and will no longer be rendered/updated 
	//	 - Remove objects/physics here
	//	   Note: Default action here automatically delete all game objects
	virtual void OnCleanupScene()		{ DeleteAllGameObjects(); };	

	// Update Scene Logic
	//   - Called once per frame and should contain all time-sensitive update logic
	//	   Note: This is time relative to seconds not milliseconds! (e.g. msec / 1000)
	virtual void OnUpdateScene(float dt);								

	// Delete all contained Objects
	//    - This is the default action upon firing OnCleanupScene()
	void DeleteAllGameObjects(); 

	// Add GameObject to the scene list
	//    - All added game objects are managed by the scene itself, firing
	//		OnRender and OnUpdate functions automatically
	void AddGameObject(Object* game_object);


	// Simple recursive search
	//   - Searches all Objects in the tree and returns the first one with the name specified
	//     or NULL if none can be found.
	Object* FindGameObject(const std::string& name);

	// The friendly name associated with this scene instance
	const std::string& GetSceneName() { return m_SceneName; }
	

	// The maximum bounds of the contained scene
	//   - This is exclusively used for shadowing purposes, ensuring all objects that could
	//     cast shadows are rendered as necessary.
	void  SetWorldRadius(float radius)	{ m_pRootGameObject->SetBoundingRadius(radius); }
	float GetWorldRadius()				{ return m_pRootGameObject->GetBoundingRadius(); }


	// Adds all visible objects to given RenderList
	void InsertToRenderList(RenderList* list, const Frustum& frustum);

	// Updates all world transforms in the Scene Tree
	void BuildWorldMatrices();

protected:

	// Recursive function called via 'BuildWorldMatrices'
	void	UpdateWorldMatrices(Object* node, const Matrix4& parentWM);

	// Recusive function called via 'InsertToRenderList'
	void	InsertToRenderList(Object* node, RenderList* list, const Frustum& frustum);

	// Recusive function called via 'OnUpdateScene'
	void	UpdateNode(float dt, Object* cNode);

protected:
	std::string			m_SceneName;
	Object*				m_pRootGameObject;
};