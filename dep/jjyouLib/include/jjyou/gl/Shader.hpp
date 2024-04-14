/***********************************************************************
 * @file	Shader.hpp
 * @author	jjyou
 * @date	2023-6-3
 * @brief	This file implements Shader class, and provides some predefined
 *			shader program codes.
***********************************************************************/
#ifndef jjyou_gl_Shader_hpp
#define jjyou_gl_Shader_hpp

#include <glad/glad.h>
#include "../glsl/glsl.hpp"
#include <memory>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

namespace jjyou {
	namespace gl {

		/***********************************************************************
		 * @namespace jjyou::gl::ShaderCode
		 * @brief This namespace includes some predefined shader program codes.
		 ***********************************************************************/
		namespace ShaderCode {

			/** @name	color_P3C4
			  * @brief	Render color using vertex positions and colors.
			  * 
			  * @param[in] position			`layout(location = 0) in vec3`.
			  * @param[in] color			`layout(location = 1) in vec4`.
			  * @param[in] modelMatrix		`uniform mat4`. Default is identity matrix.
			  * @param[in] viewMatrix		`uniform mat4`. Default is identity matrix.
			  * @param[in] projectionMatrix	`uniform mat4`.
			  * @param[out] fColor			`layout(location = 0) out vec4`.
			  */
			//@{
			inline static const char
				* color_P3C4_v = \
				"#version 330 core\n"
				"layout(location = 0) in vec3 position;\n"
				"layout(location = 1) in vec4 color;\n"
				"out vec4 vColor;\n"
				"uniform mat4 modelMatrix = mat4(1.0f);\n"
				"uniform mat4 viewMatrix = mat4(1.0f);\n"
				"uniform mat4 projectionMatrix;\n"
				"void main() {\n"
				"    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);\n"
				"    vColor = color;\n"
				"}\n",
				* color_P3C4_f = \
				"#version 330 core\n"
				"in vec4 vColor;\n"
				"layout(location = 0) out vec4 fColor;\n"
				"void main() {\n"
				"    fColor = vColor;\n"
				"}\n";
			//@}

			/** @name	color_P3C4N3
			  * @brief	Render color using vertex positions, colors and normals.
			  *			There exists a point light source. Lighting will be disabled
			  *			when \p enableLight is `false`.
			  *
			  * @param[in] position			`layout(location = 0) in vec3`.
			  * @param[in] color			`layout(location = 1) in vec4`.
			  * @param[in] normal			`layout(location = 2) in vec3`.
			  * @param[in] modelMatrix		`uniform mat4`. Default is identity matrix.
			  * @param[in] viewMatrix		`uniform mat4`. Default is identity matrix.
			  * @param[in] projectionMatrix	`uniform mat4`.
			  * @param[in] normalMatrix		`uniform mat3`. It should be mat3(transpose(inverse(modelMatrix))). Default is identity matrix.
			  * @param[in] enableLight		`uniform bool`. Default is `true`.
			  * @param[in] lightPosition	`uniform vec3`. Default is zero vector.
			  * @param[in] viewPosition		`uniform vec3`. Default is zero vector.
			  * @param[in] lightColor		`uniform vec3`. Default is all-ones vector.
			  * @param[in] ambientStrength	`uniform float`. Default is 0.1.
			  * @param[in] specularStrength	`uniform float`. Default is 0.1.
			  * @param[in] shininess		`uniform float`. Default is 32.0.
			  * @param[out] fColor			`layout(location = 0) out vec4`.
			  */
			//@{
			inline static const char
				* color_P3C4N3_v = \
				"#version 330 core\n"
				"layout(location = 0) in vec3 position;\n"
				"layout(location = 1) in vec4 color;\n"
				"layout(location = 2) in vec3 normal;\n"
				"out vec3 vPosition;\n"
				"out vec4 vColor;\n"
				"out vec3 vNormal;\n"
				"uniform mat4 modelMatrix = mat4(1.0f);\n"
				"uniform mat4 viewMatrix = mat4(1.0f);\n"
				"uniform mat4 projectionMatrix;\n"
				"uniform mat4 normalMatrix = mat4(1.0f);\n"
				"void main() {\n"
				"    vPosition = vec3(modelMatrix * vec4(position, 1.0f));\n"
				"    gl_Position = projectionMatrix * viewMatrix * vec4(vPosition, 1.0f);\n"
				"    vColor = color;\n"
				"    vNormal = mat3(normalMatrix) * normal;\n"
				"}\n",
				* color_P3C4N3_f = \
				"#version 330 core\n"
				"in vec3 vPosition;\n"
				"in vec4 vColor;\n"
				"in vec3 vNormal;\n"
				"layout(location = 0) out vec4 fColor;\n"
				"uniform bool enableLight = true;\n"
				"uniform vec3 lightPosition = vec3(0.0f);\n"
				"uniform vec3 viewPosition = vec3(0.0f);\n"
				"uniform vec3 lightColor = vec3(1.0f);\n"
				"uniform float ambientStrength = 0.1f;\n"
				"uniform float specularStrength = 0.1f;\n"
				"uniform float shininess = 32.0f;\n"
				"void main() {\n"
				"    if (!enableLight) {\n"
				"        fColor = vColor;\n"
				"    }\n"
				"    else {\n"
				"        vec3 normal = normalize(vNormal);\n"
				"        //ambient\n"
				"        vec3 ambient = ambientStrength * lightColor;\n"
				"        //diffuse\n"
				"        vec3 lightDir = normalize(lightPosition - vPosition);\n"
				"        vec3 diffuse = max(dot(normal, lightDir), 0.0f) * lightColor;\n"
				"        //specular\n"
				"        vec3 viewDir = normalize(viewPosition - vPosition);\n"
				"        vec3 reflectDir = reflect(-lightDir, normal);\n"
				"        vec3 specular = specularStrength * pow(max(dot(viewDir, reflectDir), 0.0f), shininess) * lightColor;\n"
				"        //total\n"
				"        fColor = vec4((ambient + diffuse + specular) * vColor.rgb, vColor.a);\n"
				"    }\n"
				"};\n";
			//@}

