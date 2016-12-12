/******************************************************************************
Class: NCLDebug
Author: Pieran Marris <p.marris@newcastle.ac.uk>
Description:

Combination of the debug tools available in OGLRenderer and some of the later graphics tutorials on
text renderering and particles systems. NCLDebug is designed to be a quick and dirty way of rendering
simple geometry on-screen anywhere in the program. 

All functions are global, and can be called at any time anywhere in the program. All log entries are
also printed out to the console in case of log messages/errors that occur before the renderer has initiated.


Below is a list of functions supported by NCLDebug:
Note: All functions have an "<function>NDT" varient which refers to a 'non depth-tested' alternative
	which will always appear in front of any scene geometry.

	1. DrawPoint
		Draws a circle on screen with the world-space radius provided

	2. DrawThickLine
		Draws a line between A and B in world space with a line-thickness t given in world-space

	3. DrawHairLine
		Draws a line between A and B in world space that will always be 1pixel thick, regardless of distance from the camera

	4. DrawMatrix
		Draws the X,Y,Z rotation axis of the given matrix at it's world-space position.
		Colours go in order, X:Red - Y:Green - Z:Blue

	5. DrawTriangle
		Draws a triangle on screen, with the vertices given in world-space coordinates

	6. DrawPolygon
		Draws a given polygon on screen. Converts vertices into a triangle fan with vertex[0] being the fan centre.

	7. DrawTextWs
		Draws text with font-size 'f' (given in screen pixels) at the world space position provided.

	8. DrawTextCs
		Draws text with font-size 'f' (given in screen pixels) at the clip space position provided.

	9. AddStatusEntry
		Appends the list of status entries (top left display) with the given text. The text is formatted in 
		accordance with C's printf function.
		Note: These status entries are cleared each frame and must be re-added each render cycle

	10. Log
		Appends the list of log entries (bottom left display) with the given text. The text is formatted in
		accordance with C's printf function. All log entries will also be prepended with the system time in
		hh::mm::ss format.
		Note: The Log entries are a call once and forget system, and will only be cleared once LOG_SIZE other log
			  entries have also be fired and pushed it off the stack.

	11. NCLERROR()
		Utility define to automatically add an error log entry. Using this define is preferable over just calling Log
		function as it will include the filename and linenumber it was triggered on with the relevant .cpp file.


		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <nclgl\Matrix4.h>
#include <nclgl\Matrix3.h>
#include <nclgl\Vector4.h>
#include <nclgl\Vector3.h>
#include <nclgl\Shader.h>
#include <vector>
#include <mutex>

#define MAX_LOG_SIZE		25
#define LOG_TEXT_SIZE  		14.0f
#define STATUS_TEXT_SIZE	16.0f

enum TextAlignment
{
	TEXTALIGN_LEFT,
	TEXTALIGN_RIGHT,
	TEXTALIGN_CENTRE
};

struct LogEntry
{
	Vector4		colour;
	std::string text;
};

#define NCLERROR(str, ...) NCLDebug::LogE(__FILE__, __LINE__, str, __VA_ARGS__)



class NCLDebug
{
	friend class Scene;
	friend class SceneRenderer;

public:
	//Note: Functions appended with 'NDT' (no depth testing) will always be rendered in the foreground. This can be useful for debugging things inside objects.


	//Draw Point (circle)
	static void DrawPoint(const Vector3& pos, float point_radius, const Vector3& colour);
	static void DrawPoint(const Vector3& pos, float point_radius, const Vector4& colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	static void DrawPointNDT(const Vector3& pos, float point_radius, const Vector3& colour);
	static void DrawPointNDT(const Vector3& pos, float point_radius, const Vector4& colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	//Draw Line with a given thickness 
	static void DrawThickLine(const Vector3& start, const Vector3& end, float line_width, const Vector3& colour);
	static void DrawThickLine(const Vector3& start, const Vector3& end, float line_width, const Vector4& colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	static void DrawThickLineNDT(const Vector3& start, const Vector3& end, float line_width, const Vector3& colour);
	static void DrawThickLineNDT(const Vector3& start, const Vector3& end, float line_width, const Vector4& colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	//Draw line with thickness of 1 screen pixel regardless of distance from camera
	static void DrawHairLine(const Vector3& start, const Vector3& end, const Vector3& colour);
	static void DrawHairLine(const Vector3& start, const Vector3& end, const Vector4& colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	static void DrawHairLineNDT(const Vector3& start, const Vector3& end, const Vector3& colour);
	static void DrawHairLineNDT(const Vector3& start, const Vector3& end, const Vector4& colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	//Draw Matrix (x,y,z axis at pos)
	static void DrawMatrix(const Matrix4& transform_mtx);
	static void DrawMatrix(const Matrix3& rotation_mtx, const Vector3& position);
	static void DrawMatrixNDT(const Matrix4& transform_mtx);
	static void DrawMatrixNDT(const Matrix3& rotation_mtx, const Vector3& position);

	//Draw Triangle 
	static void DrawTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector4& colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	static void DrawTriangleNDT(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector4& colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	//Draw Polygon (Renders as a triangle fan, so verts must be arranged in order)
	static void DrawPolygon(int n_verts, const Vector3* verts, const Vector4& colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	static void DrawPolygonNDT(int n_verts, const Vector3* verts, const Vector4& colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f));


	//Draw Text WorldSpace (pos given here in worldspace)
	static void DrawTextWs(const Vector3& pos, const float font_size, const TextAlignment alignment, const Vector4 colour, const string text, ...); ///See "printf" for usage manual
	static void DrawTextWsNDT(const Vector3& pos, const float font_size, const TextAlignment alignment, const Vector4 colour, const string text, ...); ///See "printf" for usage manual

	//Draw Text (pos is assumed to be pre-multiplied by projMtx * viewMtx at this point)
	static void DrawTextCs(const Vector4& pos, const float font_size, const string& text, const TextAlignment alignment = TEXTALIGN_LEFT, const Vector4 colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	//Add a status entry at the top left of the screen (Cleared each frame)
	static void AddStatusEntry(const Vector4& colour, const std::string text, ...); ///See "printf" for usuage manual

	//Add a log entry at the bottom left - persistent until scene reset
	static void Log(const Vector3& colour, const std::string text, ...); ///See "printf" for usuage manual
	static void Log(const std::string text, ...); //Default Text Colour

	//Add an error using default error formatting - use "NCLERROR("error description", <printf params>) to automatically call this function and fill in the required params
	static void LogE(const char* filename, int linenumber, const std::string text, ...);



protected:
	//Actual functions managing data parsing to save code bloat - called by public functions
	static void GenDrawPoint(bool ndt, const Vector3& pos, float point_radius, const Vector4& colour);
	static void GenDrawThickLine(bool ndt, const Vector3& start, const Vector3& end, float line_width, const Vector4& colour);
	static void GenDrawHairLine(bool ndt, const Vector3& start, const Vector3& end, const Vector4& colour);
	static void GenDrawTriangle(bool ndt, const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector4& colour);

	static void AddLogEntry(const Vector3& colour, const std::string& text);

	//Called by Scene Renderer class
	static void ClearDebugLists();
	static void SortDebugLists();
	static void DrawDebugLists();
	static void DrawDebubHUD();

	static void LoadShaders();
	static void ReleaseShaders();

	static void ClearLog();

	static void SetDebugDrawData(const Matrix4& projMtx, const Matrix4& viewMtx, const Vector3& camera_pos)
	{ 
		m_ProjMtx = projMtx;
		m_ViewMtx = viewMtx;
		m_ProjViewMtx = projMtx * viewMtx;
		m_CameraPosition = camera_pos;
	}

protected:
	static Vector3	m_CameraPosition;
	static Matrix4  m_ProjMtx;
	static Matrix4  m_ViewMtx;
	static Matrix4  m_ProjViewMtx;
	static int m_NumStatusEntries;
	static float m_MaxStatusEntryWidth;
	static std::vector<LogEntry> m_vLogEntries;
	static int m_LogEntriesOffset;

	static std::vector<Vector4> m_vChars;
	struct DebugDrawList
	{
		std::vector<Vector4> _vPoints;	
		std::vector<Vector4> _vThickLines;
		std::vector<Vector4> _vHairLines;
		std::vector<Vector4> _vTris;
	};
	static DebugDrawList m_DrawList;			//Depth-Tested
	static DebugDrawList m_DrawListNDT;			//Not Depth-Tested

	static Shader*	m_pShaderPoints;
	static Shader*	m_pShaderLines;
	static Shader*	m_pShaderHairLines;
	static Shader*	m_pShaderText;

	static GLuint	m_glArray, m_glBuffer;
	static GLuint	m_glFontTex;
	static size_t	m_OffsetChars;
};