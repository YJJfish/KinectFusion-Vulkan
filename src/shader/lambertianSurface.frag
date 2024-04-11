/***********************************************************************
 * @file	simpleSurface.frag
 * @author	jjyou
 * @date	2024-4-8
 * @brief	This file implements the fragment shader for displaying a lambertian
 *			surface that have color, depth and normal textures.
***********************************************************************/

#version 450

layout(set = 0, binding = 0) uniform CameraParameters {
	mat4 projection;
	mat4 view;
	vec4 viewPos;
} cameraParameters;

layout (set = 1, binding = 0) uniform sampler2D surfaceColorTexture;
layout (set = 1, binding = 1) uniform sampler2D surfaceDepthTexture;
layout (set = 1, binding = 2) uniform sampler2D surfaceNormalTexture;

layout(location = 0) in vec3 inUnitDepthPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

vec3 getInPosition(float inDepth) {
	return inDepth * (inUnitDepthPosition - cameraParameters.viewPos.xyz) + cameraParameters.viewPos.xyz;
}

void main() {
	vec4 inColor = texture(surfaceColorTexture, inTexCoord);
	if (inColor.a <= 0.5)
		discard;
	vec3 inNormal = texture(surfaceNormalTexture, inTexCoord).rgb;
	float inDepth = texture(surfaceDepthTexture, inTexCoord).r;
	vec3 inPosition = getInPosition(inDepth);
	// Assume a directional light along the camera's +z axis
	vec3 lightDir = -vec3(cameraParameters.view[0][2], cameraParameters.view[1][2], cameraParameters.view[2][2]);
	outColor.rgb = clamp(dot(normalize(inNormal), normalize(lightDir)), 0.0, 1.0) * inColor.rgb;
	outColor.a = inColor.a;
	gl_FragDepth = (cameraParameters.projection[2][2] * inDepth + cameraParameters.projection[3][2]) / (cameraParameters.projection[2][3] * inDepth + cameraParameters.projection[3][3]);
}