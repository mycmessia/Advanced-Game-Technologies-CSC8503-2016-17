#version 150 core

in  vec4 position;
in  vec4 colour;

out Vertex {
	vec4 pos;
	vec4 colour;
} OUT;

void main(void)	{
	gl_Position	  = position;
	OUT.pos		  = position;	
	OUT.colour    = colour;
}