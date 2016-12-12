/******************************************************************************
Class: SceneManager
Implements: SceneRenderer, TSingleton
Author: Pieran Marris <p.marris@newcastle.ac.uk>
Description:

Extends the SceneRenderer to provide means to have multiple enqueued Scenes that
can be switched between easily. Scenes can be enqueued at the start of the program
by calling SceneManager::Instance()->EnqueueScene(new <MyScene>(<SceneName?>));
If it is the first Scene to be enqueued, it will also instantly become the active scene
shown to the user.

Scenes can be switched between by using one of the JumpToScene methods. This will
call <scene>->OnInitializeScene() and <oldscene>->OnCleanupScene() respectively.

This class is a singleton, so is unique and can be accessed globally.

		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "SceneRenderer.h"

class SceneManager : public SceneRenderer, public TSingleton<SceneManager>
{
	friend class TSingleton < SceneManager >;

public:
	// Add Scene to list of possible scenes to switch to
	void EnqueueScene(Scene* scene);



	//Jump to the next scene in the list or first scene if at the end
	void JumpToScene();

	//Jump to scene index (stored in order they were originally added starting at zero)
	void JumpToScene(int idx);

	//Jump to scene name
	void JumpToScene(const std::string& friendly_name);




	//Get currently active scene (returns NULL if no scenes yet added)
	inline Scene* GetCurrentScene()			{ return m_pScene; }

	//Get currently active scene's index (return 0 if no scenes yet added)
	inline uint   GetCurrentSceneIndex()	{ return m_SceneIdx; }

	//Get total number of enqueued scenes
	inline uint   SceneCount()				{ return m_vpAllScenes.size(); }


protected:
	SceneManager();
	virtual ~SceneManager();

protected:
	uint				m_SceneIdx;
	std::vector<Scene*> m_vpAllScenes;
};