/***********************************************************************
 * @file	ModelView.hpp
 * @author	jjyou
 * @date	2023-6-3
 * @brief	This file implements ModelView class.
***********************************************************************/
#ifndef jjyou_gl_ModelView_hpp
#define jjyou_gl_ModelView_hpp

#include <numbers>
#include <algorithm>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace jjyou {
	namespace gl {

		/***********************************************************************
		 * @class ModelView
		 * @brief Model view class for computing the model matrix in OpenGL.
		 *
		 * This class provides C++ API for computing the model matrix for passing
		 * to the OpenGL shader. It is templated with a \p Type parameter for
		 * different purposes and usages.
		 * @tparam Type	The type of the class.
		 ***********************************************************************/
		template<int Type>
		class ModelView;

		/***********************************************************************
		 * @class ModelView<0>
		 * @brief Model view class for computing the model matrix in OpenGL.
		 *
		 * This class provides C++ API for computing the model matrix for passing
		 * to the OpenGL shader. The model matrix is computed in the order of
		 * scaling, rotation, and translation. In other words,
		 * \code
		 * model matrix = translation matrix * rotation matrix * scaling matrix 
		 * \endcode
		 * The scaling component is represented by a 3-dimensional vector ModelView<0>::scale.
		 * The rotation component is represented by three euler angles (in radians) ModelView<0>::euler.
		 * The translation component is represented by a 3-dimensional vector ModelView<0>::pos.
		 * By default, the model matrix is an identity matrix.
		 ***********************************************************************/
		template<>
		class ModelView<0> {

		public:

			/** @brief Scaling component.
			  */
			glm::vec3 scale;

			/** @brief Rotation component.
			  * 
			  * The rotation matrix is computed by euler angles (in radians)
			  * in the order of "ZYX".
			  */
			glm::vec3 euler;

			/** @brief Translation component.
			  */
			glm::vec3 pos;

			/** @brief Default constructor.
			  * 
			  * Construct and set the model view to default.
			  */
			ModelView(void);

			/** @brief Reset the model view to default.
			  */
			void reset(void);

			/** @brief Get the model matrix for passing to the OpenGL shader.
			  * @return		The model matrix for the current parameter settings.
			  */
			glm::mat4 getModelMatrix(void) const;
		};

		/***********************************************************************
		 * @class ModelView<1>
		 * @brief Model view class for computing the model matrix in OpenGL.
		 *
		 * This class provides C++ API for computing the model matrix for passing
		 * to the OpenGL shader. The model matrix is computed by left multiplying
		 * all matrices produced by ModelView<1>::rotate, ModelView<1>::translate,
		 * ModelView<1>::scale together, in the order of function calls.
		 * By default, the model matrix is an identity matrix.
		 ***********************************************************************/
		template<>
		class ModelView<1> {

		public:

			/** @brief Default constructor.
			  *
			  * Construct and set the model view to default.
			  */
			ModelView(void);

			/** @brief Reset the model view to default.
			  */
			void reset(void);

			/** @brief Rotate the model around an axis.
			  * @param angle	The rotation angle (in radians)
			  * @param axis		The rotation axis
			  */
			void rotate(GLfloat angle, const glm::vec3& axis);

			/** @brief Translate the model.
			  * @param dPos		The displacement for translation.
			  */
			void translate(const glm::vec3& dPos);

			/** @brief Translate the model.
			  * @param dPosX	The x component of the displacement for translation.
			  * @param dPosY	The y component of the displacement for translation.
			  * @param dPosZ	The z component of the displacement for translation.
			  */
			void translate(const GLfloat& dPosX, const GLfloat& dPosY, const GLfloat& dPosZ);

			/** @brief Scale the model.
			  * @param dScale	The scaling factors for x/y/z component.
			  */
			void scale(const glm::vec3& dScale);

			/** @brief Scale the model.
			  * @param dScaleX	The scaling factor for x component.
			  * @param dScaleY	The scaling factor for y component.
			  * @param dScaleZ	The scaling factor for z component.
			  */
			void scale(const GLfloat& dScaleX, const GLfloat& dScaleY, const GLfloat& dScaleZ);

			/** @brief Scale the model. The scaling factors for x/y/z component are the same.
			  * @param dScale	The scaling factor for x/y/z component.
			  */
			void scale(const GLfloat& dScale);

			/** @brief Get the model matrix for passing to the OpenGL shader.
			  * @return		The model matrix for the current parameter settings.
			  */
			const glm::mat4& getModelMatrix(void) const;

		private:
			glm::mat4 modelMatrix;
		};
	}
}


/*======================================================================
 | Implementation
 ======================================================================*/
 /// @cond


namespace jjyou {
	namespace gl {

		inline ModelView<0>::ModelView(void) :
			scale(1.0f),
			euler(0.0f),
			pos(0.0f)
		{}

		inline void ModelView<0>::reset(void) {
			this->scale = glm::vec3(1.0f);
			this->euler = glm::vec3(0.0f);
			this->pos = glm::vec3(0.0f);
		}

		inline glm::mat4 ModelView<0>::getModelMatrix(void) const {
			glm::mat4 model(1.0f);
			model = glm::scale(glm::mat4(1.0f), this->scale) * model;
			model = glm::rotate(glm::mat4(1.0f), this->euler.z, glm::vec3(0.0f, 0.0f, 1.0f)) * model;
			model = glm::rotate(glm::mat4(1.0f), this->euler.y, glm::vec3(0.0f, 1.0f, 0.0f)) * model;
			model = glm::rotate(glm::mat4(1.0f), this->euler.x, glm::vec3(1.0f, 0.0f, 0.0f)) * model;
			model = glm::translate(glm::mat4(1.0f), this->pos) * model;
			return model;
		}

		inline ModelView<1>::ModelView(void) :
			modelMatrix(1.0f)
		{}

		inline void ModelView<1>::reset(void) {
			this->modelMatrix = glm::mat4(1.0f);
		}

		inline void ModelView<1>::rotate(GLfloat angle, const glm::vec3& axis) {
			this->modelMatrix = glm::rotate(glm::mat4(1.0f), angle, axis) * this->modelMatrix;
		}

		inline void ModelView<1>::translate(const glm::vec3& dPos) {
			this->modelMatrix = glm::translate(glm::mat4(1.0f), dPos) * this->modelMatrix;
		}

		inline void ModelView<1>::translate(const GLfloat& dPosX, const GLfloat& dPosY, const GLfloat& dPosZ) {
			this->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(dPosX, dPosY, dPosZ)) * this->modelMatrix;
		}

		inline void ModelView<1>::scale(const glm::vec3& dScale) {
			this->modelMatrix = glm::scale(glm::mat4(1.0f), dScale) * this->modelMatrix;
		}

		inline void ModelView<1>::scale(const GLfloat& dScaleX, const GLfloat& dScaleY, const GLfloat& dScaleZ) {
			this->modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(dScaleX, dScaleY, dScaleZ)) * this->modelMatrix;
		}

		inline void ModelView<1>::scale(const GLfloat& dScale) {
			this->modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(dScale)) * this->modelMatrix;
		}

		inline const glm::mat4& ModelView<1>::getModelMatrix(void) const {
			return this->modelMatrix;
		}

	}
}
/// @endcond

#endif /* jjyou_gl_ModelView_hpp */