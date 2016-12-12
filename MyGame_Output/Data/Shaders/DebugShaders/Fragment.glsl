#version 150 core

in Vertex	{
	vec4 colour;
} IN;

out vec4 OutFrag;

void main(void)	{
	OutFrag = IN.colour;
}