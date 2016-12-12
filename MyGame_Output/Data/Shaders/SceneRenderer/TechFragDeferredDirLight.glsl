#version 150 core

uniform sampler2D uDepthTex;
uniform sampler2D uColourTex;
uniform sampler2D uNormalTex;

uniform mat4  uClipToWorldTransform;

uniform vec3  uInvLightDir;			//Directional Light
uniform vec3  uCameraPos;
uniform float uSpecularIntensity;

uniform int uShadowNum;
uniform sampler2DArrayShadow uShadowTex;
uniform mat4 uShadowTransform[16];
uniform vec2 uShadowSinglePixel;

in Vertex	{
	vec2 texCoord;
} IN;

out highp vec4 OutFrag;

const float BIAS = 0.15f;

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
		
	//tCoord.x += (noise(tsShadow.xy*1000.f)*2.f-1.f) * uShadowSinglePixel.x * 0.5f;
	//tCoord.y += (noise(tsShadow.yx*1500.f)*2.f-1.f) * uShadowSinglePixel.y * 0.5f;
	for (float y = -1.5f; y <= 1.5f; y += 1.0f)
		for (float x = -1.5f; x <= 1.5f; x += 1.0f)
			shadow += texture(uShadowTex, tCoord + vec4(uShadowSinglePixel.x * x, uShadowSinglePixel.y * y, 0, 0));
		
	return shadow / 16.0f;
}

void main(void)	{
	vec3  normal		= normalize(texture(uNormalTex, IN.texCoord).xyz * 2.0f - 1.0f);
	vec3  colour 		= texture(uColourTex, IN.texCoord).xyz;
	float depth 		= texture(uDepthTex, IN.texCoord).x;
	
	vec4 hwsPos = uClipToWorldTransform * (vec4(IN.texCoord.x, IN.texCoord.y, depth, 1.0) * 2.0 - 1.0);
	vec3 wsPos  = hwsPos.xyz / hwsPos.w;
	
	
//Shadow Calculations
vec4 shadowWsPos = vec4(wsPos + normal * BIAS, 1.0f);
	float shadow = 1.0f;
	vec3 shadowCol = vec3(0.0f);
	if (uShadowNum > 0)
	{
		shadow = 0.0f;	

		int i = uShadowNum - 1;
		for (; i >= 0; i--)
		{
			vec4 hcsShadow = uShadowTransform[i] * shadowWsPos;
			vec3 tsShadow = (hcsShadow.xyz / hcsShadow.w) * 0.5f + 0.5f;
			
			if (tsShadow.x >= 0.0f && tsShadow.x <= 1.0f
				&& tsShadow.y >= 0.0f && tsShadow.y <= 1.0f
				&& tsShadow.z >= -1.0f && tsShadow.z <= 1.0f)
			{
				shadow = shadowTest(tsShadow,i);
				break;
			}
		}

		if (i < 0) shadow = 1.0f; //Outside all shadowmaps - just pretend it's not shadowed
		shadow = max(shadow, 0.0f);
	}
	
//Lighting Calculations
	vec3 viewDir 		= normalize(uCameraPos - wsPos);
	vec3 halfDir 		= normalize(uInvLightDir + viewDir);
	float rFactor       = max(0.0, dot(halfDir , normal));
	
	float dFactor       = max(0.0, dot(uInvLightDir , normal)) * shadow;
    float sFactor       = pow(rFactor , uSpecularIntensity) * shadow;
	   
//Output Final L Colours
	OutFrag = vec4(dFactor, sFactor, 0.0, 1.0f);
}