			/** @name	color_P3C4N3_multiLights
			  * @brief	Render color using vertex positions, colors and normals.
			  *			There exists 4 point light sources, 4 directional light sources,
			  *			and 4 spotlight sources.
			  *
			  *			All types of lights have parameters called
			  *			\p enabled, \p lightColor, \p ambientStrength, and \p specularStrength
			  *			which determines whether to use that light source, the color of light,
			  *			the strength of ambient light and the strength of specular light. \n
			  *			Besides, point light types have parameters called
			  *			\p lightPosition, \p constant, \p linear, and \p quadratic. The first parameter
			  *			is the position of that light source, and the last three control the
			  *			light attenuation. \n
			  *			Directional light types have parameters called \p lightDirection. It is
			  *			the direction of that light source (pointing FROM the light source). \n
			  *			Spotlight types have parameters called
			  *			\p lightPosition, \p lightDirection, \p innerCosinCutOff, \p outerCosinCutOff,
			  *			\p constant, \p linear, and \p quadratic. \p lightPosition is the position of
			  *			that light source. \p lightDirection is the direction (pointing FROM the light source).
			  *			\p innerCosinCutOff and \p outerCosinCutOff control the size of the illumination region.
			  *			\p constant, \p linear and \p quadratic control the light attenuation. \n
			  *			All lights will be disabled when \p enableLight is `false`.
			  *
			  * @param[in] position			`layout(location = 0) in vec3`.
			  * @param[in] color			`layout(location = 1) in vec4`.
			  * @param[in] normal			`layout(location = 2) in vec3`.
			  * @param[in] modelMatrix		`uniform mat4`. Default is identity matrix.
			  * @param[in] viewMatrix		`uniform mat4`. Default is identity matrix.
			  * @param[in] projectionMatrix	`uniform mat4`.
			  * @param[in] normalMatrix		`uniform mat3`. It should be mat3(transpose(inverse(modelMatrix))). Default is identity matrix.
			  * @param[in] pointLight		`uniform PointLight[NUM_POINT_LIGHTS]`. An array of point lights.
			  *								By default, only `pointLight[0]` is enabled, with `lightPosition` equal to zero vector,
			  *								`lightColor` equal to all-ones vector, `ambientStrength` equal to 0.1,
			  *								`specularStrength` equal to 0.1, `constant` equal to 1.0, `linear` equal to 0.0,
			  *								`quadratic` equal to 0.0. In other words, it is a white point light source
			  *								located at the origin point without light attenuation.
			  * @param[in] directionalLight	`uniform DirectionalLight[NUM_DIRECTIONAL_LIGHTS]`. An array of directional lights.
			  *								By default, all directional lights are disabled.
			  * @param[in] spotLight		`uniform SpotLight[NUM_SPOT_LIGHTS]`. An array of spotlights.
			  *								By default, all spotlights are disabled.
			  * @param[in] enableLight		`uniform bool`. If it is `false` all lights will be disabled. Default is `true`.
			  * @param[in] viewPosition		`uniform vec3`. Default is zero vector.
			  * @param[in] shininess		`uniform float`. Default is 32.0.
			  * @param[out] fColor			`layout(location = 0) out vec4`.
			  */
			  //@{
			inline static const char
				* color_P3C4N3_multiLights_v = \
				"#version 330 core\n"
				"layout(location = 0) in vec3 position;\n"
				"layout(location = 1) in vec4 color;\n"
				"layout(location = 2) in vec3 normal;\n"
				"out vec3 vPosition;\n"
				"out vec4 vColor;\n"
				"out vec3 vNormal;\n"
				"uniform mat4 modelMatrix = mat4(1.0f);\n"
				"uniform mat4 viewMatrix = mat4(1.0f);\n"
				"uniform mat4 projectionMatrix;\n"
				"uniform mat3 normalMatrix = mat3(1.0f);\n"
				"void main() {\n"
				"    vPosition = vec3(modelMatrix * vec4(position, 1.0f));\n"
				"    gl_Position = projectionMatrix * viewMatrix * vec4(vPosition, 1.0f);\n"
				"    vColor = color;\n"
				"    vNormal = normalMatrix * normal;\n"
				"}\n",
				* color_P3C4N3_multiLights_f = \
				"#version 330 core\n"
				"#define NUM_POINT_LIGHTS 4\n"
				"#define NUM_DIRECTIONAL_LIGHTS 4\n"
				"#define NUM_SPOT_LIGHTS 4\n"
				"in vec3 vPosition;\n"
				"in vec4 vColor;\n"
				"in vec3 vNormal;\n"
				"layout(location = 0) out vec4 fColor;\n"
				"//Point light\n"
				"struct PointLight {\n"
				"    bool enabled;\n"
				"    vec3 lightPosition;\n"
				"    vec3 lightColor;\n"
				"    float ambientStrength;\n"
				"    float specularStrength;\n"
				"    float constant;\n"
				"    float linear;\n"
				"    float quadratic;\n"
				"};\n"
				"uniform PointLight pointLight[NUM_POINT_LIGHTS] = {\n"
				"    PointLight(true,  vec3(0.0f), vec3(1.0f), 0.1f, 0.1f, 1.0f, 0.0f, 0.0f),\n"
				"    PointLight(false, vec3(0.0f), vec3(1.0f), 0.1f, 0.1f, 1.0f, 0.0f, 0.0f),\n"
				"    PointLight(false, vec3(0.0f), vec3(1.0f), 0.1f, 0.1f, 1.0f, 0.0f, 0.0f),\n"
				"    PointLight(false, vec3(0.0f), vec3(1.0f), 0.1f, 0.1f, 1.0f, 0.0f, 0.0f) \n"
				"};\n"
				"//Directional light\n"
				"struct DirectionalLight {\n"
				"    bool enabled;\n"
				"    vec3 lightDirection;\n"
				"    vec3 lightColor;\n"
				"    float ambientStrength;\n"
				"    float specularStrength;\n"
				"};\n"
				"uniform DirectionalLight directionalLight[NUM_DIRECTIONAL_LIGHTS] = {\n"
				"    DirectionalLight(false, vec3(0.0f, 0.0f, -1.0f), vec3(1.0f), 0.1f, 0.1f),\n"
				"    DirectionalLight(false, vec3(0.0f, 0.0f, -1.0f), vec3(1.0f), 0.1f, 0.1f),\n"
				"    DirectionalLight(false, vec3(0.0f, 0.0f, -1.0f), vec3(1.0f), 0.1f, 0.1f),\n"
				"    DirectionalLight(false, vec3(0.0f, 0.0f, -1.0f), vec3(1.0f), 0.1f, 0.1f) \n"
				"};\n"
				"//Spotlight\n"
				"struct SpotLight {\n"
				"    bool enabled;\n"
				"    vec3 lightPosition;\n"
				"    vec3 lightDirection;\n"
				"    float innerCosinCutOff;\n"
				"    float outerCosinCutOff;\n"
				"    vec3 lightColor;\n"
				"    float ambientStrength;\n"
				"    float specularStrength;\n"
				"    float constant;\n"
				"    float linear;\n"
				"    float quadratic;\n"
				"};\n"
				"uniform SpotLight spotLight[NUM_SPOT_LIGHTS] = {\n"
				"    SpotLight(false, vec3(0.0f), vec3(0.0f, 0.0f, -1.0f), cos(radians(12.5f)), cos(radians(15.0f)), vec3(1.0f), 0.1f, 0.1f, 1.0f, 0.0f, 0.0f),\n"
				"    SpotLight(false, vec3(0.0f), vec3(0.0f, 0.0f, -1.0f), cos(radians(12.5f)), cos(radians(15.0f)), vec3(1.0f), 0.1f, 0.1f, 1.0f, 0.0f, 0.0f),\n"
				"    SpotLight(false, vec3(0.0f), vec3(0.0f, 0.0f, -1.0f), cos(radians(12.5f)), cos(radians(15.0f)), vec3(1.0f), 0.1f, 0.1f, 1.0f, 0.0f, 0.0f),\n"
				"    SpotLight(false, vec3(0.0f), vec3(0.0f, 0.0f, -1.0f), cos(radians(12.5f)), cos(radians(15.0f)), vec3(1.0f), 0.1f, 0.1f, 1.0f, 0.0f, 0.0f) \n"
				"};\n"
				"uniform bool enableLight = true;\n"
				"uniform vec3 viewPosition = vec3(0.0f);\n"
				"uniform float shininess = 32.0f;\n"
				"void main() {\n"
				"    if (!enableLight) {\n"
				"        fColor = vColor;\n"
				"    }\n"
				"    else {\n"
				"        vec3 normal = normalize(vNormal);\n"
				"        vec3 viewDir = normalize(viewPosition - vPosition);\n"
				"        vec3 total = vec3(0.0f);\n"
				"        //Point light\n"
				"        for (int i = 0; i < NUM_POINT_LIGHTS; i++)\n"
				"            if (pointLight[i].enabled) {\n"
				"                vec3 lightDir = normalize(pointLight[i].lightPosition - vPosition);\n"
				"                vec3 reflectDir = reflect(-lightDir, normal);\n"
				"                float distance = length(pointLight[i].lightPosition - vPosition);"
				"                float ambient = pointLight[i].ambientStrength;\n"
				"                float diffuse = max(dot(normal, lightDir), 0.0f);\n"
				"                float specular = pointLight[i].specularStrength * pow(max(dot(viewDir, reflectDir), 0.0f), shininess);\n"
				"                float attenuation = 1.0f / (pointLight[i].constant + pointLight[i].linear * distance + pointLight[i].quadratic * (distance * distance));\n"
				"                total += (ambient + diffuse + specular) * attenuation * pointLight[i].lightColor;\n"
				"            }\n"
				"        //Directional light\n"
				"        for (int i = 0; i < NUM_DIRECTIONAL_LIGHTS; i++)\n"
				"            if (directionalLight[i].enabled) {\n"
				"                vec3 lightDir = normalize(-directionalLight[i].lightDirection);\n"
				"                vec3 reflectDir = reflect(-lightDir, normal);\n"
				"                float ambient = directionalLight[i].ambientStrength;\n"
				"                float diffuse = max(dot(normal, lightDir), 0.0f);\n"
				"                float specular = directionalLight[i].specularStrength * pow(max(dot(viewDir, reflectDir), 0.0f), shininess);\n"
				"                total += (ambient + diffuse + specular) * directionalLight[i].lightColor;\n"
				"            }\n"
				"        //Spotlight\n"
				"        for (int i = 0; i < NUM_SPOT_LIGHTS; i++)\n"
				"            if (spotLight[i].enabled) {\n"
				"                vec3 lightDir = normalize(spotLight[i].lightPosition - vPosition);\n"
				"                float theta = dot(lightDir, normalize(-spotLight[i].lightDirection));\n"
				"                float epsilon = spotLight[i].innerCosinCutOff - spotLight[i].outerCosinCutOff;\n"
				"                float intensity = clamp((theta - spotLight[i].outerCosinCutOff) / epsilon, 0.0f, 1.0f);\n"
				"                vec3 reflectDir = reflect(-lightDir, normal);\n"
				"                float distance = length(spotLight[i].lightPosition - vPosition);"
				"                float ambient = spotLight[i].ambientStrength;\n"
				"                float diffuse = max(dot(normal, lightDir), 0.0f);\n"
				"                float specular = spotLight[i].specularStrength * pow(max(dot(viewDir, reflectDir), 0.0f), shininess);\n"
				"                float attenuation = 1.0f / (spotLight[i].constant + spotLight[i].linear * distance + spotLight[i].quadratic * (distance * distance));\n"
				"                total += (ambient + (diffuse + specular) * intensity)* attenuation * spotLight[i].lightColor;\n"
				"            }\n"
				"        //total\n"
				"        fColor = vec4(total * vColor.rgb, vColor.a);\n"
				"    }\n"
				"};\n";
			//@}

