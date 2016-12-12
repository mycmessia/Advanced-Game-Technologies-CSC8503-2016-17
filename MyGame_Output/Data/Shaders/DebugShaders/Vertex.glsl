#version 150 core

uniform mat4 uProjViewMtx;

in  vec4 position;
in  vec4 colour;

out Vertex {
	vec4 colour;
	vec4 pos;	
} OUT;

void main(void)	{
	vec4 vp = uProjViewMtx * vec4(position.xyz, 1.0f);
	gl_Position	  = vp;
	
	OUT.pos		  = vec4(vp.xyz, position.w);	
	OUT.colour    = colour;
}