/***********************************************************************
 * @file	surface.vert
 * @author	jjyou
 * @date	2024-4-2
 * @brief	This file implements the vertex shader for displaying a surface.
***********************************************************************/

#version 450

layout(set = 0, binding = 0) uniform CameraParameters {
	mat4 projection;
	mat4 view;
	vec4 viewPos;
} cameraParameters;

vec4 ndcPositions[6] = vec4[](
	vec4(-1.0, -1.0, 0.5, 1.0),
	vec4(-1.0, +1.0, 0.5, 1.0),
	vec4(+1.0, +1.0, 0.5, 1.0),

	vec4(-1.0, -1.0, 0.5, 1.0),
	vec4(+1.0, +1.0, 0.5, 1.0),
	vec4(+1.0, -1.0, 0.5, 1.0)
);

vec2 texCoords[6] = vec2[](
	vec2(0.0, 0.0),
	vec2(0.0, 1.0),
	vec2(1.0, 1.0),

	vec2(0.0, 0.0),
	vec2(1.0, 1.0),
	vec2(1.0, 0.0)
);

layout(location = 0) out vec3 outUnitDepthPosition;
layout(location = 1) out vec2 outTexCoord;

vec3 getOutUnitDepthPosition(void) {
	vec4 tmp = inverse(cameraParameters.projection * cameraParameters.view) * ndcPositions[gl_VertexIndex];
	return tmp.xyz / tmp.z;
}

void main() {
	gl_Position = ndcPositions[gl_VertexIndex];
	outUnitDepthPosition = getOutUnitDepthPosition();
	outTexCoord = texCoords[gl_VertexIndex];
}