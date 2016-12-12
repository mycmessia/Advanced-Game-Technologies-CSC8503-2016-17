/******************************************************************************
Class: ScreenPicker
Implements: TSingleton
Author: Pieran Marris <p.marris@newcastle.ac.uk>
Description:
Uses picking as a means to allow mouse interactivity with world space objects.
Any renderable object can be clicked/hovered over etc and only requires registering/unregistering 
itself to the Screen Picker instance when it wants to start/stop listening for mouse events.

This class works by rendering all objects in the scene to an off-screen texture, with
all clickable objects outputing there array index to the framebuffer. This is then sampled at
the mouse position and the relevent objects mouse functions are called. In order to get the
world space position of the mouse at a given location the depth buffer is also sampled and
multiplied by the inverse (projMtx*viewMtx) similar to how deferred rendering converts screen
texture coordinates back into world space.

!!WARNING!!!
The method of reading pixels back from the graphics card is 'very' slow, as it requires the entire
rendering pipeline to stall while pixels are read back to the cpu. There is ways this can be optimized
by doing the reads asynchronously through the use of pixel buffers, however this wont aliaviate the big
issue. As you progress through the physics module and hopefully implement a broadphase you will find
doing a ray cast through the broadphase a much faster means of doing mouse-interactivity than screen
picking and the way all commercial game engines will handle mouse-interactivity.


		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RenderList.h"
#include "TSingleton.h"

//Our texture only stores 16bit unsigned shorts, so has a hard limit on the number of values it can store. 
//  Hopefully you will never be able to trigger this value though. 
#define MAX_PICKABLE_OBJECTS 65534

//NSIGHT doesnt (at the time of writing) support reading GL_RED_INTEGER format. So this hack will treat all indicies as 32bit floats
// that will be converted to integers again later. Not ideal - but worth it to allow nsight debugging as needed ;)
#define USE_NSIGHT_HACK

class ScreenPicker : public TSingleton<ScreenPicker>
{
	friend class TSingleton<ScreenPicker>;
	friend class SceneRenderer;

public:

	//Add object to list of 'clickable' objects to be tested 
	void RegisterObject(Object* obj);

	//Remove object from the list of 'clickable' objects
	void UnregisterObject(Object* obj);

protected:
	//Called by ScreenRenderer
	void ClearAllObjects();
	void UpdateFBO(int screen_width, int screen_height);
	void RenderPickingScene(RenderList* scene_renderlist, const Matrix4& proj_matrix, const Matrix4& view_matrix);

	//ScreenRenderer Update Phase
	//  - Returns true if an object has been clicked
	bool HandleMouseClicks(float dt); 

	//Internal handling of different mouse state scenarios
	void HandleObjectMouseUp(float dt, bool mouse_in_window, Vector3& clip_space);
	void HandleObjectMouseMove(float dt, Vector3& clip_space);


	//Pseodo Protected
	ScreenPicker();
	virtual ~ScreenPicker();
protected:

	//Array of all objects to be tested
	std::vector<Object*> m_AllRegisteredObjects;

	//Current State
	Object*			m_pCurrentlyHoverObject;
	Object*			m_pCurrentlyHeldObject;

	//Cached data to allow world-space movement computation
	float			m_OldDepth;
	Vector3			m_OldWorldSpacePos;

	//clip-space to world-space transform
	Matrix4			m_invViewProjMtx;



	//Shader
	Shader*			m_pShaderPicker;

	//Framebuffer
	int				m_TexWidth, m_TexHeight;
	GLuint			m_PickerFBO;

	// We use RenderBuffers instead of 2D Textures as the contents
	// of the framebuffer never have to be used for anything else.
	// Also having a 2D-Texture with a pixel format of unsigned int
	// is not currently supported.
	GLuint			m_PickerRB; 
	GLuint			m_PickerDepthRB;
};