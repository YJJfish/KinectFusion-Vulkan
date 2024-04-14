/***********************************************************************
 * @file	Objects.hpp
 * @author	jjyou
 * @date	2023-6-3
 * @brief	This file defines basic objects for rendering in OpenGL.
***********************************************************************/
#ifndef jjyou_gl_Objects_hpp
#define jjyou_gl_Objects_hpp

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <string>
#include <memory>
#include <cmath>
#include "Shader.hpp"
#include "../io/PlyFile.hpp"

namespace jjyou {
	namespace gl {

		class Object {
		public:
			using Ptr = std::shared_ptr<Object>;
			virtual constexpr std::string name(void) {
				return "Object";
			}
			Object(void) {}
			virtual ~Object(void) {}
			virtual void draw(void) = 0;
		};

		class Axis : public Object{
		private:
			GLuint VAO, VBO;
			const jjyou::gl::Shader& shader;
		public:
			using Ptr = std::shared_ptr<Axis>;
			virtual constexpr std::string name(void) {
				return "Axis";
			}
			Axis(const jjyou::gl::Shader& shader) : Object(), shader(shader) {
				//generate buffers
				glGenVertexArrays(1, &this->VAO);
				glGenBuffers(1, &this->VBO);
				//bind buffers
				glBindVertexArray(this->VAO);
				glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
				//set input format
				GLint positionLoc = glGetAttribLocation(this->shader.id(), "position");
				if (positionLoc != -1) {
					glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
					glEnableVertexAttribArray(positionLoc);
				}
				GLint colorLoc = glGetAttribLocation(this->shader.id(), "color");
				if (colorLoc != -1) {
					glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
					glEnableVertexAttribArray(colorLoc);
				}
				//set input data
				std::vector<std::array<GLfloat, 6>> vertexBuffer(6);
				vertexBuffer[0] = { {0.0f,0.0f,0.0f, 1.0f,0.0f,0.0f} };
				vertexBuffer[1] = { {1.0f,0.0f,0.0f, 1.0f,0.0f,0.0f} };
				vertexBuffer[2] = { {0.0f,0.0f,0.0f, 0.0f,1.0f,0.0f} };
				vertexBuffer[3] = { {0.0f,1.0f,0.0f, 0.0f,1.0f,0.0f} };
				vertexBuffer[4] = { {0.0f,0.0f,0.0f, 0.0f,0.0f,1.0f} };
				vertexBuffer[5] = { {0.0f,0.0f,1.0f, 0.0f,0.0f,1.0f} };
				glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(decltype(vertexBuffer)::value_type), vertexBuffer.data(), GL_STATIC_DRAW);
				//unbind
				glBindVertexArray(0);
			}
			virtual ~Axis(void) {
				glDeleteVertexArrays(1, &this->VAO);
				glDeleteBuffers(1, &this->VBO);
			}
			virtual void draw(void) {
				this->shader.use();
				glBindVertexArray(this->VAO);
				glDrawArrays(GL_LINES, 0, 6);
			}
		};

