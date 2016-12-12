#include "SceneRenderer.h"
#include "PhysicsEngine.h"
#include "NCLDebug.h"
#include "CommonMeshes.h"
#include "ScreenPicker.h"
#include "BoundingBox.h"
 
void SceneRenderer::InitializeOGLContext(Window& parent)
{
	OGLRenderer::InitializeOGLContext(parent);

	InitialiseGL();
}

SceneRenderer::SceneRenderer()
	: OGLRenderer()
	, m_pScene(NULL)
	, m_ScreenFBO(NULL)
	, m_ShadowFBO(NULL)
	, m_ShadowTex(NULL)
	, m_VsyncEnabled(true)
	, m_ShadowMapsInvalidated(true)
	, m_ShadowMapNum(4)
	, m_ShadowMapSize(2048)
	, m_BackgroundColour(0.7f, 0.7f, 0.7f)
	, m_AmbientColour(0.4f, 0.4f, 0.4f)
	, m_InvLightDirection(0.5f, 1.0f, -0.8f)
	, m_SpecularIntensity(64.0f)
	, m_GammaCorrection(1.0f / 2.2f)
{
	m_InvLightDirection.Normalise();

	//We do not have an OGLContext at this point so nothing can be loaded properly, see InitializeOGLContext for all shader/resource loading
	if (!RenderList::AllocateNewRenderList(&m_pFrameRenderList, true))
	{
		NCLERROR("Unable to allocate scene render list! - Try using less shadow maps");
	}

	//Initialize the shadow render lists
	for (uint i = 0; i < m_ShadowMapNum; ++i)
	{
		if (!RenderList::AllocateNewRenderList(&m_apShadowRenderLists[i], true))
		{
			NCLERROR("Unable to allocate shadow render list %d! - Try using less shadow maps", i);
		}
	}
}

SceneRenderer::~SceneRenderer()
{
	if (m_ShadowTex)
	{
		glDeleteTextures(1, &m_ShadowTex);
		glDeleteFramebuffers(1, &m_ShadowFBO);
		m_ShadowTex = NULL;

		for (uint i = 0; i < m_ShadowMapNum; ++i)
			delete m_apShadowRenderLists[i];
	}

	if (m_pFrameRenderList)
	{
		delete m_pFrameRenderList;
		m_pFrameRenderList = NULL;
	}

	if (m_pShaderColNorm)
	{
		delete m_pShaderColNorm;
		delete m_pShaderLightDir;
		delete m_pShaderCombineLighting;
		delete m_pShaderPresentToWindow;
		delete m_pShaderShadow;
		delete m_pShaderForwardLighting;
		m_pShaderColNorm = NULL;
	}

	if (m_pCamera)
	{
		delete m_pCamera;
		m_pCamera = NULL;
	}

	if (m_ScreenTex[0])
	{
		glDeleteTextures(SCREENTEX_MAX, m_ScreenTex);
		glDeleteFramebuffers(1, &m_ScreenFBO);
		m_ScreenTex[0] = NULL;
	}

	if (m_pScene)
	{
		m_pScene->OnCleanupScene();
		delete m_pScene;
		m_pScene = NULL;
	}

	ScreenPicker::Release();
	NCLDebug::ReleaseShaders();
	CommonMeshes::ReleaseMeshes();
}

bool SceneRenderer::InitialiseGL()
{
	CommonMeshes::InitializeMeshes();

	m_pCamera = new Camera();

	BuildFBOs();

	NCLDebug::LoadShaders();

	InitializeDefaults();

	if (!ShadersLoad())
		return false;

	init = true;
	return true;
}

void SceneRenderer::InitializeDefaults()
{
	NCLDebug::ClearLog();
	ScreenPicker::Instance()->ClearAllObjects();

	m_pCamera->SetPosition(Vector3(-3.0f, 10.0f, 15.0f));
	m_pCamera->SetYaw(-10.f);
	m_pCamera->SetPitch(-30.f);

	m_BackgroundColour = Vector3(0.2f, 0.2f, 0.2f);
	m_AmbientColour = Vector3(0.4f, 0.4f, 0.4f);
	m_InvLightDirection = Vector3(0.5f, 1.0f, -0.8f); m_InvLightDirection.Normalise();
	m_SpecularIntensity = 64.0f;

	SetShadowMapNum(4);
	SetShadowMapSize(2048);
	m_NumSuperSamples = 4.0f;
	m_GammaCorrection = 1.0f / 2.2f;
}

