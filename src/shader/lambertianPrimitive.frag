/***********************************************************************
 * @file	lambertianPrimitive.frag
 * @author	jjyou
 * @date	2024-4-2
 * @brief	This file implements the fragment shader for lambertian primitives
 *			that have position, normal and color attributes.
***********************************************************************/

#version 450

layout(set = 0, binding = 0) uniform CameraParameters {
	mat4 projection;
	mat4 view;
	vec4 viewPos;
} cameraParameters;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() {
	// Assume a directional light along the camera's +z axis
	outColor = clamp(dot(normalize(inNormal), normalize(cameraParameters.viewPos - inPosition)), 0.0, 1.0) * inColor;
}