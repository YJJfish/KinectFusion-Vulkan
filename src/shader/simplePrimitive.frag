/***********************************************************************
 * @file	simplePrimitive.frag
 * @author	jjyou
 * @date	2024-4-1
 * @brief	This file implements the fragment shader for simple primitives
 *			that have only position and color attributes. (E.g. axes, cameras.)
***********************************************************************/

#version 450

layout(location = 0) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = inColor;
}