void SceneRenderer::SetShadowMapNum(uint num) {
	if (m_ShadowMapNum != num && num <= SHADOWMAP_MAX)
	{
		if (m_ShadowMapNum > 0)
		{
			for (int i = m_ShadowMapNum - 1; i >= (int)num; --i)
			{	
				delete m_apShadowRenderLists[i];
				m_apShadowRenderLists[i] = NULL;				
			}
		}
		for (uint i = m_ShadowMapNum; i < num; i++)
		{		
			RenderList::AllocateNewRenderList(&m_apShadowRenderLists[i], true);
		}
		m_ShadowMapNum = num;
		m_ShadowMapsInvalidated = true;
	}
}

void SceneRenderer::SetShadowMapSize(uint size) {
	if (!m_ShadowMapsInvalidated)
		m_ShadowMapsInvalidated = (size != m_ShadowMapSize);

	m_ShadowMapSize = size;
}

void SceneRenderer::RenderScene()
{
	if (m_pScene == NULL)
		return;

	//Check to see if the window has been resized
	BuildFBOs();
	
	//Reset all varying data
	Mesh::Reset();
	textureMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	viewMatrix = m_pCamera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(PROJ_NEAR, PROJ_FAR, (float)m_ScreenTexWidth / (float)m_ScreenTexHeight, PROJ_FOV);
	m_FrameFrustum.FromMatrix(projMatrix * viewMatrix);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH_CLAMP);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//Update all Object's World Transform
	m_pScene->BuildWorldMatrices();
	NCLDebug::SetDebugDrawData(projMatrix, viewMatrix, m_pCamera->GetPosition());


	//Setup default shader uniforms
	ShadersSetDefaults();

	//Render Shadow Maps
	RenderShadowMaps();

	//Build Scene Render List
	m_pFrameRenderList->UpdateCameraWorldPos(m_pCamera->GetPosition());
	m_pFrameRenderList->RemoveExcessObjects(m_FrameFrustum);
	m_pFrameRenderList->SortLists();
	m_pScene->InsertToRenderList(m_pFrameRenderList, m_FrameFrustum);


	//Use Scene Render List for Picking
	ScreenPicker::Instance()->UpdateFBO(width, height);
	ScreenPicker::Instance()->RenderPickingScene(m_pFrameRenderList, projMatrix, viewMatrix);

	//Main Render Window
	{
		//Render Opaque Objects via deferred rendering (Quicker)
		DeferredRenderOpaqueObjects();

		//Render Transparent Objects via forward rendering - slower but only way :[
		glStencilFunc(GL_ALWAYS, 1, 1);
		ForwardRenderTransparentObjects();

		//Render Debug Data (NCLDebug)
		PhysicsEngine::Instance()->DebugRender();
		NCLDebug::SortDebugLists();
		NCLDebug::DrawDebugLists();	
	}

	//Downsample and present our complete image to the window
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	SetCurrentShader(m_pShaderPresentToWindow);
	glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_COLOUR1]);
	glDrawArrays(GL_POINTS, 0, 1);

	//Finally draw any non-anti aliasing HUD elements
	NCLDebug::DrawDebubHUD();
	NCLDebug::ClearDebugLists();

	//Finally swap buffers and get ready to repeat the whole process
	SwapBuffers();
}

void SceneRenderer::UpdateScene(float dt)
{
	if (m_pScene != NULL)
	{
		if (!ScreenPicker::Instance()->HandleMouseClicks(dt))
			m_pCamera->HandleMouse(dt);

		m_pCamera->HandleKeyboard(dt);
		m_pScene->OnUpdateScene(dt);
	}		
}





