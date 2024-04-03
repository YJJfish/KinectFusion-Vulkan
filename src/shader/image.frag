/***********************************************************************
 * @file	image.frag
 * @author	jjyou
 * @date	2024-4-2
 * @brief	This file implements the fragment shader for displaying 2D images.
***********************************************************************/

#version 450

layout (set = 0, binding = 0) uniform sampler2D imageColorTexture;
layout (set = 0, binding = 1) uniform sampler2D imageDepthTexture;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(imageColorTexture, inTexCoord);
	gl_FragDepth = texture(imageDepthTexture, inTexCoord).r;
}