			/** @name	depth_P3
			  * @brief	Render depth using vertex positions.
			  *
			  * @param[in] position			`layout(location = 0) in vec3`.
			  * @param[in] modelMatrix		`uniform mat4`. Default is identity matrix.
			  * @param[in] viewMatrix		`uniform mat4`. Default is identity matrix.
			  * @param[in] projectionMatrix	`uniform mat4`.
			  * @param[out] fDepth			`layout(location = 1) out float`.
			  */
			//@{
			inline static const char
				* depth_P3_v = \
				"#version 330 core\n"
				"layout(location = 0) in vec3 position;\n"
				"out float vDepth;\n"
				"uniform mat4 modelMatrix = mat4(1.0f);\n"
				"uniform mat4 viewMatrix = mat4(1.0f);\n"
				"uniform mat4 projectionMatrix;\n"
				"void main() {\n"
				"    vec4 point = viewMatrix * modelMatrix * vec4(position, 1.0f);\n"
				"    gl_Position = projectionMatrix * point;\n"
				"    vDepth = -point.z / point.w;\n"
				"}\n",
				* depth_P3_f = \
				"#version 330 core\n"
				"in float vDepth;\n"
				"layout(location = 1) out float fDepth;\n"
				"void main() {\n"
				"    fDepth = vDepth;\n"
				"}\n";
			//@}

