#version 150 core

in  vec4 position;
in  vec4 colour;

out Vertex {
	vec4 colour;
	vec4 pos;	
} OUT;

void main(void)	{
	gl_Position	  = vec4(position.xyz, 1.0f);
	OUT.pos		  = position;	
	OUT.colour    = colour;
}