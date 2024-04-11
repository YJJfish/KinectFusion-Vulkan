/***********************************************************************
 * @file	simpleSurface.frag
 * @author	jjyou
 * @date	2024-4-2
 * @brief	This file implements the fragment shader for displaying a simple
 *			surface that have color and depth textures.
***********************************************************************/

#version 450

layout(set = 0, binding = 0) uniform CameraParameters {
	mat4 projection;
	mat4 view;
	vec4 viewPos;
} cameraParameters;

layout (set = 1, binding = 0) uniform sampler2D surfaceColorTexture;
layout (set = 1, binding = 1) uniform sampler2D surfaceDepthTexture;

layout(location = 0) in vec3 inUnitDepthPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

vec3 getInPosition(float inDepth) {
	return inDepth * (inUnitDepthPosition - cameraParameters.viewPos.xyz) + cameraParameters.viewPos.xyz;
}

void main() {
	vec4 inColor = texture(surfaceColorTexture, inTexCoord);
	float inDepth = texture(surfaceDepthTexture, inTexCoord).r;
	vec3 inPosition = getInPosition(inDepth);
	outColor = inColor;
	gl_FragDepth = (cameraParameters.projection[2][2] * inDepth + cameraParameters.projection[3][2]) / (cameraParameters.projection[2][3] * inDepth + cameraParameters.projection[3][3]);
}