			/** @name	colorDepth_P3C4
			  * @brief	Render color and depth using vertex positions and colors.
			  *
			  * @param[in] position			`layout(location = 0) in vec3`.
			  * @param[in] color			`layout(location = 1) in vec4`.
			  * @param[in] modelMatrix		`uniform mat4`. Default is identity matrix.
			  * @param[in] viewMatrix		`uniform mat4`. Default is identity matrix.
			  * @param[in] projectionMatrix	`uniform mat4`.
			  * @param[out] fColor			`layout(location = 0) out vec4`.
			  * @param[out] fDepth			`layout(location = 1) out float`.
			  */
			//@{
			inline static const char
				* colorDepth_P3C4_v = \
				"#version 330 core\n"
				"layout(location = 0) in vec3 position;\n"
				"layout(location = 1) in vec4 color;\n"
				"out vec4 vColor;\n"
				"out float vDepth;\n"
				"uniform mat4 modelMatrix = mat4(1.0f);\n"
				"uniform mat4 viewMatrix = mat4(1.0f);\n"
				"uniform mat4 projectionMatrix;\n"
				"void main() {\n"
				"    vec4 point = viewMatrix * modelMatrix * vec4(position, 1.0f);\n"
				"    gl_Position = projectionMatrix * point;\n"
				"    vColor = color;\n"
				"    vDepth = -point.z / point.w;\n"
				"}\n",
				* colorDepth_P3C4_f = \
				"#version 330 core\n"
				"in vec4 vColor;\n"
				"in float vDepth;\n"
				"layout(location = 0) out vec4 fColor;\n"
				"layout(location = 1) out float fDepth;\n"
				"void main() {\n"
				"    fColor = vColor;\n"
				"    fDepth = vDepth;\n"
				"}\n";
			//@}

