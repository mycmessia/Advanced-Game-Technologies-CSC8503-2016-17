#include "NCLDebug.h"
#include <algorithm>
#include <sstream>

Vector3	NCLDebug::m_CameraPosition;
Matrix4	NCLDebug::m_ProjMtx;
Matrix4	NCLDebug::m_ViewMtx;
Matrix4	NCLDebug::m_ProjViewMtx;

int NCLDebug::m_NumStatusEntries = 0;
float NCLDebug::m_MaxStatusEntryWidth = 0.0f;
std::vector<LogEntry> NCLDebug::m_vLogEntries;
int NCLDebug::m_LogEntriesOffset = 0;
size_t	NCLDebug::m_OffsetChars  = 0;

std::vector<Vector4> NCLDebug::m_vChars;
NCLDebug::DebugDrawList NCLDebug::m_DrawList;
NCLDebug::DebugDrawList NCLDebug::m_DrawListNDT;

Shader*	NCLDebug::m_pShaderPoints = NULL;
Shader*	NCLDebug::m_pShaderLines = NULL;
Shader*	NCLDebug::m_pShaderHairLines = NULL;
Shader*	NCLDebug::m_pShaderText = NULL;

GLuint NCLDebug::m_glArray = NULL;
GLuint NCLDebug::m_glBuffer = NULL;
GLuint NCLDebug::m_glFontTex = NULL;



//Draw Point (circle)
void NCLDebug::GenDrawPoint(bool ndt, const Vector3& pos, float point_radius, const Vector4& colour)
{
	auto list = ndt ? &m_DrawListNDT : &m_DrawList;
	list->_vPoints.push_back(Vector4(pos.x, pos.y, pos.z, point_radius));
	list->_vPoints.push_back(colour);
}

void NCLDebug::DrawPoint(const Vector3& pos, float point_radius, const Vector3& colour)
{
	GenDrawPoint(false, pos, point_radius, Vector4(colour.x, colour.y, colour.z, 1.0f));
}
void NCLDebug::DrawPoint(const Vector3& pos, float point_radius, const Vector4& colour)
{
	GenDrawPoint(false, pos, point_radius, colour);
}
void NCLDebug::DrawPointNDT(const Vector3& pos, float point_radius, const Vector3& colour)
{
	GenDrawPoint(true, pos, point_radius, Vector4(colour.x, colour.y, colour.z, 1.0f));
}
void NCLDebug::DrawPointNDT(const Vector3& pos, float point_radius, const Vector4& colour)
{
	GenDrawPoint(true, pos, point_radius, colour);
}



//Draw Line with a given thickness 
void NCLDebug::GenDrawThickLine(bool ndt, const Vector3& start, const Vector3& end, float line_width, const Vector4& colour)
{
	auto list = ndt ? &m_DrawListNDT : &m_DrawList;

	//For Depth Sorting
	Vector3 midPoint = (start + end) * 0.5f;
	float camDist = Vector3::Dot(midPoint - m_CameraPosition, midPoint - m_CameraPosition);

	//Add to Data Structures
	list->_vThickLines.push_back(Vector4(start.x, start.y, start.z, line_width));
	list->_vThickLines.push_back(colour);

	list->_vThickLines.push_back(Vector4(end.x, end.y, end.z, camDist));
	list->_vThickLines.push_back(colour);

	GenDrawPoint(ndt, start, line_width * 0.5f, colour);
	GenDrawPoint(ndt, end, line_width * 0.5f, colour);
}
void NCLDebug::DrawThickLine(const Vector3& start, const Vector3& end, float line_width, const Vector3& colour)
{
	GenDrawThickLine(false, start, end, line_width, Vector4(colour.x, colour.y, colour.z, 1.0f));
}
void NCLDebug::DrawThickLine(const Vector3& start, const Vector3& end, float line_width, const Vector4& colour)
{
	GenDrawThickLine(false, start, end, line_width, colour);
}
void NCLDebug::DrawThickLineNDT(const Vector3& start, const Vector3& end, float line_width, const Vector3& colour)
{
	GenDrawThickLine(true, start, end, line_width, Vector4(colour.x, colour.y, colour.z, 1.0f));
}
void NCLDebug::DrawThickLineNDT(const Vector3& start, const Vector3& end, float line_width, const Vector4& colour)
{
	GenDrawThickLine(true, start, end, line_width, colour);
}


