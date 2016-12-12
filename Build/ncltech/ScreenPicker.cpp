#include "ScreenPicker.h"
#include "NCLDebug.h"
#include "Scene.h"

ScreenPicker::ScreenPicker()
	: m_pCurrentlyHeldObject(NULL)
	, m_pCurrentlyHoverObject(NULL)
	, m_TexWidth(0)
	, m_TexHeight(0)
	, m_PickerFBO(NULL)
	, m_PickerRB(NULL)
	, m_PickerDepthRB(NULL)
	, m_pShaderPicker(NULL)
{
}

ScreenPicker::~ScreenPicker()
{
	m_pCurrentlyHeldObject = NULL;
	m_pCurrentlyHoverObject = NULL;

	if (m_PickerRB)
	{
		glDeleteRenderbuffers(1, &m_PickerRB);
		glDeleteRenderbuffers(1, &m_PickerDepthRB);
		glDeleteFramebuffers(1, &m_PickerFBO);
		m_PickerRB = NULL;
	}

	if (m_pShaderPicker)
	{
		delete m_pShaderPicker;
		m_pShaderPicker = NULL;
	}
}

void ScreenPicker::RegisterObject(Object* obj)
{
	if (m_AllRegisteredObjects.size() < MAX_PICKABLE_OBJECTS)
	{
		m_AllRegisteredObjects.push_back(obj);
		obj->GetScreenPickerIdx() = m_AllRegisteredObjects.size();
	}
	else
	{
		NCLERROR("MAX SCREEN PICKER ITEM COUNT REACHED!");
	}
}

void ScreenPicker::UnregisterObject(Object* obj)
{
	auto loc = std::find(m_AllRegisteredObjects.begin(), m_AllRegisteredObjects.end(), obj);

	if (loc != m_AllRegisteredObjects.end())
	{
		obj->GetScreenPickerIdx() = 0;
		m_AllRegisteredObjects.erase(loc);
	}

	//Iterate through all remaining objects and update their indices
#pragma omp parallel for
	for (int i = 0; i < (int)m_AllRegisteredObjects.size(); ++i)
	{
		m_AllRegisteredObjects[i]->GetScreenPickerIdx() = i + 1;
	}
}

void ScreenPicker::UpdateFBO(int screen_width, int screen_height)
{
	//Have to load the shader here incase the screenpicker constructor is called before we have an OGL context
	if (m_pShaderPicker == NULL)
	{
#ifdef USE_NSIGHT_HACK
		m_pShaderPicker = new Shader(SHADERDIR"SceneRenderer/TechVertexShadow.glsl", SHADERDIR"SceneRenderer/TechFragScreenPicker_nsightfix.glsl");
#else
		m_pShaderPicker = new Shader(SHADERDIR"SceneRenderer/TechVertexShadow.glsl", SHADERDIR"SceneRenderer/TechFragScreenPicker.glsl");
#endif
		glBindFragDataLocation(m_pShaderPicker->GetProgram(), 0, "OutFrag");
		if (!m_pShaderPicker->LinkProgram())
		{
			NCLERROR("Unable to build ScreenPicker Shader!");
		}
	}

	if (m_TexWidth != screen_width || m_TexHeight != screen_height)
	{
		//Build Textures
		m_TexWidth = screen_width;
		m_TexHeight = screen_height;

		if (!m_PickerRB) glGenRenderbuffers(1, &m_PickerRB);
		glBindRenderbuffer(GL_RENDERBUFFER, m_PickerRB);
#ifdef USE_NSIGHT_HACK
		glRenderbufferStorage(GL_RENDERBUFFER, GL_R32F, m_TexWidth, m_TexHeight);
#else
		glRenderbufferStorage(GL_RENDERBUFFER, GL_R16UI, m_TexWidth, m_TexHeight);
#endif
		if (!m_PickerDepthRB) glGenRenderbuffers(1, &m_PickerDepthRB);
		glBindRenderbuffer(GL_RENDERBUFFER, m_PickerDepthRB);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_TexWidth, m_TexHeight);

		//Build FBO
		if (!m_PickerFBO) glGenFramebuffers(1, &m_PickerFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_PickerFBO);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_PickerRB);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_PickerDepthRB);

		//Validate our framebuffer
		GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			
			NCLERROR("Unable to create ScreenPicker Framebuffer! StatusCode: %x", status);
		}
	}
}