			/** @name	colorDepth_P3C4N3
			  * @brief	Render color and depth using vertex positions, colors and normals.
			  *			There exists a point light source. Lighting will be disabled
			  *			when \p enableLight is `false`.
			  *
			  * @param[in] position			`layout(location = 0) in vec3`.
			  * @param[in] color			`layout(location = 1) in vec4`.
			  * @param[in] normal			`layout(location = 2) in vec3`.
			  * @param[in] modelMatrix		`uniform mat4`. Default is identity matrix.
			  * @param[in] viewMatrix		`uniform mat4`. Default is identity matrix.
			  * @param[in] projectionMatrix	`uniform mat4`.
			  * @param[in] normalMatrix		`uniform mat3`. It should be mat3(transpose(inverse(modelMatrix))). Default is identity matrix.
			  * @param[in] enableLight		`uniform bool`. Default is `true`.
			  * @param[in] lightPosition	`uniform vec3`. Default is zero vector.
			  * @param[in] viewPosition		`uniform vec3`. Default is zero vector.
			  * @param[in] lightColor		`uniform vec3`. Default is all-ones vector.
			  * @param[in] ambientStrength	`uniform float`. Default is 0.1.
			  * @param[in] specularStrength	`uniform float`. Default is 0.1.
			  * @param[in] shininess		`uniform float`. Default is 32.0.
			  * @param[out] fColor			`layout(location = 0) out vec4`.
			  * @param[out] fDepth			`layout(location = 1) out float`.
			  */
			//@{
			inline static const char
				* colorDepth_P3C4N3_v = \
				"#version 330 core\n"
				"layout(location = 0) in vec3 position;\n"
				"layout(location = 1) in vec4 color;\n"
				"layout(location = 2) in vec3 normal;\n"
				"out vec3 vPosition;\n"
				"out vec4 vColor;\n"
				"out vec3 vNormal;\n"
				"out float vDepth;\n"
				"uniform mat4 modelMatrix = mat4(1.0f);\n"
				"uniform mat4 viewMatrix = mat4(1.0f);\n"
				"uniform mat4 projectionMatrix;\n"
				"uniform mat3 normalMatrix = mat3(1.0f);\n"
				"void main() {\n"
				"    vPosition = vec3(modelMatrix * vec4(position, 1.0f));\n"
				"    vec4 point = viewMatrix * vec4(vPosition, 1.0f);\n"
				"    gl_Position = projectionMatrix * point;\n"
				"    vColor = color;\n"
				"    vNormal = normalMatrix * normal;\n"
				"    vDepth = -point.z / point.w;\n"
				"}\n",
				* colorDepth_P3C4N3_f = \
				"#version 330 core\n"
				"in vec3 vPosition;\n"
				"in vec4 vColor;\n"
				"in vec3 vNormal;\n"
				"in float vDepth;\n"
				"layout(location = 0) out vec4 fColor;\n"
				"layout(location = 1) out float fDepth;\n"
				"uniform bool enableLight = true;\n"
				"uniform vec3 lightPosition = vec3(0.0f);\n"
				"uniform vec3 viewPosition = vec3(0.0f);\n"
				"uniform vec3 lightColor = vec3(1.0f);\n"
				"uniform float ambientStrength = 0.1f;\n"
				"uniform float specularStrength = 0.1f;\n"
				"uniform float shininess = 32.0f;\n"
				"void main() {\n"
				"    //color\n"
				"    if (!enableLight) {\n"
				"        fColor = vColor;\n"
				"    }\n"
				"    else {\n"
				"        vec3 normal = normalize(vNormal);\n"
				"        //ambient\n"
				"        vec3 ambient = ambientStrength * lightColor;\n"
				"        //diffuse\n"
				"        vec3 lightDir = normalize(lightPosition - vPosition);\n"
				"        vec3 diffuse = max(dot(normal, lightDir), 0.0f) * lightColor;\n"
				"        //specular\n"
				"        vec3 viewDir = normalize(viewPosition - vPosition);\n"
				"        vec3 reflectDir = reflect(-lightDir, normal);\n"
				"        vec3 specular = specularStrength * pow(max(dot(viewDir, reflectDir), 0.0f), shininess) * lightColor;\n"
				"        //total\n"
				"        fColor = vec4((ambient + diffuse + specular) * vColor.rgb, vColor.a);\n"
				"    }\n"
				"    //depth\n"
				"    fDepth = vDepth;\n"
				"}\n";
		}

