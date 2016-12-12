/******************************************************************************
Class: SceneRenderer
Implements: OGLRenderer
Author: Pieran Marris <p.marris@newcastle.ac.uk>
Description:
Summation of most of the topics covered in Graphics for Games in one single renderer 
instance. It takes in a Scene instance which contains a tree of all Objects and then
renders all opaque objects via deferred rendering and all transparent objects via forward
rednering.

All the things you might want to change per-scene are accessible via getters and setters
such as the camera class, light direction and background colour. Anything with a getter/setter
is automatically reset to it's default when the scene is switched via the SceneManager, so 
should be set within the given OnInitializeScene function within your scenes.


The renderer has attempted to be made as generic as possible, and should be fairly hands free 
from a usage point of view.

However if you are interested in some of the extra features/pit falls
I will try my best to detail them below:

Not Handled:
	1. [gfx tut #12] Post-Processing
	2. [gfx tut #14] Point Lights (Only single directional light)
	3. [gfx tut #15] Bump-Mapping 	
	4. [gfx tut #16] Environment Maps/SkyBoxes

Performance Issue:
	1. All objects are rendered single file, limiting the maximum throughput of the gfx card to
	   how fast we can upload uniform matrices and call the draw functions. Other than a complicated
	   instancing system, a much better and faster solution would be to use glMultiDrawIndirect to
	   draw an entire RenderList in a single draw call, allowing the gfx card to render objects as fast
	   as it physically can.

	2. It iterates over every object in the scene and updates them, and iterates through every object in 
	   the scene to build world tranforms each frame. It would make alot more sense if the objects themselves
	   registered if they needed updating or not and only update the objects in the scene that require it.
	   As your games get bigger, the world and the number of stationary objects will also increase so something
	   like this will save you alot of time.

Extra Features:
	1. Cascading Shadow Maps
		This is a technique of using multiple shadow maps with different projection/view matricies to
		provide higher quality shadows near the camera and approximate shadows far away. The overall
		result is the appearance of high quality shadows everywhere you look. The current implementation 
		maps the shadow maps to sections of the camera's view frustum, which is a quick means of implementing
		them. 
		If your interested in doing more with shadows - or want to implement them properly - I highly suggest
		reading the article below, it explains (much better than me) what cascading shadow maps are and
		the best practices for implementing them. 
		https://msdn.microsoft.com/en-gb/library/windows/desktop/ee416307(v=vs.85).aspx

	2. SSAA - SuperSampling Anti-Aliasing
		Anti-Aliasing has a big effect on overall appearance, and if your interested in it's effect I highly suggest
		putting it on a button-press and swapping between 1x and 8x+ anti-aliasing to see the difference. There are
		numerous ways of approximating anti-aliasing, and most video games will use a combination of them. The current
		implementation however uses	super sampling (known as true anti-aliasing), this involves rendering the entire scene
		at much higher resolution and then downsampling the final image into the final window. This provides the highest
		quality anti-aliasing of any method, however requires opengl to render the entire scene at x times the
		resolution and then taking x^2 samples per final fragment - resulting in a /very/ slow and /very/ memory intensive algorithm. 
		I couldn't find a nice single page that describes all the different AA techniques however im a sure quick google, 
		or a look at a AAA game settings, will list the common (and much more effecient) methods out there.

	3. Gamma Correction
		Monitor light is not actually linear, so displaying a pixel with colour [0.1, 0.1, 0.1] will not be the same difference
		from black as [0.9, 0.9, 0.9] is from white. This is known as the gamma curve and is automatically accounted for by your
		graphics card in the background. However, when we are doing lighting calculations (additive) we are expecting our colours
		to be linear so in order to correctly light up a scene we need to un-correct the gamma curve, do linear calculations, and
		then correct it again at the end. This is made much simpler with the sRGB colour format (used here in the main FBO)
		and enabling GL_FRAMEBUFFER_SRGB which tells opengl to automatically convert to and from our gamma curve everytime we read/write
		to and from a sRGB texture. At the last stage we do however need to apply gamma correction ourselves, this is carried out by 
		applying "gl_FragColor.rgb = pow(outcolor.rgb, 1.0 / gamma)" to our final glsl output.
		If your interested in gamma curves/correction the following article has all you will need to know:
		http://http.developer.nvidia.com/GPUGems3/gpugems3_ch24.html


		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <nclgl\OGLRenderer.h>
#include <nclgl\common.h>
#include "Scene.h"
#include "RenderList.h"
#include "TSingleton.h"

enum ScreenTextures //..or GBuffer
{
	SCREENTEX_DEPTH    = 0,			//Depth Buffer
	SCREENTEX_STENCIL  = 0,			//Stencil Buffer (Same Tex as Depth)
	SCREENTEX_COLOUR0  = 1,			//Main Render (gamma corrected) (Ping-Pong A)
	SCREENTEX_COLOUR1  = 2,			//Main Render (gamma corrected) (Ping-Pong B)
	SCREENTEX_NORMALS  = 3,			//Deferred Render - World Space Normals
	SCREENTEX_LIGHTING = 4,			//Deferred Render - Diffuse/Spec Computations
	SCREENTEX_MAX
};

#define SHADOWMAP_MAX 16	//Hard limit defined in shader

#define PROJ_FAR				80.0f			//Can see for 80m - setting this too far really hurts shadow quality as they attempt to cover the entirety of the view frustum
#define PROJ_NEAR				0.01f			//Nearest object @ 1cm
#define PROJ_FOV				45.0f			//45 degree field of view

class SceneRenderer : public OGLRenderer
{
public:
	//Initialize the SceneRenderer and all OpenGL components
	virtual void InitializeOGLContext(Window& parent) override;

	//Render Current Scene
	virtual void RenderScene() override;

	//Update Current Scene
	// Note: dt (delta time) here is in seconds not milliseconds to tie in with the physics updates
	virtual void UpdateScene(float dt) override; 

	//Get Camera instance
	inline Camera* GetCamera()									{ return m_pCamera; }

	//Get Render Lighting Parameters
	inline const Vector3& GetBackgroundColor()					{ return m_BackgroundColour; }
	inline const Vector3& GetAmbientColor()						{ return m_AmbientColour; }
	inline const Vector3& GetInverseLightDirection()			{ return m_InvLightDirection; }
	inline const float	  GetSpecularIntensity()				{ return m_SpecularIntensity; }

	//Set Render Lighting Parameters
	inline void SetBackgroundColor(const Vector3& col)			{ m_BackgroundColour = col; }
	inline void SetAmbientColor(const Vector3& col)				{ m_AmbientColour = col; }
	inline void SetInverseLightDirection(const Vector3& dir)	{ m_InvLightDirection = dir; m_InvLightDirection.Normalise(); }
	inline void SetSpecularIntensity(float intensity)			{ m_SpecularIntensity = intensity; }

	//Turn V-Sync on-off (if vsync is on, the renderer will be locked to only run at the speed of the monitor)
	inline bool GetVsyncEnabled()								{ return m_VsyncEnabled; }
	inline void SetVsyncEnabled(bool enabled)					{ wglSwapIntervalEXT((m_VsyncEnabled = enabled) ? 1 : 0); }

	//Get/Set Gamma Correction (default: 2.2)
	inline float GetGammaCorrection()							{ return m_GammaCorrection; }
	inline void  SetGammaCorrection(float gamma)				{ m_GammaCorrection = gamma; }

	//Get/Set Shadow map texture size (default: 2048)
	inline uint GetShadowMapSize()								{ return m_ShadowMapSize; }
	void SetShadowMapSize(uint size);

	//Get/Set Number of shadow maps to use for light source (default: 4)
	inline uint GetShadowMapNum()								{ return m_ShadowMapNum; }
	void SetShadowMapNum(uint num);

	//Get/Set Super sampling ammount (default: 4x)
	inline float GetSuperSamplingScalar()						{ return m_NumSuperSamples; }
	inline void  SetSuperSamplingScalar(float scalar)			{ m_NumSuperSamples = scalar; }


protected:
	//Class-Only Functions
	SceneRenderer();
	virtual ~SceneRenderer();

	bool InitialiseGL();
	void InitializeDefaults();

	void BuildFBOs();
	bool ShadersLoad();
	void ShadersSetDefaults();

	void DeferredRenderOpaqueObjects();
	void ForwardRenderTransparentObjects();

	void RenderShadowMaps();

protected:
	//Current Scene
	Scene*				m_pScene;
	
	//Shaders
	Shader*				m_pShaderShadow;
	Shader*				m_pShaderForwardLighting;
	Shader*				m_pShaderColNorm; 
	Shader*				m_pShaderLightDir;
	Shader*				m_pShaderCombineLighting;
	Shader*				m_pShaderPresentToWindow;

	//Camera + view frustum/renderlist
	Camera*				m_pCamera;
	Frustum				m_FrameFrustum;
	RenderList*			m_pFrameRenderList;

	//Render FBO
	GLuint				m_ScreenTexWidth, m_ScreenTexHeight;
	GLuint				m_ScreenFBO;
	GLuint				m_ScreenTex[SCREENTEX_MAX];

	//Shadow Maps
	uint				m_ShadowMapNum;
	uint				m_ShadowMapSize;
	bool				m_ShadowMapsInvalidated;
	GLuint				m_ShadowFBO;
	GLuint				m_ShadowTex;
	Matrix4				m_ShadowProj[SHADOWMAP_MAX];
	Matrix4				m_ShadowProjView[SHADOWMAP_MAX];
	RenderList*			m_apShadowRenderLists[SHADOWMAP_MAX];

	//Render Paramaters
	float				m_GammaCorrection; //Monitor Default: 1.0 / 2.2 (Where 2.2 here is the gamma of the monitor which we need to invert before showing)
	Vector3				m_BackgroundColour;
	Vector3				m_AmbientColour;
	Vector3				m_InvLightDirection;
	float				m_SpecularIntensity;
	bool				m_VsyncEnabled;
	float				m_NumSuperSamples;
};