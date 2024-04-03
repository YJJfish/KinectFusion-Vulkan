/***********************************************************************
 * @file	simplePrimitive.vert
 * @author	jjyou
 * @date	2024-4-1
 * @brief	This file implements the vertex shader for simple primitives
 *			that have only position and color attributes. (E.g. axes, cameras.)
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
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() {
	gl_Position = cameraParameters.projection * cameraParameters.view * modelTransforms.model * vec4(inPosition, 1.0);
	outColor = inColor;
}