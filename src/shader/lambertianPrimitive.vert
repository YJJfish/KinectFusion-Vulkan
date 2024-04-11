/***********************************************************************
 * @file	lambertianPrimitive.vert
 * @author	jjyou
 * @date	2024-4-2
 * @brief	This file implements the vertex shader for lambertian primitives
 *			that have position, normal and color attributes.
***********************************************************************/

#version 450

layout(set = 0, binding = 0) uniform CameraParameters {
	mat4 projection;
	mat4 view;
	vec4 viewPos;
} cameraParameters;

layout(set = 1, binding = 0) uniform ModelTransforms {
	mat4 model;
	mat4 normal;
} modelTransforms;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec4 outColor;

void main() {
	gl_Position = cameraParameters.projection * cameraParameters.view * modelTransforms.model * vec4(inPosition, 1.0);
	gl_PointSize = 2.0;
	outPosition = vec3(modelTransforms.model * vec4(inPosition, 1.0));
	outNormal = mat3(modelTransforms.normal) * inNormal;
	outColor = inColor;
}