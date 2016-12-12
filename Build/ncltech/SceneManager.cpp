#include "SceneManager.h"
#include "NCLDebug.h"
#include "PhysicsEngine.h"

SceneManager::SceneManager() 
	: SceneRenderer()
	, m_SceneIdx(NULL)
{

}

SceneManager::~SceneManager()
{
	m_SceneIdx = 0;
	for (Scene* scene : m_vpAllScenes)
	{
		if (scene != m_pScene)
		{
			scene->OnCleanupScene();
			delete scene;
		}
	}
	m_vpAllScenes.clear();
}


void SceneManager::EnqueueScene(Scene* scene)
{
	if (scene == NULL)
	{
		NCLERROR("Attempting to enqueue NULL scene");
		return;
	}

	m_vpAllScenes.push_back(scene);

	//If this was the first scene, activate it immediately
	if (m_vpAllScenes.size() == 1)
		JumpToScene(0);
	else
		Window::GetWindow().SetWindowTitle("NCLTech - [%d/%d] %s", m_SceneIdx + 1, m_vpAllScenes.size(), m_pScene->GetSceneName().c_str());
}

void SceneManager::JumpToScene()
{
	JumpToScene((m_SceneIdx + 1) % m_vpAllScenes.size());
}

void SceneManager::JumpToScene(int idx)
{
	if (idx < 0 || idx >= (int)m_vpAllScenes.size())
	{
		NCLERROR("Invalid Scene Index: %d", idx);
		return;
	}

	//Clear up old scene
	if (m_pScene)
	{
		PhysicsEngine::Instance()->RemoveAllPhysicsObjects();

		m_pFrameRenderList->RemoveAllObjects();

		for (uint i = 0; i < m_ShadowMapNum; ++i)
			m_apShadowRenderLists[i]->RemoveAllObjects();

		m_pScene->OnCleanupScene();
	}

	m_SceneIdx = idx;
	m_pScene = m_vpAllScenes[idx];

	//Initialize new scene
	PhysicsEngine::Instance()->SetDefaults();
	InitializeDefaults();
	m_pScene->OnInitializeScene();
	Window::GetWindow().SetWindowTitle("NCLTech - [%d/%d] %s", idx + 1, m_vpAllScenes.size(), m_pScene->GetSceneName().c_str());
}

void SceneManager::JumpToScene(const std::string& friendly_name)
{
	bool found = false;
	uint idx = 0;
	for (uint i = 0; found == false && i < m_vpAllScenes.size(); ++i)
	{
		if (m_vpAllScenes[i]->GetSceneName() == friendly_name)
		{
			found = true;
			idx = i;
			break;
		}
	}

	if (found)
	{
		JumpToScene(idx);
	}
	else
	{
		NCLERROR("Unknown Scene Alias: \"%s\"", friendly_name.c_str());
	}
}