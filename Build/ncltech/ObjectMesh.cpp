#include "ObjectMesh.h"

ObjectMesh::ObjectMesh(const std::string& name)
	: Object(name)
	, m_DeleteMeshOnCleanup(false)
	, m_DeleteTexOnCleanup(false)
	, m_pMesh(NULL)
	, m_Texture(0)
{
}

ObjectMesh::~ObjectMesh()
{
	if (m_DeleteMeshOnCleanup && m_pMesh)
	{
		delete m_pMesh;
		m_pMesh = NULL;
	}

	if (m_DeleteTexOnCleanup && m_Texture)
	{
		glDeleteTextures(1, &m_Texture);
		m_Texture = 0;
	}
}

void ObjectMesh::SetMesh(Mesh* mesh, bool deleteOnCleanup)
{
	m_pMesh = mesh;
	m_DeleteMeshOnCleanup = deleteOnCleanup;

	if (!m_Texture)
	{
		m_Texture = m_pMesh->GetTexture();
	}
}

void ObjectMesh::SetTexture(GLuint tex, bool deleteOnCleanup)
{
	m_Texture = tex;
	m_DeleteTexOnCleanup = deleteOnCleanup;
}

void ObjectMesh::OnRenderObject()
{
	if (m_Texture)
	{
		m_pMesh->SetTexture(m_Texture);
	}

	m_pMesh->Draw();
}