//Draw line with thickness of 1 screen pixel regardless of distance from camera
void NCLDebug::GenDrawHairLine(bool ndt, const Vector3& start, const Vector3& end, const Vector4& colour)
{
	auto list = ndt ? &m_DrawListNDT : &m_DrawList;
	list->_vHairLines.push_back(Vector4(start.x, start.y, start.z, 1.0f));
	list->_vHairLines.push_back(colour);

	list->_vHairLines.push_back(Vector4(end.x, end.y, end.z, 1.0f));
	list->_vHairLines.push_back(colour);
}
void NCLDebug::DrawHairLine(const Vector3& start, const Vector3& end, const Vector3& colour)
{
	GenDrawHairLine(false, start, end, Vector4(colour.x, colour.y, colour.z, 1.0f));
}
void NCLDebug::DrawHairLine(const Vector3& start, const Vector3& end, const Vector4& colour)
{
	GenDrawHairLine(false, start, end, colour);
}
void NCLDebug::DrawHairLineNDT(const Vector3& start, const Vector3& end, const Vector3& colour)
{
	GenDrawHairLine(true, start, end, Vector4(colour.x, colour.y, colour.z, 1.0f));
}
void NCLDebug::DrawHairLineNDT(const Vector3& start, const Vector3& end, const Vector4& colour)
{
	GenDrawHairLine(true, start, end, colour);
}


