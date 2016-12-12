#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

out Vertex	{
	vec2 texCoord;
} OUT;

void main()  
{  
	OUT.texCoord = vec2(0.0f, 0.0f);
	gl_Position = vec4(-1.0f, -1.0f, 0.0f, 1.0f);   
	EmitVertex();		

	OUT.texCoord = vec2(1.0f, 0.0f);
	gl_Position = vec4(1.0f, -1.0f, 0.0f, 1.0f);   
	EmitVertex();
	
	OUT.texCoord = vec2(0.0f, 1.0f);
	gl_Position = vec4(-1.0f, 1.0f, 0.0f, 1.0f);   
	EmitVertex();

	OUT.texCoord = vec2(1.0f, 1.0f);
	gl_Position = vec4(1.0f, 1.0f, 0.0f, 1.0f);   
	EmitVertex();	
}  