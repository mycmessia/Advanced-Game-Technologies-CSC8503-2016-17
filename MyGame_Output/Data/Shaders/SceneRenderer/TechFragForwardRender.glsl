#version 150 core

uniform sampler2D uDiffuseTex;

uniform vec3  	uInvLightDir;			//Directional Light
uniform vec3  	uCameraPos;
uniform float 	uSpecularIntensity;
uniform vec3  	uAmbientColour;

uniform int 				 uShadowNum;
uniform sampler2DArrayShadow uShadowTex;		
uniform mat4 				 uShadowTransform[16];	//Probably should be a ubo
uniform vec2 				 uShadowSinglePixel;

in Vertex	{
	vec4 worldPos;
	vec2 texCoord;
	vec4 colour;
	vec3 normal;
} IN;

out vec4 OutFrag;

const float BIAS = 0.01f;
float shadowTest(vec3 tsShadow, int tsLayer)
{	
	vec4 tCoord;
	tCoord.xyw = tsShadow;	//I don't know why...
	tCoord.z = tsLayer;

	/*
	PCF filtering
	  - Takes a 4x4 sample around each pixel and averages the result, bluring the edges of shadows
	  - Requires 16 linearly interpolated samples thus taking a very long time. 
	  - Considering looking into exponential shadow mapping as a nicer looking and faster way to achieve soft shadowing.
	 */
	float shadow = 0.0f;
		
	for (float y = -1.5f; y <= 1.5f; y += 1.0f)
		for (float x = -1.5f; x <= 1.5f; x += 1.0f)
			shadow += texture(uShadowTex, tCoord + vec4(uShadowSinglePixel.x * x, uShadowSinglePixel.y * y, 0, -BIAS));
		
	return shadow / 16.0f;
}


void main(void)	{
	vec3 normal 		= normalize(IN.normal);
	vec4 texColour  	= texture(uDiffuseTex, IN.texCoord);
	vec4 colour 		= IN.colour * texColour;
	vec3 wsPos 			= IN.worldPos.xyz / IN.worldPos.w;
	
	//Shadow Calculations
	vec4 shadowWsPos = vec4(wsPos + normal * 0.2f, 1.0f);
	
	
	float shadow = 1.0f;
	if (uShadowNum > 0)
	{
		shadow = 0.0f;
		for (int i = uShadowNum - 1; i >= 0; i--)
		{
			vec4 hcsShadow = uShadowTransform[i] * shadowWsPos;
			vec3 tsShadow = (hcsShadow.xyz / hcsShadow.w) * 0.5f + 0.5f;
			
			if (tsShadow.x >= 0.0f && tsShadow.x <= 1.0f
				&& tsShadow.y >= 0.0f && tsShadow.y <= 1.0f)
			{
				shadow += shadowTest(tsShadow,i);
				break;
			}
		}

		shadow = max(shadow, 0.0f);
	}
	
//Lighting Calculations
	vec3 viewDir 		= normalize(uCameraPos - wsPos );
	vec3 halfDir 		= normalize(uInvLightDir + viewDir);
	float rFactor       = max(0.0, dot(halfDir , normal ));
	
	float dFactor       = max(0.0, dot(uInvLightDir , normal )) ;
    float sFactor       = pow(rFactor , uSpecularIntensity );
	   
//Colour Computations
	vec3 specColour = min(colour.rgb + vec3(0.5f), vec3(1)); //Quick hack to approximate specular colour of an object, assuming the light colour is white
	
//Output Final Colour
	vec3 diffuse = colour.rgb * dFactor * shadow;
	vec3 specular = specColour * sFactor * shadow;
	
	OutFrag.xyz 	= colour.rgb * uAmbientColour + (diffuse + specular * 0.5f) * (vec3(1) - uAmbientColour);
	OutFrag.a 		= colour.a;
}