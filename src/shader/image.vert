/***********************************************************************
 * @file	image.vert
 * @author	jjyou
 * @date	2024-4-2
 * @brief	This file implements the vertex shader for displaying 2D images.
***********************************************************************/

#version 450

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

layout(location = 0) out vec2 outTexCoord;

void main() {
	gl_Position = ndcPositions[gl_VertexID];
	outTexCoord = texCoords[gl_VertexID];
}