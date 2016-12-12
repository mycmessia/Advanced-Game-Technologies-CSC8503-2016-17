#version 330 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

uniform float uAspect;

in Vertex	{
	vec4 colour;
	vec4 pos;
} IN[];

out Vertex	{
	vec4 colour;
} OUT;

void main()  
{  
	float thickness = IN[0].pos.w * 1.2f;
	
	vec4 p0 = gl_in[0].gl_Position;
	vec4 p1 = gl_in[1].gl_Position;
	

	vec2 line = p1.xy / abs(p1.w) - p0.xy / abs(p0.w);
	vec2 line_normal = normalize(vec2(-line.y, line.x));
	
	vec4 offset = vec4(line_normal.x * uAspect, line_normal.y, 0, 0) * thickness;
	
	OUT.colour = IN[0].colour;
	gl_Position = p0 + offset;  EmitVertex();	
	gl_Position = p0 - offset;  EmitVertex();	
	gl_Position = p1 + offset;  EmitVertex();
	gl_Position = p1 - offset;  EmitVertex();
}