		/***********************************************************************
		 * @class Shader
		 * @brief Shader class for loading and manipulating general-use shaders.
		 *
		 * This class provides C++ API for loading and manipulating general-use
		 * shaders. Users can either write their own shader codes, or use the
		 * default shader codes provided by this library.
		 ***********************************************************************/
		class Shader {
		public:

			/** @brief Type of shared pointer of Shader class.
			  */
			using Ptr = std::shared_ptr<Shader>;

			/** @brief Construct an empty shader.
			  */
			Shader(void);

			/** @brief Construct and load codes at construction time.
			  *
			  * @param vShaderCode	character buffer for vertex code.
			  * @param fShaderCode	character buffer for fragment code.
			  */
			Shader(const char* vShaderCode, const char* fShaderCode);

			/** @brief Construct and load codes at construction time.
			  *
			  * @param vertexPath	path to vertex code file.
			  * @param fragmentPath	path to fragment code file.
			  */
			Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);

			/** @brief Destructor.
			  */
			~Shader(void);

			/** @brief Check whether the shader is empty.
			  * 
			  * @return `true` if the shader is empty.
			  */
			bool empty(void) const;

			/** @brief Clear loaded program.
			  */
			void clear(void);

			/** @brief Load shader codes.
			  * 
			  * Load shader codes from character buffers. If the instance has
			  * already loaded codes before, these codes will be cleared before
			  * loading new codes.
			  * 
			  * @param vShaderCode	character buffer for vertex code.
			  * @param fShaderCode	character buffer for fragment code.
			  */
			void load(const char* vShaderCode, const char* fShaderCode);

			/** @brief Load shader codes.
			  *
			  * Load shader codes from files. If the instance has
			  * already loaded codes before, these codes will be cleared before
			  * loading new codes.
			  *
			  * @param vertexPath	path to vertex code file.
			  * @param fragmentPath	path to fragment code file.
			  */
			void load(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);

			/** @brief Activate the shader.
			  */
			void use() const;

			/** @brief Get the shader program's _id
			  * 
			  * @return The _id of the shader program.
			  */
			GLuint id() const;

			/** @name	uniform variable utility function
			  */
			//@{
			/** @brief Set `bool` uniform variable
			  */
			void setBool(const std::string& name, bool value) const {
				glUniform1i(glGetUniformLocation(this->_id, name.c_str()), (GLint)value);
			}

			/** @brief Set `bvec2` uniform variable
			  */
			void setBVec2(const std::string& name, const jjyou::glsl::bvec2& value) const {
				glUniform2i(glGetUniformLocation(this->_id, name.c_str()), (GLint)value[0], (GLint)value[1]);
			}

			/** @brief Set `bvec2` uniform variable
			  */
			void setBVec2(const std::string& name, bool x, bool y) const {
				glUniform2i(glGetUniformLocation(this->_id, name.c_str()), (GLint)x, (GLint)y);
			}

			/** @brief Set `bvec3` uniform variable
			  */
			void setBVec3(const std::string& name, const jjyou::glsl::bvec3& value) const {
				glUniform3i(glGetUniformLocation(this->_id, name.c_str()), (GLint)value[0], (GLint)value[1], (GLint)value[2]);
			}

			/** @brief Set `bvec3` uniform variable
			  */
			void setBVec3(const std::string& name, bool x, bool y, bool z) const {
				glUniform3i(glGetUniformLocation(this->_id, name.c_str()), (GLint)x, (GLint)y, (GLint)z);
			}

			/** @brief Set `bvec4` uniform variable
			  */
			void setBVec4(const std::string& name, const jjyou::glsl::bvec4& value) const {
				glUniform4i(glGetUniformLocation(this->_id, name.c_str()), (GLint)value[0], (GLint)value[1], (GLint)value[2], (GLint)value[3]);
			}

			/** @brief Set `bvec4` uniform variable
			  */
			void setBVec4(const std::string& name, bool x, bool y, bool z, bool w) const {
				glUniform4i(glGetUniformLocation(this->_id, name.c_str()), (GLint)x, (GLint)y, (GLint)z, (GLint)w);
			}

			/** @brief Set `int` uniform variable
			  */
			void setInt(const std::string& name, GLint value) const {
				glUniform1i(glGetUniformLocation(this->_id, name.c_str()), value);
			}

			/** @brief Set `ivec2` uniform variable
			  */
			void setIVec2(const std::string& name, const jjyou::glsl::ivec2& value) const {
				glUniform2iv(glGetUniformLocation(this->_id, name.c_str()), 1, &value[0]);
			}

			/** @brief Set `ivec2` uniform variable
			  */
			void setIVec2(const std::string& name, GLint x, GLint y) const {
				glUniform2i(glGetUniformLocation(this->_id, name.c_str()), x, y);
			}

