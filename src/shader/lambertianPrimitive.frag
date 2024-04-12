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
	// Assume a directional light along the camera's +z axis, intensity=0.99, ambient=0.01
	vec3 lightDir = -vec3(cameraParameters.view[0][2], cameraParameters.view[1][2], cameraParameters.view[2][2]);
	outColor.rgb = (0.99 * clamp(dot(normalize(inNormal), normalize(lightDir)), 0.0, 1.0) + 0.01) * inColor.rgb;
	outColor.a = inColor.a;
}