//Draw Matrix (x,y,z axis at pos)
void NCLDebug::DrawMatrix(const Matrix4& mtx)
{
	Vector3 position = mtx.GetPositionVector();
	GenDrawHairLine(false, position, position + Vector3(mtx.values[0], mtx.values[1], mtx.values[2]), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	GenDrawHairLine(false, position, position + Vector3(mtx.values[4], mtx.values[5], mtx.values[6]), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	GenDrawHairLine(false, position, position + Vector3(mtx.values[8], mtx.values[9], mtx.values[10]), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
}
void NCLDebug::DrawMatrix(const Matrix3& mtx, const Vector3& position)
{
	GenDrawHairLine(false, position, position + mtx.GetCol(0), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	GenDrawHairLine(false, position, position + mtx.GetCol(1), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	GenDrawHairLine(false, position, position + mtx.GetCol(2), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
}
void NCLDebug::DrawMatrixNDT(const Matrix4& mtx)
{
	Vector3 position = mtx.GetPositionVector();
	GenDrawHairLine(true, position, position + Vector3(mtx.values[0], mtx.values[1], mtx.values[2]), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	GenDrawHairLine(true, position, position + Vector3(mtx.values[4], mtx.values[5], mtx.values[6]), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	GenDrawHairLine(true, position, position + Vector3(mtx.values[8], mtx.values[9], mtx.values[10]), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
}
void NCLDebug::DrawMatrixNDT(const Matrix3& mtx, const Vector3& position)
{
	GenDrawHairLine(true, position, position + mtx.GetCol(0), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	GenDrawHairLine(true, position, position + mtx.GetCol(1), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	GenDrawHairLine(true, position, position + mtx.GetCol(2), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
}



//Draw Triangle 
void NCLDebug::GenDrawTriangle(bool ndt, const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector4& colour)
{
	auto list = ndt ? &m_DrawListNDT : &m_DrawList;

	//For Depth Sorting
	Vector3 midPoint = (v0 + v1 + v2) * (1.0f / 3.0f);
	float camDist = Vector3::Dot(midPoint - m_CameraPosition, midPoint - m_CameraPosition);

	//Add to data structures
	list->_vTris.push_back(Vector4(v0.x, v0.y, v0.z, camDist));
	list->_vTris.push_back(colour);

	list->_vTris.push_back(Vector4(v1.x, v1.y, v1.z, 1.0f));
	list->_vTris.push_back(colour);

	list->_vTris.push_back(Vector4(v2.x, v2.y, v2.z, 1.0f));
	list->_vTris.push_back(colour);
}

void NCLDebug::DrawTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector4& colour)
{
	GenDrawTriangle(false, v0, v1, v2, colour);
}

void NCLDebug::DrawTriangleNDT(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Vector4& colour)
{
	GenDrawTriangle(true, v0, v1, v2, colour);
}

//Draw Polygon (Renders as a triangle fan, so verts must be arranged in order)
void NCLDebug::DrawPolygon(int n_verts, const Vector3* verts, const Vector4& colour)
{
	for (int i = 2; i < n_verts; ++i)
	{
		GenDrawTriangle(false, verts[0], verts[i - 1], verts[i], colour);
	}
}

void NCLDebug::DrawPolygonNDT(int n_verts, const Vector3* verts, const Vector4& colour)
{
	for (int i = 2; i < n_verts; ++i)
	{
		GenDrawTriangle(true, verts[0], verts[i - 1], verts[i], colour);
	}
}





void NCLDebug::DrawTextCs(const Vector4& cs_pos, const float font_size, const string& text, const TextAlignment alignment, const Vector4 colour)
{
	Vector2 screenSize = Window::GetWindow().GetScreenSize();
	Vector3 cs_size = Vector3(font_size / screenSize.x, font_size / screenSize.y, 0.0f);
	cs_size = cs_size * cs_pos.w;

	//Work out the starting position of text based off desired alignment
	float x_offset = 0.0f;
	int text_len = text.length();

	switch (alignment)
	{
	case TEXTALIGN_RIGHT:
		x_offset = -text_len * cs_size.x * 1.2f;
		break;

	case TEXTALIGN_CENTRE:
		x_offset = -text_len * cs_size.x * 0.6f;
		break;
	}



	//Add each characters to the draw list individually
	for (int i = 0; i < text_len; ++i)
	{
		Vector4 char_pos = Vector4(cs_pos.x + x_offset, cs_pos.y, cs_pos.z, cs_pos.w);
		Vector4 char_data = Vector4(cs_size.x, cs_size.y, (float)(text[i]), 0.0f);

		m_vChars.push_back(char_pos);
		m_vChars.push_back(char_data);
		m_vChars.push_back(colour);
		m_vChars.push_back(colour);	//We dont really need this, but we need the padding to match the same vertex format as all the other debug drawables

		x_offset += cs_size.x * 1.2f;
	}
}


//Draw Text WorldSpace
void NCLDebug::DrawTextWs(const Vector3& pos, const float font_size, const TextAlignment alignment, const Vector4 colour, const string text, ...)
{
	va_list args;
	va_start(args, text);

	char buf[1024];
	int needed = vsnprintf_s(buf, 1023, _TRUNCATE, text.c_str(), args);
	va_end(args);

	int length = (needed < 0) ? 1024 : needed;

	std::string formatted_text = std::string(buf, (size_t)length);

	Vector4 cs_pos = m_ProjViewMtx * Vector4(pos.x, pos.y, pos.z, 1.0f);
	DrawTextCs(cs_pos, font_size, formatted_text, alignment, colour);
}

void NCLDebug::DrawTextWsNDT(const Vector3& pos, const float font_size, const TextAlignment alignment, const Vector4 colour, const string text, ...)
{
	va_list args;
	va_start(args, text);

	char buf[1024];
	int needed = vsnprintf_s(buf, 1023, _TRUNCATE, text.c_str(), args);
	va_end(args);

	int length = (needed < 0) ? 1024 : needed;

	std::string formatted_text = std::string(buf, (size_t)length);

	Vector4 cs_pos = m_ProjViewMtx * Vector4(pos.x, pos.y, pos.z, 1.0f);
	cs_pos.z = 1.0f * cs_pos.w;
	DrawTextCs(cs_pos, font_size, formatted_text, alignment, colour);
}


//Status Entry
void NCLDebug::AddStatusEntry(const Vector4& colour, const std::string text, ...)
{
	float cs_size_x = STATUS_TEXT_SIZE / Window::GetWindow().GetScreenSize().x * 2.0f;
	float cs_size_y = STATUS_TEXT_SIZE / Window::GetWindow().GetScreenSize().y * 2.0f;

	va_list args;
	va_start(args, text);

	char buf[1024];
	int needed = vsnprintf_s(buf, 1023, _TRUNCATE, text.c_str(), args);
	va_end(args);

	int length = (needed < 0) ? 1024 : needed;

	std::string formatted_text = std::string(buf, (size_t)length);

	DrawTextCs(Vector4(-1.0f + cs_size_x * 0.5f, 1.0f - (m_NumStatusEntries * cs_size_y) - cs_size_y, -1.0f, 1.0f), STATUS_TEXT_SIZE, formatted_text, TEXTALIGN_LEFT, colour);
	m_NumStatusEntries++;
	m_MaxStatusEntryWidth = max(m_MaxStatusEntryWidth, cs_size_x * 0.6f * length);
}


//Log
void NCLDebug::AddLogEntry(const Vector3& colour, const std::string& text)
{
	time_t now = time(0);
	tm ltm;
	localtime_s(&ltm, &now);

	std::stringstream ss;
	ss << "[" << ltm.tm_hour << ":" << ltm.tm_min << ":" << ltm.tm_sec << "] ";

	LogEntry le;
	le.text = ss.str() + text;
	le.colour = Vector4(colour.x, colour.y, colour.z, 1.0f);

	if (m_vLogEntries.size() < MAX_LOG_SIZE)
		m_vLogEntries.push_back(le);
	else
	{
		m_vLogEntries[m_LogEntriesOffset] = le;
		m_LogEntriesOffset = (m_LogEntriesOffset + 1) % MAX_LOG_SIZE;
	}

	std::cout << text << endl;
}

void NCLDebug::Log(const Vector3& colour, const std::string text, ...)
{
	va_list args;
	va_start(args, text);

	char buf[1024];
	int needed = vsnprintf_s(buf, 1023, _TRUNCATE, text.c_str(), args);
	va_end(args);

	int length = (needed < 0) ? 1024 : needed;
	AddLogEntry(colour, std::string(buf, (size_t)length));
}

void NCLDebug::Log(const std::string text, ...)
{
	va_list args;
	va_start(args, text);

	char buf[1024];
	int needed = vsnprintf_s(buf, 1023, _TRUNCATE, text.c_str(), args);
	va_end(args);

	int length = (needed < 0) ? 1024 : needed;
	AddLogEntry(Vector3(0.4f, 1.0f, 0.6f), std::string(buf, (size_t)length));
}

void NCLDebug::LogE(const char* filename, int linenumber, const std::string text, ...)
{
	//Error Format:
	//<text>
	//		-> <line number> : <file name> 

	va_list args;
	va_start(args, text);

	char buf[1024];
	int needed = vsnprintf_s(buf, 1023, _TRUNCATE, text.c_str(), args);
	va_end(args);

	int length = (needed < 0) ? 1024 : needed;

	Log(Vector3(1.0f, 0.25f, 0.25f), "[ERROR] %s:%d", filename, linenumber);
	AddLogEntry(Vector3(1.0f, 0.5f, 0.5f), "\t \x01 \"" + std::string(buf, (size_t)length) + "\"");
	
	std::cout << endl;
}




void NCLDebug::ClearDebugLists()
{
	m_vChars.clear();

	auto clear_list = [](NCLDebug::DebugDrawList& list)
	{
		list._vPoints.clear();
		list._vThickLines.clear();
		list._vHairLines.clear();
		list._vTris.clear();	
	};
	clear_list(m_DrawList);
	clear_list(m_DrawListNDT);

	m_NumStatusEntries = 0;
	m_MaxStatusEntryWidth = 0.0f;
}

void NCLDebug::ClearLog()
{
	m_vLogEntries.clear();
	m_LogEntriesOffset = 0;
}

struct PointVertex
{
	Vector4 pos;
	Vector4 col;
};

struct LineVertex
{
	PointVertex p0;
	PointVertex p1;
};

struct TriVertex
{
	PointVertex p0;
	PointVertex p1;
	PointVertex p2;
};

void NCLDebug::SortDebugLists()
{
	//Draw log text
	float cs_size_x = LOG_TEXT_SIZE / Window::GetWindow().GetScreenSize().x * 2.0f;
	float cs_size_y = LOG_TEXT_SIZE / Window::GetWindow().GetScreenSize().y * 2.0f;
	size_t log_len = m_vLogEntries.size();

	float max_x = 0.0f;
	for (size_t i = 0; i < log_len; ++i)
	{
		max_x = max(max_x, m_vLogEntries[i].text.length() * cs_size_x * 0.6f);

		size_t idx = (i + m_LogEntriesOffset) % MAX_LOG_SIZE;
		float alpha = ((m_vLogEntries.size() - i) / (float(MAX_LOG_SIZE)));
		alpha = 1.0f - (alpha * alpha);

		DrawTextCs(Vector4(-1.0f + cs_size_x * 0.5f, -1.0f + ((log_len - i - 1) * cs_size_y) + cs_size_y, 0.0f, 1.0f), LOG_TEXT_SIZE, m_vLogEntries[idx].text, TEXTALIGN_LEFT, m_vLogEntries[idx].colour);
	}


	auto sort_lists = [](NCLDebug::DebugDrawList& list)
	{
		//Sort Points
		if (!list._vPoints.empty())
		{
			PointVertex* points = reinterpret_cast<PointVertex*>(&list._vPoints[0].x);
			std::sort(points, points + list._vPoints.size() / 2, [&](const PointVertex& a, const PointVertex& b)
			{
				float a2 = Vector3::Dot(a.pos.ToVector3() - m_CameraPosition, a.pos.ToVector3() - m_CameraPosition);
				float b2 = Vector3::Dot(b.pos.ToVector3() - m_CameraPosition, b.pos.ToVector3() - m_CameraPosition);
				return (a2 > b2);
			});
		}

		//Sort Lines
		if (!list._vThickLines.empty())
		{
			LineVertex* lines = reinterpret_cast<LineVertex*>(&list._vThickLines[0].x);
			std::sort(lines, lines + list._vThickLines.size() / 4, [](const LineVertex& a, const LineVertex& b)
			{
				return (a.p1.pos.w > b.p1.pos.w);
			});
		}

		//Sort Triangles
		if (!list._vTris.empty())
		{
			TriVertex* tris = reinterpret_cast<TriVertex*>(&list._vTris[0].x);
			std::sort(tris, tris + list._vTris.size() / 6, [](const TriVertex& a, const TriVertex& b)
			{
				return (a.p0.pos.w > b.p0.pos.w);
			});
		}
	};

	sort_lists(m_DrawList);
	sort_lists(m_DrawListNDT);



	//Draw background to text areas
	// - This is done last as to avoid additional triangle-sorting
	Matrix4 invProjView = Matrix4::Inverse(m_ProjViewMtx);
	float rounded_offset_x = 10.f / Window::GetWindow().GetScreenSize().x * 2.0f;
	float rounded_offset_y = 10.f / Window::GetWindow().GetScreenSize().y * 2.0f;
	const Vector4 log_background_col(0.1f, 0.1f, 0.1f, 0.5f);

	Vector3 centre;
	Vector3 last;
	auto NextTri = [&](const Vector3& point)
	{
		DrawTriangleNDT(centre, last, point, log_background_col);
		DrawHairLineNDT(last, point);
		last = point;
	};

	//Draw Log Background
	if (m_vLogEntries.size() > 0)
	{
		float top_y = -1 + m_vLogEntries.size() * cs_size_y + cs_size_y;
		max_x = max_x - 1 + cs_size_x;

		centre = invProjView * Vector3(-1, -1, 0);
		last = invProjView * Vector3(max_x, -1, 0);
		NextTri(invProjView * Vector3(max_x, top_y - rounded_offset_y, 0.0f));
		for (int i = 0; i < 5; ++i)
		{
			Vector3 round_offset = Vector3(
				cos(DegToRad(i * 22.5f)) * rounded_offset_x,
				sin(DegToRad(i * 22.5f)) * rounded_offset_y,
				0.0f);
			NextTri(invProjView * Vector3(max_x + round_offset.x - rounded_offset_x, top_y + round_offset.y - rounded_offset_y, 0.0f));
		}
		NextTri(invProjView * Vector3(-1, top_y, 0.0f));
	}

	//Draw Status Background
	if (m_NumStatusEntries > 0)
	{
		float cs_size_x = STATUS_TEXT_SIZE / Window::GetWindow().GetScreenSize().x * 2.0f;
		float cs_size_y = STATUS_TEXT_SIZE / Window::GetWindow().GetScreenSize().y * 2.0f;

		float btm_y = 1 - m_NumStatusEntries * cs_size_y - cs_size_y;
		max_x = -1 + cs_size_x + m_MaxStatusEntryWidth;

		centre = invProjView * Vector3(-1, 1, 0);
		last = invProjView * Vector3(-1, btm_y, 0);

		NextTri(invProjView * Vector3(max_x - rounded_offset_x, btm_y, 0.0f));
		for (int i = 4; i >= 0; --i)
		{
			Vector3 round_offset = Vector3(
				cos(DegToRad(i * 22.5f)) * rounded_offset_x,
				sin(DegToRad(i * 22.5f)) * rounded_offset_y,
				0.0f);
			NextTri(invProjView * Vector3(max_x + round_offset.x - rounded_offset_x, btm_y - round_offset.y + rounded_offset_y, 0.0f));
		}
		NextTri(invProjView * Vector3(max_x, 1, 0.0f));
	}
}

void NCLDebug::DrawDebugLists()
{
	if (!m_glArray)
	{
		NCLERROR("Unable to load all ncldebug shaders!");
		return;
	}

	//Buffer all data into the single buffer object
	size_t max_size = 0;
	max_size += m_DrawList._vPoints.size() + m_DrawList._vThickLines.size() + m_DrawList._vHairLines.size() + m_DrawList._vTris.size();
	max_size += m_DrawListNDT._vPoints.size() + m_DrawListNDT._vThickLines.size() + m_DrawListNDT._vHairLines.size() + m_DrawListNDT._vTris.size();
	max_size += m_vChars.size();
	max_size *= sizeof(Vector4);


	size_t buffer_offsets[8];
	//DT Draw List
	buffer_offsets[0] = 0;
	buffer_offsets[1] = m_DrawList._vPoints.size();
	buffer_offsets[2] = buffer_offsets[1] + m_DrawList._vThickLines.size();
	buffer_offsets[3] = buffer_offsets[2] + m_DrawList._vHairLines.size();

	//NDT Draw List
	buffer_offsets[4] = buffer_offsets[3] + m_DrawList._vTris.size();
	buffer_offsets[5] = buffer_offsets[4] + m_DrawListNDT._vPoints.size();
	buffer_offsets[6] = buffer_offsets[5] + m_DrawListNDT._vThickLines.size();
	buffer_offsets[7] = buffer_offsets[6] + m_DrawListNDT._vHairLines.size();

	//Char Offset 
	m_OffsetChars     = buffer_offsets[7] + m_DrawListNDT._vTris.size();



	const size_t stride = 2 * sizeof(Vector4);

	glBindVertexArray(m_glArray);
	glBindBuffer(GL_ARRAY_BUFFER, m_glBuffer);
	glBufferData(GL_ARRAY_BUFFER, max_size, NULL, GL_STATIC_DRAW);

	glVertexAttribPointer(VERTEX_BUFFER, 4, GL_FLOAT, GL_FALSE, stride, (void*)(0));
	glEnableVertexAttribArray(VERTEX_BUFFER);
	glVertexAttribPointer(COLOUR_BUFFER, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(Vector4)));
	glEnableVertexAttribArray(COLOUR_BUFFER);

	auto buffer_drawlist = [&](NCLDebug::DebugDrawList& list, size_t* offsets)
	{
		if (!list._vPoints.empty()) glBufferSubData(GL_ARRAY_BUFFER, offsets[0] * sizeof(Vector4), list._vPoints.size() * sizeof(Vector4), &list._vPoints[0].x);
		if (!list._vThickLines.empty()) glBufferSubData(GL_ARRAY_BUFFER, offsets[1] * sizeof(Vector4), list._vThickLines.size() * sizeof(Vector4), &list._vThickLines[0].x);
		if (!list._vHairLines.empty()) glBufferSubData(GL_ARRAY_BUFFER, offsets[2] * sizeof(Vector4), list._vHairLines.size() * sizeof(Vector4), &list._vHairLines[0].x);
		if (!list._vTris.empty()) glBufferSubData(GL_ARRAY_BUFFER, offsets[3] * sizeof(Vector4), list._vTris.size() * sizeof(Vector4), &list._vTris[0].x);
	};
	buffer_drawlist(m_DrawList, &buffer_offsets[0]);
	buffer_drawlist(m_DrawListNDT, &buffer_offsets[4]);
	if (!m_vChars.empty()) glBufferSubData(GL_ARRAY_BUFFER, m_OffsetChars * sizeof(Vector4), m_vChars.size() * sizeof(Vector4), &m_vChars[0].x);

	Vector2 screen_size = Window::GetWindow().GetScreenSize();
	float aspectRatio = screen_size.y / screen_size.x;


	auto render_drawlist = [&](NCLDebug::DebugDrawList& list, size_t* offsets)
	{
		if (m_pShaderPoints && list._vPoints.size() > 0)
		{
			glUseProgram(m_pShaderPoints->GetProgram());
			glUniformMatrix4fv(glGetUniformLocation(m_pShaderPoints->GetProgram(), "uProjMtx"), 1, GL_FALSE, &m_ProjMtx.values[0]);
			glUniformMatrix4fv(glGetUniformLocation(m_pShaderPoints->GetProgram(), "uViewMtx"), 1, GL_FALSE, &m_ViewMtx.values[0]);

			glDrawArrays(GL_POINTS, offsets[0] >> 1, list._vPoints.size() >> 1);
		}

		if (m_pShaderLines && list._vThickLines.size() > 0)
		{
			glUseProgram(m_pShaderLines->GetProgram());
			glUniformMatrix4fv(glGetUniformLocation(m_pShaderLines->GetProgram(), "uProjViewMtx"), 1, GL_FALSE, &m_ProjViewMtx.values[0]);
			glUniform1f(glGetUniformLocation(m_pShaderLines->GetProgram(), "uAspect"), aspectRatio);

			glDrawArrays(GL_LINES, offsets[1] >> 1, list._vThickLines.size() >> 1);
		}

		if (m_pShaderHairLines && (list._vHairLines.size() + list._vTris.size()) > 0)
		{
			glUseProgram(m_pShaderHairLines->GetProgram());
			glUniformMatrix4fv(glGetUniformLocation(m_pShaderHairLines->GetProgram(), "uProjViewMtx"), 1, GL_FALSE, &m_ProjViewMtx.values[0]);

			if (!list._vHairLines.empty()) glDrawArrays(GL_LINES, offsets[2] >> 1, list._vHairLines.size() >> 1);
			if (!list._vTris.empty()) glDrawArrays(GL_TRIANGLES, offsets[3] >> 1, list._vTris.size() >> 1);
		}
	};

	render_drawlist(m_DrawList, &buffer_offsets[0]);

	glDisable(GL_DEPTH_TEST);
	render_drawlist(m_DrawListNDT, &buffer_offsets[4]);
	glEnable(GL_DEPTH_TEST);
}

//This Function has been abstracted from previous set of draw calls as to avoid supersampling the font bitmap.. which is bad enough as it is.
void NCLDebug::DrawDebubHUD()
{
	//All text data already updated in main DebugDrawLists
	// - we just need to rebind and draw it

	if (m_pShaderText && m_vChars.size() > 0)
	{
		glBindVertexArray(m_glArray);
		glUseProgram(m_pShaderText->GetProgram());
		glUniform1i(glGetUniformLocation(m_pShaderText->GetProgram(), "uFontTex"), 5);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, m_glFontTex);

		glDrawArrays(GL_LINES, m_OffsetChars >> 1, m_vChars.size() >> 1);
	}
}

void NCLDebug::LoadShaders()
{
	m_pShaderPoints = new Shader(
		SHADERDIR"DebugShaders/PointVertex.glsl",
		SHADERDIR"DebugShaders/PointFragment.glsl",
		SHADERDIR"DebugShaders/PointGeometry.glsl");
	if (!m_pShaderPoints->LinkProgram())
	{
		NCLERROR("NCLDebug Point shader could not be loaded");
	}

	m_pShaderLines = new Shader(
		SHADERDIR"DebugShaders/Vertex.glsl",
		SHADERDIR"DebugShaders/Fragment.glsl",
		SHADERDIR"DebugShaders/LineGeometry.glsl");
	if (!m_pShaderLines->LinkProgram())
	{
		NCLERROR("NCLDebug ThickLine shader could not be loaded");
	}

	m_pShaderHairLines = new Shader(
		SHADERDIR"DebugShaders/VertexColOnly.glsl",
		SHADERDIR"DebugShaders/Fragment.glsl");
	if (!m_pShaderHairLines->LinkProgram())
	{
		NCLERROR("NCLDebug HairLine shader could not be loaded");
	}

	m_pShaderText = new Shader(
		SHADERDIR"DebugShaders/TextVertex.glsl",
		SHADERDIR"DebugShaders/TextFragment.glsl",
		SHADERDIR"DebugShaders/TextGeometry.glsl");
	if (!m_pShaderText->LinkProgram())
	{
		NCLERROR("NCLDebug Text shader could not be loaded");
	}

	//Create Buffers
	glGenVertexArrays(1, &m_glArray);
	glGenBuffers(1, &m_glBuffer);

	//Load Font Texture
	m_glFontTex = SOIL_load_OGL_texture(
		TEXTUREDIR"font512.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT);
	glBindTexture(GL_TEXTURE_2D, m_glFontTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (!m_glFontTex)
	{
		NCLERROR("NCLDebug could not load font texture");
	}

}

void NCLDebug::ReleaseShaders()
{
	if (m_pShaderPoints)
	{
		delete m_pShaderPoints;
		m_pShaderPoints = NULL;
	}

	if (m_pShaderLines)
	{
		delete m_pShaderLines;
		m_pShaderLines = NULL;
	}

	if (m_pShaderHairLines)
	{
		delete m_pShaderHairLines;
		m_pShaderHairLines = NULL;
	}

	if (m_pShaderText)
	{
		delete m_pShaderText;
		m_pShaderText = NULL;
	}

	if (m_glArray)
	{
		glDeleteVertexArrays(1, &m_glArray);
		glDeleteBuffers(1, &m_glBuffer);
		m_glArray = NULL;
	}

	if (m_glFontTex)
	{
		glDeleteTextures(1, &m_glFontTex);
		m_glFontTex = NULL;
	}
}

