#pragma once

#include <nclgl\Mesh.h>
#include <ncltech\Scene.h>
#include <ncltech\CommonUtils.h>

class EmptyScene : public Scene
{
public:
	EmptyScene(const std::string& friendly_name) : Scene(friendly_name) {}
	virtual ~EmptyScene() {}

	virtual void OnInitializeScene()	 override
	{
		Scene::OnInitializeScene();
		this->AddGameObject(CommonUtils::BuildCuboidObject("Ground", Vector3(0.0f, -1.0f, 0.0f), Vector3(20.0f, 1.0f, 20.0f), false, 0.0f, true, false, Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
	}

};