			/** @brief Set `ivec3` uniform variable
			  */
			void setIVec3(const std::string& name, const jjyou::glsl::ivec3& value) const {
				glUniform3iv(glGetUniformLocation(this->_id, name.c_str()), 1, &value[0]);
			}

			/** @brief Set `ivec3` uniform variable
			  */
			void setIVec3(const std::string& name, GLint x, GLint y, GLint z) const {
				glUniform3i(glGetUniformLocation(this->_id, name.c_str()), x, y, z);
			}

			/** @brief Set `ivec4` uniform variable
			  */
			void setIVec4(const std::string& name, const jjyou::glsl::ivec4& value) const {
				glUniform4iv(glGetUniformLocation(this->_id, name.c_str()), 1, &value[0]);
			}

			/** @brief Set `ivec4` uniform variable
			  */
			void setIVec4(const std::string& name, GLint x, GLint y, GLint z, GLint w) const {
				glUniform4i(glGetUniformLocation(this->_id, name.c_str()), x, y, z, w);
			}

			/** @brief Set `uint` uniform variable
			  */
			void setUInt(const std::string& name, GLuint value) const {
				glUniform1ui(glGetUniformLocation(this->_id, name.c_str()), value);
			}

			/** @brief Set `uvec2` uniform variable
			  */
			void setUVec2(const std::string& name, const jjyou::glsl::uvec2& value) const {
				glUniform2uiv(glGetUniformLocation(this->_id, name.c_str()), 1, &value[0]);
			}

			/** @brief Set `uvec2` uniform variable
			  */
			void setUVec2(const std::string& name, GLuint x, GLuint y) const {
				glUniform2ui(glGetUniformLocation(this->_id, name.c_str()), x, y);
			}

			/** @brief Set `uvec3` uniform variable
			  */
			void setUVec3(const std::string& name, const jjyou::glsl::uvec3& value) const {
				glUniform3uiv(glGetUniformLocation(this->_id, name.c_str()), 1, &value[0]);
			}

			/** @brief Set `uvec3` uniform variable
			  */
			void setUVec3(const std::string& name, GLuint x, GLuint y, GLuint z) const {
				glUniform3ui(glGetUniformLocation(this->_id, name.c_str()), x, y, z);
			}

			/** @brief Set `uvec4` uniform variable
			  */
			void setUVec4(const std::string& name, const jjyou::glsl::uvec4& value) const {
				glUniform4uiv(glGetUniformLocation(this->_id, name.c_str()), 1, &value[0]);
			}

			/** @brief Set `uvec4` uniform variable
			  */
			void setUVec4(const std::string& name, GLuint x, GLuint y, GLuint z, GLuint w) const {
				glUniform4ui(glGetUniformLocation(this->_id, name.c_str()), x, y, z, w);
			}

			/** @brief Set `float` uniform variable
			  */
			void setFloat(const std::string& name, GLfloat value) const {
				glUniform1f(glGetUniformLocation(this->_id, name.c_str()), value);
			}

			/** @brief Set `vec2` uniform variable
			  */
			void setVec2(const std::string& name, const jjyou::glsl::vec2& value) const {
				glUniform2fv(glGetUniformLocation(this->_id, name.c_str()), 1, &value[0]);
			}

			/** @brief Set `vec2` uniform variable
			  */
			void setVec2(const std::string& name, GLfloat x, GLfloat y) const {
				glUniform2f(glGetUniformLocation(this->_id, name.c_str()), x, y);
			}

			/** @brief Set `vec3` uniform variable
			  */
			void setVec3(const std::string& name, const jjyou::glsl::vec3& value) const {
				glUniform3fv(glGetUniformLocation(this->_id, name.c_str()), 1, &value[0]);
			}

			/** @brief Set `vec3` uniform variable
			  */
			void setVec3(const std::string& name, GLfloat x, GLfloat y, GLfloat z) const {
				glUniform3f(glGetUniformLocation(this->_id, name.c_str()), x, y, z);
			}

			/** @brief Set `vec4` uniform variable
			  */
			void setVec4(const std::string& name, const jjyou::glsl::vec4& value) const {
				glUniform4fv(glGetUniformLocation(this->_id, name.c_str()), 1, &value[0]);
			}

			/** @brief Set `vec4` uniform variable
			  */
			void setVec4(const std::string& name, GLfloat x, GLfloat y, GLfloat z, GLfloat w) const {
				glUniform4f(glGetUniformLocation(this->_id, name.c_str()), x, y, z, w);
			}

