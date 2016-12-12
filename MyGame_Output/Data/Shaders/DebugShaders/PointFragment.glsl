#version 150 core

in Vertex	{
	vec4 colour;
	vec2 rPos;
	float depth_min;
	float depth_max;
} IN;

out vec4 OutFrag;

void main(void)	{

	float distSq = dot(IN.rPos, IN.rPos);
	if (distSq > 1.f)
	{
		discard;
	}
	float z = sqrt(1.0f - distSq);
	
	float ndc_depth = mix(IN.depth_max, IN.depth_min, z);	
	float depth = (((gl_DepthRange.far-gl_DepthRange.near) * ndc_depth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
	gl_FragDepth = depth;
	
	OutFrag = IN.colour;
}