/***********************************************************************
 * @file	FrameBuffer.hpp
 * @author	jjyou
 * @date	2023-5-23
 * @brief	This file implements FrameBuffer class,
 *			a wrapper class for OpenGL framebuffer object.
***********************************************************************/
#ifndef jjyou_gl_FrameBuffer_hpp
#define jjyou_gl_FrameBuffer_hpp

#include <glad/glad.h>
#include <memory>
#include <vector>

namespace jjyou {
	namespace gl {

		class FrameBuffer {
		public:
			using Ptr = std::shared_ptr<FrameBuffer>;
			enum class AttachType {
				NotAttached = 0,
				Texture2D = 1,
				RenderBuffer = 2
			};
			//https://docs.gl/es3/glTexImage2D
			//https://docs.gl/gl4/glTexImage2D
			struct Attachment {
				AttachType attachType;
				GLuint index;
				GLenum internalFormat;
				GLenum format;
				GLenum type;
				Attachment(void) {
					this->reset();
				}
				void set(AttachType attachType, GLuint index, GLenum internalFormat, GLenum format, GLenum type) {
					this->attachType = attachType;
					this->index = index;
					this->internalFormat = internalFormat;
					this->format = format;
					this->type = type;
				}
				void reset(void) {
					this->attachType = AttachType::NotAttached;
					this->index = 0;
					this->internalFormat = 0;
					this->format = 0;
					this->type = 0;
				}
			};
			FrameBuffer(GLsizei width, GLsizei height) :
				width(width), height(height)
			{
				glGenFramebuffers(1, &this->fbo);
				glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &this->maxAttachments);
				this->colorAttachments.resize(this->maxAttachments);
			}
			void bind(void) {
				glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
			}
			bool resize(GLsizei width, GLsizei height) {
				if (width <= 0 || height <= 0)
					return false;
				if (this->width == width && this->height == height)
					return true;
				this->width = width;
				this->height = height;
				this->bind();
				//color
				for (int index = 0; index < this->maxAttachments; index++) {
					Attachment& colorAttachment = this->colorAttachments[index];
					if (colorAttachment.attachType == AttachType::Texture2D) {
						glBindTexture(GL_TEXTURE_2D, colorAttachment.index);
						//TODO: use glTexStorage2D sometimes
						glTexImage2D(GL_TEXTURE_2D, 0, colorAttachment.internalFormat, this->width, this->height, 0, colorAttachment.format, colorAttachment.type, nullptr);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, colorAttachment.index, 0);
					}
					else if (colorAttachment.attachType == AttachType::RenderBuffer) {
						glBindRenderbuffer(GL_RENDERBUFFER, colorAttachment.index);
						glRenderbufferStorage(GL_RENDERBUFFER, colorAttachment.format, this->width, this->height);
						glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, colorAttachment.index);
					}
				}
				//depth
				if (this->depthAttachment.attachType == AttachType::Texture2D) {
					glBindTexture(GL_TEXTURE_2D, this->depthAttachment.index);
					glTexImage2D(GL_TEXTURE_2D, 0, this->depthAttachment.internalFormat, this->width, this->height, 0, this->depthAttachment.format, this->depthAttachment.type, nullptr);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depthAttachment.index, 0);
				}
				else if (this->depthAttachment.attachType == AttachType::RenderBuffer) {
					glBindRenderbuffer(GL_RENDERBUFFER, this->depthAttachment.index);
					glRenderbufferStorage(GL_RENDERBUFFER, this->depthAttachment.format, this->width, this->height);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->depthAttachment.index);
				}
				//stencil
				if (this->stencilAttachment.attachType == AttachType::Texture2D) {
					glBindTexture(GL_TEXTURE_2D, this->stencilAttachment.index);
					glTexImage2D(GL_TEXTURE_2D, 0, this->stencilAttachment.internalFormat, this->width, this->height, 0, this->stencilAttachment.format, this->stencilAttachment.type, nullptr);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, this->stencilAttachment.index, 0);
				}
				else if (this->stencilAttachment.attachType == AttachType::RenderBuffer) {
					glBindRenderbuffer(GL_RENDERBUFFER, this->stencilAttachment.index);
					glRenderbufferStorage(GL_RENDERBUFFER, this->stencilAttachment.format, this->width, this->height);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->stencilAttachment.index);
				}
				return true;
			}
			bool clearColorAttachment(int index) {
				if (index < 0 || index >= this->maxAttachments)
					return false;
				this->bind();
				if (this->colorAttachments[index].attachType == AttachType::Texture2D) {
					glBindTexture(GL_TEXTURE_2D, 0);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, 0, 0);
					glDeleteTextures(1, &this->colorAttachments[index].index);
				}
				else if (this->colorAttachments[index].attachType == AttachType::RenderBuffer) {
					glBindRenderbuffer(GL_RENDERBUFFER, 0);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, 0);
					glDeleteRenderbuffers(1, &this->colorAttachments[index].index);
				}
				else
					return false;
				this->colorAttachments[index].reset();
				return true;
			}
			bool clearDepthAttachment(void) {
				this->bind();
				if (this->depthAttachment.attachType == AttachType::Texture2D) {
					glBindTexture(GL_TEXTURE_2D, 0);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
					glDeleteTextures(1, &this->depthAttachment.index);
				}
				else if (this->depthAttachment.attachType == AttachType::RenderBuffer) {
					glBindRenderbuffer(GL_RENDERBUFFER, 0);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
					glDeleteRenderbuffers(1, &this->depthAttachment.index);
				}
				else
					return false;
				this->depthAttachment.reset();
				return true;
			}
			bool clearStencilAttachment(void) {
				this->bind();
				if (this->stencilAttachment.attachType == AttachType::Texture2D) {
					glBindTexture(GL_TEXTURE_2D, 0);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
					glDeleteTextures(1, &this->stencilAttachment.index);
				}
				else if (this->stencilAttachment.attachType == AttachType::RenderBuffer) {
					glBindRenderbuffer(GL_RENDERBUFFER, 0);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
					glDeleteRenderbuffers(1, &this->stencilAttachment.index);
				}
				else
					return false;
				this->stencilAttachment.reset();
				return true;
			}
			bool setColorAttachment(int index, GLenum internalFormat, GLenum format, GLenum type, AttachType attachType) {
				if (index < 0 || index >= this->maxAttachments)
					return false;
				this->clearColorAttachment(index);
				if (attachType == AttachType::NotAttached)
					return true;
				Attachment& colorAttachment = this->colorAttachments[index];
				colorAttachment.set(
					attachType,
					0,
					internalFormat,
					format,
					type
				);
				if (colorAttachment.attachType == AttachType::Texture2D) {
					glGenTextures(1, &colorAttachment.index);
					glBindTexture(GL_TEXTURE_2D, colorAttachment.index);
					//TODO: use glTexStorage2D sometimes
					glTexImage2D(GL_TEXTURE_2D, 0, colorAttachment.internalFormat, this->width, this->height, 0, colorAttachment.format, colorAttachment.type, nullptr);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, colorAttachment.index, 0);
				}
				else if (colorAttachment.attachType == AttachType::RenderBuffer) {
					glGenRenderbuffers(1, &colorAttachment.index);
					glBindRenderbuffer(GL_RENDERBUFFER, colorAttachment.index);
					glRenderbufferStorage(GL_RENDERBUFFER, colorAttachment.format, this->width, this->height);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, colorAttachment.index);
				}
				return true;
			}
			bool setDepthAttachment(GLenum internalFormat, GLenum format, GLenum type, AttachType attachType) {
				this->clearDepthAttachment();
				if (attachType == AttachType::NotAttached)
					return true;
				this->depthAttachment.set(
					attachType,
					0,
					internalFormat,
					format,
					type
				);
				if (this->depthAttachment.attachType == AttachType::Texture2D) {
					glBindTexture(GL_TEXTURE_2D, this->depthAttachment.index);
					glGenTextures(1, &this->depthAttachment.index);
					glTexImage2D(GL_TEXTURE_2D, 0, this->depthAttachment.internalFormat, this->width, this->height, 0, this->depthAttachment.format, this->depthAttachment.type, nullptr);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depthAttachment.index, 0);
				}
				else if (this->depthAttachment.attachType == AttachType::RenderBuffer) {
					glGenRenderbuffers(1, &this->depthAttachment.index);
					glBindRenderbuffer(GL_RENDERBUFFER, this->depthAttachment.index);
					glRenderbufferStorage(GL_RENDERBUFFER, this->depthAttachment.format, this->width, this->height);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->depthAttachment.index);
				}
				return true;
			}
			bool setStencilAttachment(GLenum internalFormat, GLenum format, GLenum type, AttachType attachType) {
				this->clearStencilAttachment();
				if (attachType == AttachType::NotAttached)
					return true;
				this->stencilAttachment.set(
					attachType,
					0,
					internalFormat,
					format,
					type
				);
				if (this->stencilAttachment.attachType == AttachType::Texture2D) {
					glGenTextures(1, &this->stencilAttachment.index);
					glBindTexture(GL_TEXTURE_2D, this->stencilAttachment.index);
					glTexImage2D(GL_TEXTURE_2D, 0, this->stencilAttachment.internalFormat, this->width, this->height, 0, this->stencilAttachment.format, this->stencilAttachment.type, nullptr);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, this->stencilAttachment.index, 0);
				}
				else if (this->stencilAttachment.attachType == AttachType::RenderBuffer) {
					glGenRenderbuffers(1, &this->stencilAttachment.index);
					glBindRenderbuffer(GL_RENDERBUFFER, this->stencilAttachment.index);
					glRenderbufferStorage(GL_RENDERBUFFER, this->stencilAttachment.format, this->width, this->height);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->stencilAttachment.index);
				}
				return true;
			}
			bool setDrawBuffers(std::initializer_list<GLenum> list) {
				std::vector<GLenum> indices;
				for (const auto& index : list) {
					if (index < 0 || index >= this->maxAttachments || this->colorAttachments[index].attachType == AttachType::NotAttached)
						return false;
					indices.push_back(GL_COLOR_ATTACHMENT0 + index);
				}
				this->bind();
				glDrawBuffers(indices.size(), indices.data());
				return true;
			}
			template<class Iter> bool setDrawBuffers(const Iter begin, const Iter end) {
				std::vector<GLenum> indices;
				for (Iter iter = begin; iter != end; iter++) {
					if (*iter < 0 || *iter >= this->maxAttachments || this->colorAttachments[*iter].attachType == AttachType::NotAttached)
						return false;
					indices.push_back(GL_COLOR_ATTACHMENT0 + *iter);
				}
				this->bind();
				glDrawBuffers(indices.size(), indices.data());
				return true;
			}
			bool checkStatus(void) {
				this->bind();
				return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
			}
			//https://docs.gl/gl4/glGetTexImage
			bool readColorAttachment(int index, GLenum format, GLenum type, void* dst) {
				if (index < 0 || index >= this->maxAttachments)
					return false;
				this->bind();
				if (this->colorAttachments[index].attachType == AttachType::Texture2D) {
					glBindTexture(GL_TEXTURE_2D, this->colorAttachments[index].index);
					glGetTexImage(GL_TEXTURE_2D, 0, format, type, dst);
				}
				else if (this->colorAttachments[index].attachType == AttachType::RenderBuffer) {
					glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
					glReadPixels(0, 0, this->width, this->height, format, type, dst);
				}
				else
					return false;
				return true;
			}
			bool readDepthAttachment(GLenum type, void* dst) {
				this->bind();
				if (this->depthAttachment.attachType == AttachType::Texture2D) {
					glBindTexture(GL_TEXTURE_2D, this->depthAttachment.index);
					glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, type, dst);
				}
				else if (this->depthAttachment.attachType == AttachType::RenderBuffer) {
					glReadPixels(0, 0, this->width, this->height, GL_DEPTH_COMPONENT, type, dst);
				}
				else
					return false;
				return true;
			}
			bool readStencilAttachment(GLenum type, void* dst) {
				this->bind();
				if (this->stencilAttachment.attachType == AttachType::Texture2D) {
					glBindTexture(GL_TEXTURE_2D, this->stencilAttachment.index);
					glGetTexImage(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX, type, dst);
				}
				else if (this->stencilAttachment.attachType == AttachType::RenderBuffer) {
					glReadPixels(0, 0, this->width, this->height, GL_STENCIL_INDEX, type, dst);
				}
				else
					return false;
				return true;
			}
			~FrameBuffer(void) {
				for (int i = 0; i < this->maxAttachments; i++)
					this->clearColorAttachment(i);
				this->clearDepthAttachment();
				this->clearStencilAttachment();
				glDeleteFramebuffers(1, &this->fbo);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

		public:
			GLsizei width, height;
			GLuint fbo;
			GLint maxAttachments;

			std::vector<Attachment> colorAttachments;
			Attachment depthAttachment;
			Attachment stencilAttachment;
		};

	}
}

#endif /* jjyou_gl_FrameBuffer_hpp */