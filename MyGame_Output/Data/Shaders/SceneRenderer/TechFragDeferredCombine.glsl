#version 150 core

uniform sampler2D uColourTex;
uniform sampler2D uLightingTex;

uniform vec3  	  uAmbientColour;

in Vertex	{
	vec2 texCoord;
} IN;

out vec4 OutFrag;

void main(void)	{

	vec3  colour		= texture(uColourTex, IN.texCoord).xyz;
	vec2  light			= texture(uLightingTex, IN.texCoord).xy; //Diffuse factor / Specular factor

	//Colour Computations
	vec3 specColour = min(colour + vec3(0.5f), vec3(1)); //Quick hack to approximate specular colour of an object

	
//Lighting Combination
	vec3 finalColour = colour * (uAmbientColour + vec3(light.x))
						+ specColour * light.y * 0.6;

//Output Final L Colours
	OutFrag = vec4(finalColour, 1.0f);
}