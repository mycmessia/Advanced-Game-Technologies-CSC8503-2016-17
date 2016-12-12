#version 150 core

uniform mat4 uModelMtx;
uniform mat4 uProjViewMtx;
uniform vec4 uColour;

in  vec3 position;
in  vec2 texCoord;
in  vec3 normal;

out Vertex	{
	vec4 worldPos;
	vec2 texCoord;
	vec4 colour;
	vec3 normal;
} OUT;

void main(void)	{
	vec4 wp 		= uModelMtx * vec4(position, 1.0);
	gl_Position		= uProjViewMtx * wp;
	
	OUT.worldPos 	= wp;
	OUT.texCoord	= texCoord;
	OUT.colour		= uColour;
	
	//This is a much quicker way to calculate the rotated normal value, however it only works
	//  when the model matrix has the same scaling on all axis. If this is not the case, use the other method below.
	//OUT.normal		= mat3(uModelMtx) * normal;
	
	// Use this if your objects have different scaling values for the x,y,z axis
	OUT.normal		  = transpose(inverse(mat3(uModelMtx))) * normalize(normal);
}