void SceneRenderer::RenderShadowMaps()
{
	if (m_pScene != NULL)
	{
		const float proj_range = PROJ_FAR - PROJ_NEAR;
		Matrix4 view = Matrix4::BuildViewMatrix(Vector3(0.0f, 0.0f, 0.0f), m_InvLightDirection);
		Matrix4 invCamProjView = Matrix4::Inverse(projMatrix * viewMatrix);


		auto compute_depth = [&](float x)
		{
			float proj_start = -(proj_range * x + PROJ_NEAR);
			return (proj_start*projMatrix[10] + projMatrix[14]) / (proj_start*projMatrix[11]);
		};

		const float log_factor = 50.0f; //Sub-divide view frustum in log50 increments
		const float log_scalar = 1.0f / logf(log_factor);

		float itr_factor = (log_factor-1.0f) / float(m_ShadowMapNum);

#pragma omp parallel for
		for (int i = 0; i < (int)m_ShadowMapNum; ++i)
		{

			float factor_n = 1.0f - logf(i * itr_factor + 1.0f) * log_scalar;
			float factor_f = 1.0f - logf((i + 1) * itr_factor + 1.0f) * log_scalar;

			float near_depth = compute_depth(factor_n);
			float far_depth = compute_depth(factor_f);

			//Build Bounding Box around frustum section
			BoundingBox bb;
			bb.ExpandToFit(invCamProjView * Vector3(-1.0f, -1.0f, near_depth));
			bb.ExpandToFit(invCamProjView * Vector3(-1.0f, 1.0f, near_depth));
			bb.ExpandToFit(invCamProjView * Vector3(1.0f, -1.0f, near_depth));
			bb.ExpandToFit(invCamProjView * Vector3(1.0f, 1.0f, near_depth));
			bb.ExpandToFit(invCamProjView * Vector3(-1.0f, -1.0f, far_depth));
			bb.ExpandToFit(invCamProjView * Vector3(-1.0f, 1.0f, far_depth));
			bb.ExpandToFit(invCamProjView * Vector3(1.0f, -1.0f, far_depth));
			bb.ExpandToFit(invCamProjView * Vector3(1.0f, 1.0f, far_depth));


			//Rotate bounding box so it's orientated in the lights direction
			Vector3 centre = (bb._max + bb._min) * 0.5f;
			Matrix4 localView = view * Matrix4::Translation(-centre);

			bb = bb.Transform(localView);
			bb._min.z = min(bb._min.z, -m_pScene->GetWorldRadius());
			bb._max.z = max(bb._max.z, m_pScene->GetWorldRadius());

			//Build Light Projection		
			m_ShadowProj[i] = Matrix4::Orthographic(bb._max.z, bb._min.z, bb._min.x, bb._max.x, bb._max.y, bb._min.y);
			m_ShadowProjView[i] = m_ShadowProj[i] * localView;

			//Construct Shadow RenderList 
			Vector3 top_mid = centre + view * Vector3(0.0f, 0.0f, bb._max.z);
			Frustum f; f.FromMatrix(m_ShadowProjView[i]);
			m_apShadowRenderLists[i]->UpdateCameraWorldPos(top_mid);
			m_apShadowRenderLists[i]->RemoveExcessObjects(f);
			m_apShadowRenderLists[i]->SortLists();
			m_pScene->InsertToRenderList(m_apShadowRenderLists[i], f);
		}



		//Render Shadow Maps (OpenGL cannot be multithreaded easily)
		glBindFramebufferEXT(GL_FRAMEBUFFER, m_ShadowFBO);
		glViewport(0, 0, m_ShadowMapSize, m_ShadowMapSize);
		
		SetCurrentShader(m_pShaderShadow);
		GLint uniloc_projviewMatrix = glGetUniformLocation(currentShader->GetProgram(), "uProjViewMtx");
		GLint uniloc_modelMatrix = glGetUniformLocation(currentShader->GetProgram(), "uModelMtx");

		for (uint i = 0; i < m_ShadowMapNum; ++i)
		{
			glUniformMatrix4fv(uniloc_projviewMatrix, 1, false, (float*)&m_ShadowProjView[i]);

			glFramebufferTextureLayer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT, m_ShadowTex, 0, i);
			glClear(GL_DEPTH_BUFFER_BIT);
			
			auto per_obj_render = [&](Object* obj)
			{
				glUniformMatrix4fv(uniloc_modelMatrix, 1, false, (float*)&obj->GetWorldTransform());
				obj->OnRenderObject();
			};
			m_apShadowRenderLists[i]->RenderOpaqueObjects(per_obj_render);
			m_apShadowRenderLists[i]->RenderTransparentObjects(per_obj_render);
		}
	}

}

