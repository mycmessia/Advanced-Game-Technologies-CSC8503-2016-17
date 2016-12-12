#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4  uProjMtx;
uniform mat4  uViewMtx;

in Vertex	{
	vec4 colour;
	vec4 pos;
} IN[];

out Vertex	{
	vec4 colour;
	vec2 rPos;
	float depth_min;
	float depth_max;
} OUT;

void main()  
{  
	OUT.colour = IN[0].colour;
	
	vec4 p0 = uViewMtx * gl_in[0].gl_Position;
	float radius = IN[0].pos.w;

	vec4 near_depth = uProjMtx * (p0 + vec4(0,0,radius, 0));
	vec4 far_depth = uProjMtx * p0;
	OUT.depth_min = near_depth.z / near_depth.w;
	OUT.depth_max = far_depth.z / far_depth.w;
	

	OUT.rPos = vec2(-1, 1);
	gl_Position = uProjMtx * (p0 + vec4(-radius, radius, 0, 0));	
	EmitVertex();	
	
	OUT.rPos = vec2(-1, -1);
	gl_Position = uProjMtx * (p0 + vec4(-radius, -radius, 0, 0));	
	EmitVertex();
	
	OUT.rPos = vec2(1, 1);
	gl_Position = uProjMtx * (p0 + vec4(radius, radius, 0, 0));	
	EmitVertex();
	
	OUT.rPos = vec2(1, -1);
	gl_Position = uProjMtx * (p0 + vec4(radius, -radius, 0, 0));	
	EmitVertex();	
}  