		class CameraFrame : public Object{
		private:
			GLuint VAO, VBO, EBO;
			const jjyou::gl::Shader& shader;
		public:
			using Ptr = std::shared_ptr<CameraFrame>;
			virtual constexpr std::string name(void) {
				return "CameraFrame";
			}
			CameraFrame(
				const jjyou::gl::Shader& shader,
				GLfloat xFov, GLfloat yFov
			) : Object(), shader(shader) {
				this->internalConstructor(
					1, 1,
					0.5f / std::tan(xFov / 2.0f), 0.0f,
					0.5f / std::tan(yFov / 2.0f), 0.0f
				);
			}
			CameraFrame(
				const jjyou::gl::Shader& shader,
				int frameWidth, int frameHeight,
				GLfloat fx, GLfloat cx,
				GLfloat fy, GLfloat cy
			) : Object(), shader(shader) {
				this->internalConstructor(
					frameWidth, frameHeight,
					fx, cx,
					fy, cy
				);
			}
			virtual ~CameraFrame(void) {
				glDeleteVertexArrays(1, &this->VAO);
				glDeleteBuffers(1, &this->VBO);
				glDeleteBuffers(1, &this->EBO);
			}
			virtual void draw(void) {
				this->shader.use();
				glBindVertexArray(this->VAO);
				glDrawElements(GL_LINES, 16, GL_UNSIGNED_INT, 0);
			}
		private:
			void internalConstructor(
				int frameWidth, int frameHeight,
				GLfloat fx, GLfloat cx,
				GLfloat fy, GLfloat cy
			) {
				//generate buffers
				glGenVertexArrays(1, &this->VAO);
				glGenBuffers(1, &this->VBO);
				glGenBuffers(1, &this->EBO);
				//bind buffers
				glBindVertexArray(this->VAO);
				glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
				//set input format
				GLint positionLoc = glGetAttribLocation(this->shader.id(), "position");
				if (positionLoc != -1) {
					glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
					glEnableVertexAttribArray(positionLoc);
				}
				GLint colorLoc = glGetAttribLocation(this->shader.id(), "color");
				if (colorLoc != -1) {
					glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
					glEnableVertexAttribArray(colorLoc);
				}
				//set input data
				std::vector<std::array<GLfloat, 6>> vertexBuffer(5);
				vertexBuffer[0] = { {0.0f,0.0f,0.0f, 1.0f,1.0f,1.0f} };
				vertexBuffer[1] = { {(-0.5f - cx) / fx,-(-0.5f - cy) / fy,1.0f, 1.0f,1.0f,1.0f} };
				vertexBuffer[2] = { {(-0.5f - cx) / fx,-(frameHeight - 0.5f - cy) / fy,1.0f, 1.0f,1.0f,1.0f} };
				vertexBuffer[3] = { {(frameWidth - 0.5f - cx) / fx,-(frameHeight - 0.5f - cy) / fy,1.0f, 1.0f,1.0f,1.0f} };
				vertexBuffer[4] = { {(frameWidth - 0.5f - cx) / fx,-(-0.5f - cy) / fy,1.0f, 1.0f,1.0f,1.0f} };
				glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(decltype(vertexBuffer)::value_type), vertexBuffer.data(), GL_STATIC_DRAW);
				std::vector<unsigned int> indexBuffer = {
					0, 1,
					0, 2,
					0, 3,
					0, 4,
					1, 2,
					2, 3,
					3, 4,
					4, 1
				};
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(decltype(indexBuffer)::value_type), indexBuffer.data(), GL_STATIC_DRAW);
				//unbind
				glBindVertexArray(0);
			}
		};

		class Sphere : public Object {
		private:
			GLuint VAO, VBO, EBO;
			const jjyou::gl::Shader& shader;
			size_t numSectors, numStacks;
		public:
			using Ptr = std::shared_ptr<Sphere>;
			virtual constexpr std::string name(void) {
				return "Sphere";
			}
			Sphere(
				const jjyou::gl::Shader& shader,
				size_t numSectors, size_t numStacks,
				const std::array<GLfloat, 4> color = { {1.0f, 1.0f, 1.0f, 1.0f} }
			) : Object(), shader(shader), numSectors(numSectors), numStacks(numStacks) {
				std::vector<std::array<GLfloat, 3>> vertexBuffer((numSectors + 1) * (numStacks + 1));
				for (int y = 0; y <= numStacks; y++) {
					GLfloat pitch = std::numbers::pi_v<GLfloat> *((GLfloat)y / numStacks - 0.5);
					for (int x = 0; x <= numSectors; x++) {
						GLfloat yaw = 2 * std::numbers::pi_v<GLfloat> * x / numSectors;
						vertexBuffer[y * (numSectors + 1) + x] = {{
							cos(pitch) * cos(yaw),
							sin(pitch),
							cos(pitch) * sin(yaw)
						} };
					}
				}
				std::vector<unsigned int> indexBuffer(numSectors * numStacks * 6);
				for (int y = 0; y < numStacks; y++) {
					for (int x = 0; x < numSectors; x++) {
						indexBuffer[(y * numSectors + x) * 6 + 0] = y * (numSectors + 1) + x;
						indexBuffer[(y * numSectors + x) * 6 + 1] = (y + 1) * (numSectors + 1) + x;
						indexBuffer[(y * numSectors + x) * 6 + 2] = y * (numSectors + 1) + x + 1;
						indexBuffer[(y * numSectors + x) * 6 + 3] = y * (numSectors + 1) + x + 1;
						indexBuffer[(y * numSectors + x) * 6 + 4] = (y + 1) * (numSectors + 1) + x;
						indexBuffer[(y * numSectors + x) * 6 + 5] = (y + 1) * (numSectors + 1) + x + 1;
					}
				}
				//generate buffers
				glGenVertexArrays(1, &this->VAO);
				glGenBuffers(1, &this->VBO);
				glGenBuffers(1, &this->EBO);
				//bind buffers
				glBindVertexArray(this->VAO);
				glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
				//set input format
				GLint positionLoc = glGetAttribLocation(this->shader.id(), "position");
				if (positionLoc != -1) {
					glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
					glEnableVertexAttribArray(positionLoc);
				}
				GLint colorLoc = glGetAttribLocation(this->shader.id(), "color");
				if (colorLoc != -1) {
					glVertexAttrib4fv(colorLoc, &color[0]);
				}
				GLint normalLoc = glGetAttribLocation(this->shader.id(), "normal");
				if (normalLoc != -1) {
					glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
					glEnableVertexAttribArray(normalLoc);
				}
				//set input data
				glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(decltype(vertexBuffer)::value_type), vertexBuffer.data(), GL_STATIC_DRAW);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(decltype(indexBuffer)::value_type), indexBuffer.data(), GL_STATIC_DRAW);
				//unbind
				glBindVertexArray(0);
			}
			void setColor(const std::array<GLfloat, 4> color) {
				glBindVertexArray(this->VAO);
				GLint colorLoc = glGetAttribLocation(this->shader.id(), "color");
				if (colorLoc != -1) {
					glVertexAttrib4fv(colorLoc, &color[0]);
				}
				glBindVertexArray(0);
			}
			virtual ~Sphere(void) {
				glDeleteVertexArrays(1, &this->VAO);
				glDeleteBuffers(1, &this->VBO);
				glDeleteBuffers(1, &this->EBO);
			}
			virtual void draw(void) {
				this->shader.use();
				glBindVertexArray(this->VAO);
				glDrawElements(GL_TRIANGLES, this->numSectors * this->numStacks * 6, GL_UNSIGNED_INT, 0);
			}
		};

		class PolygonMesh : public Object {
		private:
			GLuint VAO, VBO, edgeEBO, faceEBO, faceEdgeEBO;
			const jjyou::gl::Shader& shader;
			size_t numVertices, numEdges, numFaceEdges, numFaces;
			int drawMode;
		public:
			using Ptr = std::shared_ptr<PolygonMesh>;
			enum {
				None = 0b0000,
				DrawPoints = 0b0001,
				DrawEdges = 0b0010,
				DrawFaceEdges = 0b0100,
				DrawFaces = 0b1000
			};
			virtual constexpr std::string name(void) {
				return "PolygonMesh";
			}
			template <class VertexTy, class ColorTy, bool HasAlpha>
			PolygonMesh(
				const jjyou::gl::Shader& shader,
				const io::PlyFile<VertexTy, ColorTy, HasAlpha>& plyFile
			) : Object(),
				shader(shader),
				numVertices(plyFile.vertex.size()),
				numEdges(plyFile.edge.size()),
				numFaceEdges(0),
				numFaces(0),
				drawMode((plyFile.face.size() ? DrawFaces : DrawPoints) | (plyFile.edge.size() ? DrawEdges : None))
			{
				//generate vertex array
				glGenVertexArrays(1, &this->VAO);
				glBindVertexArray(this->VAO);
				//generate array buffer for vertices
				glGenBuffers(1, &this->VBO);
				glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
				size_t vertexAttributesSize = 3;
				if (plyFile.vertexNormal.size() == plyFile.vertex.size())
					vertexAttributesSize += 3;
				if (plyFile.vertexColor.size() == plyFile.vertex.size())
					vertexAttributesSize += (HasAlpha ? 4 : 3);
				std::vector<GLfloat> vertexBuffer;
				vertexBuffer.reserve(vertexAttributesSize * plyFile.vertex.size());
				for (int cnt = 0; cnt < plyFile.vertex.size(); cnt++) {
					for (int i = 0; i < 3; i++)
						vertexBuffer.push_back(static_cast<GLfloat>(plyFile.vertex[cnt][i]));
					if (plyFile.vertexNormal.size() == plyFile.vertex.size())
						for (int i = 0; i < 3; i++)
							vertexBuffer.push_back(static_cast<GLfloat>(plyFile.vertexNormal[cnt][i]));
					if (plyFile.vertexColor.size() == plyFile.vertex.size())
						for (int i = 0; i < (HasAlpha ? 4 : 3); i++)
							vertexBuffer.push_back(utils::color_cast<ColorTy, GLfloat>(plyFile.vertexColor[cnt][i]));
				}
				glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(decltype(vertexBuffer)::value_type), vertexBuffer.data(), GL_STATIC_DRAW);
				//set input format
				GLint positionLoc = glGetAttribLocation(this->shader.id(), "position");
				if (positionLoc != -1) {
					glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, vertexAttributesSize * sizeof(GLfloat), 0);
					glEnableVertexAttribArray(positionLoc);
				}
				if (plyFile.vertexNormal.size() == plyFile.vertex.size()) {
					GLint normalLoc = glGetAttribLocation(this->shader.id(), "normal");
					if (normalLoc != -1) {
						glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, vertexAttributesSize * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
						glEnableVertexAttribArray(normalLoc);
					}
				}
				if (plyFile.vertexColor.size() == plyFile.vertex.size()) {
					GLint colorLoc = glGetAttribLocation(this->shader.id(), "color");
					if (colorLoc != -1) {
						glVertexAttribPointer(colorLoc, HasAlpha ? 4 : 3, GL_FLOAT, GL_FALSE, vertexAttributesSize * sizeof(GLfloat), (void*)((3 + ((plyFile.vertexNormal.size() == plyFile.vertex.size()) ? 3 : 0)) * sizeof(GLfloat)));
						glEnableVertexAttribArray(colorLoc);
					}
				}
				//generate index buffer for edges
				glGenBuffers(1, &this->edgeEBO);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->edgeEBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, plyFile.edge.size() * sizeof(decltype(plyFile.edge)::value_type), plyFile.edge.data(), GL_STATIC_DRAW);
				//generate index buffer for face edges and faces
				glGenBuffers(1, &this->faceEdgeEBO);
				glGenBuffers(1, &this->faceEBO);
				for (const auto& f : plyFile.face)
					if (f.size() >= 3)
						this->numFaces += f.size() - 2;
				this->numFaceEdges = this->numFaces * 3;
				std::vector<int> faceBuffer, faceEdgeBuffer;
				faceBuffer.reserve(this->numFaces * 3);
				faceEdgeBuffer.reserve(this->numFaceEdges * 2);
				for (const auto& f : plyFile.face)
					if (f.size() >= 3)
						for (int i = 1; i < f.size() - 1; i++) {
							faceBuffer.push_back(f.front());
							faceBuffer.push_back(f[i]);
							faceBuffer.push_back(f[i + 1]);
							faceEdgeBuffer.push_back(f.front());
							faceEdgeBuffer.push_back(f[i]);
							faceEdgeBuffer.push_back(f[i]);
							faceEdgeBuffer.push_back(f[i + 1]);
							faceEdgeBuffer.push_back(f[i + 1]);
							faceEdgeBuffer.push_back(f.front());
						}
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->faceEdgeEBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceEdgeBuffer.size() * sizeof(decltype(faceEdgeBuffer)::value_type), faceEdgeBuffer.data(), GL_STATIC_DRAW);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->faceEBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceBuffer.size() * sizeof(decltype(faceBuffer)::value_type), faceBuffer.data(), GL_STATIC_DRAW);
			}
			virtual ~PolygonMesh(void) {
				glDeleteVertexArrays(1, &this->VAO);
				glDeleteBuffers(1, &this->VBO);
				glDeleteBuffers(1, &this->edgeEBO);
				glDeleteBuffers(1, &this->faceEBO);
				glDeleteBuffers(1, &this->faceEdgeEBO);
			}
			void setDrawMode(int drawMode) {
				this->drawMode = drawMode;
			}
			int getDrawMode(void) {
				return this->drawMode;
			}
			void setColor(const std::array<GLfloat, 4> color) {
				glBindVertexArray(this->VAO);
				GLint colorLoc = glGetAttribLocation(this->shader.id(), "color");
				if (colorLoc != -1) {
					glDisableVertexAttribArray(colorLoc);
					glVertexAttrib4fv(colorLoc, &color[0]);
				}
				glBindVertexArray(0);
			}
			void draw(int drawMode) {
				this->drawMode = drawMode;
				this->draw();
			}
			virtual void draw(void) {
				this->shader.use();
				glBindVertexArray(this->VAO);
				if (this->drawMode & DrawPoints)
					glDrawArrays(GL_POINTS, 0, this->numVertices);
				if (this->drawMode & DrawEdges) {
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->edgeEBO);
					glDrawElements(GL_LINES, this->numEdges * 2, GL_UNSIGNED_INT, 0);
				}
				if (this->drawMode & DrawFaceEdges) {
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->faceEdgeEBO);
					glDrawElements(GL_LINES, this->numFaceEdges * 2, GL_UNSIGNED_INT, 0);
				}
				if (this->drawMode & DrawFaces) {
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->faceEBO);
					glDrawElements(GL_TRIANGLES, this->numFaces * 3, GL_UNSIGNED_INT, 0);
				}
			}
		};
	}
}

#endif /* jjyou_gl_Objects_hpp */