void SceneRenderer::BuildFBOs()
{
	//Util Function
	auto build_texture = [&](uint texID, GLint internalformat, uint width, uint height, bool depth, bool samplerShadow)
	{
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if (samplerShadow)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
		}

		if (depth)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}
		else
			glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
	};

	//Construct Scene GBuffer
	if (m_ScreenTexWidth != uint(width * m_NumSuperSamples) || m_ScreenTexHeight != uint(height * m_NumSuperSamples))
	{
		m_ScreenTexWidth = uint(width * m_NumSuperSamples);
		m_ScreenTexHeight = uint(height * m_NumSuperSamples);

		if (!m_ScreenTex[0]) glGenTextures(SCREENTEX_MAX, m_ScreenTex);

		//Generate our Scene Depth Texture
		build_texture(m_ScreenTex[SCREENTEX_DEPTH], GL_DEPTH24_STENCIL8, m_ScreenTexWidth, m_ScreenTexHeight, true, false);
		build_texture(m_ScreenTex[SCREENTEX_COLOUR0], GL_SRGB8, m_ScreenTexWidth, m_ScreenTexHeight, false, false);
		build_texture(m_ScreenTex[SCREENTEX_COLOUR1], GL_SRGB8, m_ScreenTexWidth, m_ScreenTexHeight, false, false);
		build_texture(m_ScreenTex[SCREENTEX_NORMALS], GL_RGB16F, m_ScreenTexWidth, m_ScreenTexHeight, false, false);
		build_texture(m_ScreenTex[SCREENTEX_LIGHTING], GL_RG16F, m_ScreenTexWidth, m_ScreenTexHeight, false, false);

		//Generate our Framebuffer
		if (!m_ScreenFBO) glGenFramebuffers(1, &m_ScreenFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_ScreenFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_DEPTH], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_DEPTH], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_COLOUR0], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_COLOUR1], 0);

		//Validate our framebuffer
		GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			NCLERROR("Unable to create Screen Framebuffer! StatusCode: %x", status);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	//Construct our Shadow Maps
	if (m_ShadowMapsInvalidated && m_ShadowMapNum > 0)
	{	
		m_ShadowMapsInvalidated = false;

		if (!m_ShadowTex) glGenTextures(1, &m_ShadowTex);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_ShadowTex);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32, m_ShadowMapSize, m_ShadowMapSize, m_ShadowMapNum);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
		

		if (!m_ShadowFBO) glGenFramebuffers(1, &m_ShadowFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowFBO);
		glFramebufferTextureLayer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT, m_ShadowTex, 0, 0);
		glDrawBuffers(0, GL_NONE);
		GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			NCLERROR("Unable to create Shadow Framebuffer! StatusCode: %x", status);
		}
	}
}


