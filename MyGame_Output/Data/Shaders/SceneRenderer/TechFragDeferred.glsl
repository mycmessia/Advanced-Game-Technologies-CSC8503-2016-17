#version 150 core

uniform sampler2D uDiffuseTex;

in Vertex	{
	vec4 worldPos;
	vec2 texCoord;
	vec4 colour;
	vec3 normal;
} IN;

out vec4 OutFrag[2];

void main(void)	{
	vec3 normal 	= normalize(IN.normal);
	vec4 texColour  = texture(uDiffuseTex, IN.texCoord);

//Output Final Fragment Colour
	OutFrag[0] 	= IN.colour * texColour;
	OutFrag[1]   = vec4(normal * 0.5f + 0.5f, 1.0f);
}