bool ScreenPicker::HandleMouseClicks(float dt)
{
	if (m_AllRegisteredObjects.size() > 0 && m_TexWidth > 0 && m_TexHeight > 0)
	{
		Vector2 mousepos;
		bool mouseInWindow = Window::GetWindow().GetMouseScreenPos(&mousepos);
		mousepos.y = m_TexHeight - mousepos.y; //Flip Y as opengl uses bottom left as origin

		Vector3 clipspacepos = Vector3(
			mousepos.x / (m_TexWidth * 0.5f) - 1.0f,
			mousepos.y / (m_TexHeight * 0.5f) - 1.0f,
			0.0f);

		bool mouseDown = Window::GetMouse()->ButtonDown(MOUSE_LEFT);
		bool mouseHeld = Window::GetMouse()->ButtonHeld(MOUSE_LEFT);

		//Do we have an object already being dragged?
		if (m_pCurrentlyHeldObject != NULL)
		{
			//Is the object still being dragged?
			if (!mouseDown || !mouseInWindow)
			{
				HandleObjectMouseUp(dt, mouseInWindow, clipspacepos);
			}
			else
			{
				HandleObjectMouseMove(dt, clipspacepos);				
			}

			return true;
		}
		else if (m_PickerFBO != NULL)
		{
			//Are we hovering over an object?
			uint pixelIdx = 0;

			glBindFramebuffer(GL_FRAMEBUFFER, m_PickerFBO);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
#ifdef USE_NSIGHT_HACK
			float pixelIdxf = 0.0f;
			glReadPixels((int)mousepos.x, (int)mousepos.y, 1, 1, GL_RED, GL_FLOAT, &pixelIdxf);
			pixelIdx = (uint)pixelIdxf;
#else
			glReadPixels((int)mousepos.x, (int)mousepos.y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &pixelIdx);
#endif

			if (pixelIdx > 0 && pixelIdx <= m_AllRegisteredObjects.size())
			{
				//Compute World Space position
				float pixelDepth;
				glReadPixels((int)mousepos.x, (int)mousepos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &pixelDepth);

				clipspacepos.z = pixelDepth * 2.0f - 1.0f;
				m_OldWorldSpacePos = m_invViewProjMtx * clipspacepos;
				m_OldDepth = pixelDepth;

				Object* target_obj = m_AllRegisteredObjects[pixelIdx - 1];

				//Are we clicking the object or just hovering?
				if (!mouseHeld)
				{
					if (mouseDown)
					{
						m_pCurrentlyHeldObject = target_obj;
						m_pCurrentlyHeldObject->OnMouseDown(dt, m_OldWorldSpacePos);

						return true;
					}
					else
					{
						if (target_obj != m_pCurrentlyHoverObject)
						{
							if (m_pCurrentlyHoverObject != NULL) m_pCurrentlyHoverObject->OnMouseLeave(dt);
							m_pCurrentlyHoverObject = target_obj;
							m_pCurrentlyHoverObject->OnMouseEnter(dt);

							Window::GetWindow().SetCursorStyle(CURSOR_STYLE_GRAB);
						}
					}
				}
			}
			else if (m_pCurrentlyHoverObject != NULL)
			{
				m_pCurrentlyHoverObject->OnMouseLeave(dt);
				m_pCurrentlyHoverObject = NULL;

				Window::GetWindow().SetCursorStyle(CURSOR_STYLE_DEFAULT);
			}
		}
	}
	return false;
}

void ScreenPicker::HandleObjectMouseUp(float dt, bool mouse_in_window, Vector3& clip_space)
{
	if (!mouse_in_window)
	{
		m_pCurrentlyHeldObject->OnMouseUp(dt, m_OldWorldSpacePos);
	}
	else
	{
		clip_space.z = m_OldDepth * 2.0f - 1.0f;
		Vector3 finalWorldSpacePos = m_invViewProjMtx * clip_space;

		m_pCurrentlyHeldObject->OnMouseUp(dt, finalWorldSpacePos);
	}

	m_pCurrentlyHeldObject = NULL;
}

void ScreenPicker::HandleObjectMouseMove(float dt, Vector3& clip_space)
{
	//Compute World Space position
	clip_space.z = m_OldDepth * 2.0f - 1.0f;
	Vector3 newWorldSpacePos = m_invViewProjMtx * clip_space;

	Vector3 worldMovement = newWorldSpacePos - m_OldWorldSpacePos;
	m_OldWorldSpacePos = newWorldSpacePos;

	m_pCurrentlyHeldObject->OnMouseMove(dt, newWorldSpacePos, worldMovement);
}


void ScreenPicker::RenderPickingScene(RenderList* scene_renderlist, const Matrix4& proj_matrix, const Matrix4& view_matrix)
{
	Matrix4 projview = proj_matrix * view_matrix;
	m_invViewProjMtx = Matrix4::Inverse(projview);


	//Check to see if we even need an updated picking texture?
	Vector2 mousepos;
	if (m_pCurrentlyHeldObject != NULL || !Window::GetWindow().GetMouseScreenPos(&mousepos))
	{
		return;
	}

	//Setup Shader
	glUseProgram(m_pShaderPicker->GetProgram());
	glUniformMatrix4fv(glGetUniformLocation(m_pShaderPicker->GetProgram(), "uProjViewMtx"), 1, GL_FALSE, &projview.values[0]);

	

	//Bind FBO
	glBindFramebuffer(GL_FRAMEBUFFER, m_PickerFBO);
	glViewport(0, 0, m_TexWidth, m_TexHeight);
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	//Render Objects
	GLint uniloc_modelMatrix = glGetUniformLocation(m_pShaderPicker->GetProgram(), "uModelMtx");
	GLint uniloc_idx = glGetUniformLocation(m_pShaderPicker->GetProgram(), "uObjID");
	auto per_object_render = [&](Object* obj) {
		glUniformMatrix4fv(uniloc_modelMatrix, 1, false, (float*)&obj->GetWorldTransform());
		glUniform1ui(uniloc_idx, obj->GetScreenPickerIdx());
		obj->OnRenderObject();
	};
	scene_renderlist->RenderOpaqueObjects(per_object_render);
	scene_renderlist->RenderTransparentObjects(per_object_render);

	glEnable(GL_BLEND);
}

void ScreenPicker::ClearAllObjects()
{
	m_AllRegisteredObjects.clear();

	if (m_pCurrentlyHoverObject != NULL)
	{
		Window::GetWindow().SetCursorStyle(CURSOR_STYLE_DEFAULT);
	}
	m_pCurrentlyHoverObject = NULL;
	m_pCurrentlyHeldObject = NULL;
}