bool SceneRenderer::ShadersLoad()
{
	auto SHADERERROR = [&](const char* name) {
		NCLERROR("Could not link shader: %s", name);
	};

	m_pShaderColNorm = new Shader(
		SHADERDIR"SceneRenderer/TechVertexFull.glsl",
		SHADERDIR"SceneRenderer/TechFragDeferred.glsl");
	if (!m_pShaderColNorm->LinkProgram()){
		SHADERERROR("Default Deferred Render");
		return false;
	}

	m_pShaderLightDir = new Shader(
		SHADERDIR"Common/EmptyVertex.glsl",
		SHADERDIR"SceneRenderer/TechFragDeferredDirLight.glsl",
		SHADERDIR"Common/FullScreenQuadGeometry.glsl");
	if (!m_pShaderLightDir->LinkProgram()){
		SHADERERROR("Deferred Render Light Computation");
		return false;
	}

	m_pShaderCombineLighting = new Shader(
		SHADERDIR"Common/EmptyVertex.glsl",
		SHADERDIR"SceneRenderer/TechFragDeferredCombine.glsl",
		SHADERDIR"Common/FullScreenQuadGeometry.glsl");
	if (!m_pShaderCombineLighting->LinkProgram()){
		SHADERERROR("Deferred Render Light Combination");
		return false;
	}

	m_pShaderPresentToWindow = new Shader(
		SHADERDIR"Common/EmptyVertex.glsl",
		SHADERDIR"SceneRenderer/TechFragSuperSample.glsl",
		SHADERDIR"Common/FullScreenQuadGeometry.glsl");
	if (!m_pShaderPresentToWindow->LinkProgram()){
		SHADERERROR("Present to window / SuperSampling");
		return false;
	}

	m_pShaderShadow = new Shader(
		SHADERDIR"SceneRenderer/TechVertexShadow.glsl",
		SHADERDIR"Common/EmptyFragment.glsl");
	if (!m_pShaderShadow->LinkProgram()){
		SHADERERROR("Shadow Shader");
		return false;
	}

	m_pShaderForwardLighting = new Shader(
		SHADERDIR"SceneRenderer/TechVertexFull.glsl",
		SHADERDIR"SceneRenderer/TechFragForwardRender.glsl");
	if (!m_pShaderForwardLighting->LinkProgram()){
		SHADERERROR("Forward Renderer");
		return false;
	}

	return true;
}

void SceneRenderer::ShadersSetDefaults()
{
	Matrix4 projview = projMatrix * viewMatrix;
	Vector3 camPos = m_pCamera->GetPosition();
	Matrix4 clipspaceToWorldspace = Matrix4::Inverse(projMatrix * viewMatrix);

	SetCurrentShader(m_pShaderColNorm);
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "uProjViewMtx"), 1, false, (float*)&projview);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "uDiffuseTex"), 0);


	SetCurrentShader(m_pShaderLightDir);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "uDepthTex"), 5);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "uColourTex"), 6);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "uNormalTex"), 7);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "uShadowTex"), 8);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "uShadowNum"), m_ShadowMapNum);
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "uShadowSinglePixel"), 1.0f / m_ShadowMapSize, 1.0f / m_ShadowMapSize);
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "uClipToWorldTransform"), 1, GL_FALSE, &clipspaceToWorldspace.values[0]);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "uInvLightDir"), 1, &m_InvLightDirection.x);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "uCameraPos"), 1, &camPos.x);
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "uSpecularIntensity"), m_SpecularIntensity);


	SetCurrentShader(m_pShaderForwardLighting);
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "uProjViewMtx"), 1, false, (float*)&projview);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "uDiffuseTex"), 0);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "uShadowTex"), 8);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "uShadowNum"), m_ShadowMapNum);
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "uShadowSinglePixel"), 1.0f / m_ShadowMapSize, 1.0f / m_ShadowMapSize);
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "uClipToWorldTransform"), 1, GL_FALSE, &clipspaceToWorldspace.values[0]);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "uInvLightDir"), 1, &m_InvLightDirection.x);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "uCameraPos"), 1, &camPos.x);
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "uSpecularIntensity"), m_SpecularIntensity);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "uAmbientColour"), 1, &m_AmbientColour.x);


	SetCurrentShader(m_pShaderCombineLighting);
	glUniform3fv(glGetUniformLocation(currentShader->GetProgram(), "uAmbientColour"), 1, &m_AmbientColour.x);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "uColourTex"), 5);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "uLightingTex"), 6);


	SetCurrentShader(m_pShaderPresentToWindow);
	glUniform2f(glGetUniformLocation(currentShader->GetProgram(), "uSinglepixel"), 1.0f / m_ScreenTexWidth, 1.0f / m_ScreenTexHeight);
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "uNumSamples"), m_NumSuperSamples);
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(), "uGammaCorrection"), 1.0f / m_GammaCorrection);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "uColourTex"), 5);
}

