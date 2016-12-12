/******************************************************************************
Class: ObjectMesh
Implements: Object
Author: Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description: 

Extension to the base Object class to handle managing and rendering a given
mesh object. For most game objects this will probably be the most common base
class. By default an object must handle it's own rendering incase it
needs to render something other than a mesh, or just wants to handle its own
VAO itself (e.g. the particle renderer from the gfx tutorials).

The 'deleteMeshOnCleanup' and 'deleteTexOnCleanup' refer to the memory management.
If they are set then the relavent components will automatically be cleanup when the
object is deleted. This can be useful for a hands free approach, though does mean
care must be taken when multiple objects point to the same mesh/texture that it 
does not get deleted multiple times.

		(\_/)							
		( '_')							
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\			
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Object.h"
#include <nclgl/Mesh.h>

class ObjectMesh : public Object
{
public:
	ObjectMesh(const std::string& name);
	virtual ~ObjectMesh();

	//Get/Set the mesh to use
	void	SetMesh(Mesh* mesh, bool deleteMeshOnCleanup);
	Mesh*	GetMesh()		{ return m_pMesh; }

	//Get/Set the texture to use for mesh rendering
	void	SetTexture(GLuint tex, bool deleteTexOnCleanup);
	GLuint  GetTexture()	{ return m_Texture; }

protected:
	//Handles OpenGL calls to Render the object - called by SceneRenderer
	void	OnRenderObject() override;				

protected:
	GLuint  m_Texture;
	Mesh*	m_pMesh;

	bool	m_DeleteMeshOnCleanup;
	bool	m_DeleteTexOnCleanup;
};