			/** @brief Set `mat2` uniform variable
			  */
			void setMat2(const std::string& name, const jjyou::glsl::mat2& mat) const {
				glUniformMatrix2fv(glGetUniformLocation(this->_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
			}

			/** @brief Set `mat2x3` uniform variable
			  */
			void setMat2x3(const std::string& name, const jjyou::glsl::mat2x3& mat) const {
				glUniformMatrix2x3fv(glGetUniformLocation(this->_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
			}

			/** @brief Set `mat2x4` uniform variable
			  */
			void setMat2x4(const std::string& name, const jjyou::glsl::mat2x4& mat) const {
				glUniformMatrix2x4fv(glGetUniformLocation(this->_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
			}

			/** @brief Set `mat3` uniform variable
			  */
			void setMat3(const std::string& name, const jjyou::glsl::mat3& mat) const {
				glUniformMatrix3fv(glGetUniformLocation(this->_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
			}

			/** @brief Set `mat3x2` uniform variable
			  */
			void setMat3x2(const std::string& name, const jjyou::glsl::mat3x2& mat) const {
				glUniformMatrix3x2fv(glGetUniformLocation(this->_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
			}

			/** @brief Set `mat3x4` uniform variable
			  */
			void setMat3x4(const std::string& name, const jjyou::glsl::mat3x4& mat) const {
				glUniformMatrix3x4fv(glGetUniformLocation(this->_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
			}

			/** @brief Set `mat4` uniform variable
			  */
			void setMat4(const std::string& name, const jjyou::glsl::mat4& mat) const {
				glUniformMatrix4fv(glGetUniformLocation(this->_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
			}

			/** @brief Set `mat4x2` uniform variable
			  */
			void setMat4x2(const std::string& name, const jjyou::glsl::mat4x2& mat) const {
				glUniformMatrix4x2fv(glGetUniformLocation(this->_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
			}

			/** @brief Set `mat4x3` uniform variable
			  */
			void setMat4x3(const std::string& name, const jjyou::glsl::mat4x3& mat) const {
				glUniformMatrix4x3fv(glGetUniformLocation(this->_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
			}
			//@}

		private:
			GLuint _id;
			static void checkCompileErrors(GLuint shader, std::string type);
		};
	}
}


/*======================================================================
 | Implementation
 ======================================================================*/
 /// @cond


namespace jjyou {
	namespace gl {

		inline Shader::Shader(void) : _id(0) {}

		inline Shader::Shader(const char* vShaderCode, const char* fShaderCode) : _id(0) {
			this->load(vShaderCode, fShaderCode);
		}

		inline Shader::Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath) : _id(0) {
			this->load(vertexPath, fragmentPath);
		}

		inline Shader::~Shader(void) {
			this->clear();
		}

		inline bool Shader::empty(void) const {
			return (this->_id == 0);
		}

		inline void Shader::clear(void) {
			if (!this->empty()) {
				glDeleteProgram(this->_id);
				this->_id = 0;
			}
		}

		inline void Shader::load(const char* vShaderCode, const char* fShaderCode) {
			this->clear();
			unsigned int vertex, fragment;
			// vertex shader
			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &vShaderCode, nullptr);
			glCompileShader(vertex);
			this->checkCompileErrors(vertex, "VERTEX");
			// fragment Shader
			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment, 1, &fShaderCode, nullptr);
			glCompileShader(fragment);
			this->checkCompileErrors(fragment, "FRAGMENT");
			// shader Program
			this->_id = glCreateProgram();
			glAttachShader(this->_id, vertex);
			glAttachShader(this->_id, fragment);
			glLinkProgram(this->_id);
			this->checkCompileErrors(this->_id, "PROGRAM");
			// delete the shaders as they're linked into our program now and no longer necessary
			glDeleteShader(vertex);
			glDeleteShader(fragment);
		}

		inline void Shader::load(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath) {
			// retrieve the vertex/fragment source code from filePath
			std::string vertexCode;
			std::string fragmentCode;
			std::ifstream vShaderFile;
			std::ifstream fShaderFile;
			// ensure ifstream objects can throw exceptions:
			vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			try {
				// open files
				vShaderFile.open(vertexPath);
				fShaderFile.open(fragmentPath);
				std::stringstream vShaderStream, fShaderStream;
				// read file's buffer contents into streams
				vShaderStream << vShaderFile.rdbuf();
				fShaderStream << fShaderFile.rdbuf();
				// close file handlers
				vShaderFile.close();
				fShaderFile.close();
				// convert stream into string
				vertexCode = vShaderStream.str();
				fragmentCode = fShaderStream.str();
			}
			catch (std::ifstream::failure& e) {
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
			}
			const char* vShaderCode = vertexCode.c_str();
			const char* fShaderCode = fragmentCode.c_str();
			this->load(vShaderCode, fShaderCode);
		}

		inline void Shader::use() const {
			if (!this->empty())
				glUseProgram(this->_id);
		}

		inline GLuint Shader::id() const {
			return this->_id;
		}

		inline void Shader::checkCompileErrors(GLuint shader, std::string type) {
			GLint success;
			GLchar infoLog[1024];
			if (type != "PROGRAM") {
				glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
				if (!success) {
					glGetShaderInfoLog(shader, 1024, NULL, infoLog);
					std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				}
			}
			else {
				glGetProgramiv(shader, GL_LINK_STATUS, &success);
				if (!success) {
					glGetProgramInfoLog(shader, 1024, NULL, infoLog);
					std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				}
			}
		}
	}
}
/// @endcond

#endif /* jjyou_gl_Shader_hpp */