void SceneRenderer::DeferredRenderOpaqueObjects()
{
	const GLenum drawbuffers_1[1] = { GL_COLOR_ATTACHMENT0 };
	const GLenum drawbuffers_2[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };


	//Render the Scene to Depth/Color/Normal textures
	glBindFramebuffer(GL_FRAMEBUFFER, m_ScreenFBO);
	glViewport(0, 0, m_ScreenTexWidth, m_ScreenTexHeight);
	glClearColor(m_BackgroundColour.x, m_BackgroundColour.y, m_BackgroundColour.z, 0.0f);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_COLOUR0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_NORMALS], 0);
	glDrawBuffers(2, drawbuffers_2);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	SetCurrentShader(m_pShaderColNorm);

	glStencilFunc(GL_ALWAYS, 1, 1);
	glStencilOp(GL_ZERO, GL_KEEP, GL_REPLACE);

	GLint uniloc_modelMatrix = glGetUniformLocation(currentShader->GetProgram(), "uModelMtx");
	GLint uniloc_nodeColour = glGetUniformLocation(currentShader->GetProgram(), "uColour");
	m_pFrameRenderList->RenderOpaqueObjects([&](Object* obj)
	{
		glUniformMatrix4fv(uniloc_modelMatrix, 1, false, (float*)&obj->GetWorldTransform());
		if (uniloc_nodeColour > -1) glUniform4fv(uniloc_nodeColour, 1, (float*)&obj->GetColour());
		obj->OnRenderObject();
	});


	//Compute Lighting
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_LIGHTING], 0);
	glDrawBuffers(1, drawbuffers_1);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	SetCurrentShader(m_pShaderLightDir);
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "uShadowTransform[0]"), m_ShadowMapNum, GL_FALSE, &m_ShadowProjView[0].values[0]);
	glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_DEPTH]);
	glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_COLOUR0]);
	glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_NORMALS]);
	glActiveTexture(GL_TEXTURE8 ); glBindTexture(GL_TEXTURE_2D_ARRAY, m_ShadowTex);
	
	glStencilFunc(GL_EQUAL, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glDrawArrays(GL_POINTS, 0, 1);

	//Finally Combine all the colours/lighting/shadows etc into a single texture
	glBindFramebuffer(GL_FRAMEBUFFER, m_ScreenFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_COLOUR1], 0);
	glClear(GL_COLOR_BUFFER_BIT);

	SetCurrentShader(m_pShaderCombineLighting);
	glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_COLOUR0]);
	glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_LIGHTING]);
	glDrawArrays(GL_POINTS, 0, 1);
	
	glEnable(GL_DEPTH_TEST);
}

void SceneRenderer::ForwardRenderTransparentObjects()
{
	//Draw all transparent objects onto scene using forward-rendering
	SetCurrentShader(m_pShaderForwardLighting);
	glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "uShadowTransform[0]"), m_ShadowMapNum, GL_FALSE, &m_ShadowProjView[0].values[0]);
	glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_DEPTH]);
	glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_COLOUR0]);
	glActiveTexture(GL_TEXTURE7); glBindTexture(GL_TEXTURE_2D, m_ScreenTex[SCREENTEX_COLOUR1]);
	glActiveTexture(GL_TEXTURE8); glBindTexture(GL_TEXTURE_2D_ARRAY, m_ShadowTex);

	GLint uniloc_modelMatrix = glGetUniformLocation(currentShader->GetProgram(), "uModelMtx");
	GLint uniloc_nodeColour = glGetUniformLocation(currentShader->GetProgram(), "uColour");
	m_pFrameRenderList->RenderTransparentObjects([&](Object* obj)
	{
		glUniformMatrix4fv(uniloc_modelMatrix, 1, false, (float*)&obj->GetWorldTransform());
		if (uniloc_nodeColour > -1) glUniform4fv(uniloc_nodeColour, 1, (float*)&obj->GetColour());
		glCullFace(GL_FRONT);
		obj->OnRenderObject();

		glCullFace(GL_BACK);
		obj->OnRenderObject();
	});
}