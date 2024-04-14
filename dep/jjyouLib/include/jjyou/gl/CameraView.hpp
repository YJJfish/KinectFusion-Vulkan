/***********************************************************************
 * @file	CameraView.hpp
 * @author	jjyou
 * @date	2023-12-31
 * @brief	This file implements CameraView class.
***********************************************************************/
#ifndef jjyou_gl_CameraView_hpp
#define jjyou_gl_CameraView_hpp

#include <numbers>
#include <algorithm>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace jjyou {
	namespace gl {

		/***********************************************************************
		 * @class CameraView
		 * @brief Base class for computing the view matrix in OpenGL.
		 * 
		 * This is the base class for any camera view classes.
		 ***********************************************************************/
		class CameraView {

		public:

			/** @brief Default constructor.
			  *
			  * Construct and set the camera pose to default.
			  */
			CameraView(void);

			/** @brief Destructor.
			  *
			  * Destory the instance.
			  */
			virtual ~CameraView(void);

			/** @brief Reset the camera pose to default.
			  */
			virtual void reset(void);

			/** @brief	Get the view matrix for passing to the OpenGL shader.
			  * @return	The view matrix for the current camera pose.
			  */
			virtual glm::mat4 getViewMatrix(void) const = 0;

		};

		/***********************************************************************
		 * @class FirstPersonView
		 * @brief First-person view class for computing the view matrix in OpenGL.
		 *
		 * This class provides C++ API for adjusting the pose of a camera and
		 * computing the corresponding view matrix for passing to the OpenGL shader.
		 * The position of the camera is determined by a 3-dimensional vector, and
		 * the orientation of the camera is determined by yaw, pitch and roll. \n
		 * Yaw, pitch and roll are defined as follows:
		 * \code
		 * Yaw:
		 *  +z
		 *  |   /
		 *  |  /
		 *  | /
		 *  |/ yaw
		 *  +---------- +x
		 * 
		 * Pitch:
		 *  +y  front
		 *  |   /
		 *  |  /
		 *  | /
		 *  |/ pitch
		 *  +---------- x/z plane
		 * 
		 * Roll:
		 *    up
		 *    |       /
		 *    |roll /
		 *    |   /
		 *    | /
		 *  front------- right
		 * \endcode
		 * By default, the camera is located at `(0,0,0)`, looking in the
		 * direction of `(0,0,-1)`, with its "up" vector equal to `(0,1,0)`.
		 ***********************************************************************/
		class FirstPersonView : public CameraView {

		public:

			/** @brief Default constructor.
			  *
			  * Construct and set the camera pose to default.
			  */
			FirstPersonView(void);

			/** @brief Destructor.
			  *
			  * Destory the instance.
			  */
			virtual ~FirstPersonView(void) override;

			/** @brief Reset the camera pose to default.
			  */
			virtual void reset(void) override;

		public:

			/** @brief Get the position of the camera.
			  */
			const glm::vec3& getPos(void) const;

			/** @brief Set the position of the camera.
			  */
			void setPos(const glm::vec3& pos);

			/** @brief Set the position of the camera.
			  */
			void setPos(GLfloat posX, GLfloat posY, GLfloat posZ);

			/** @brief Get the "front" direction of the camera.
			  */
			const glm::vec3& getFront(void) const;

			/** @brief Get the "back" direction of the camera.
			  */
			glm::vec3 getBack(void) const;

			/** @brief Get the "up" direction of the camera.
			  */
			const glm::vec3& getUp(void) const;

			/** @brief Get the "down" direction of the camera.
			  */
			glm::vec3 getDown(void) const;

			/** @brief Get the "left" direction of the camera.
			  */
			const glm::vec3& getLeft(void) const;

			/** @brief Get the "right" direction of the camera.
			  */
			glm::vec3 getRight(void) const;

			/** @brief Get the yaw angle (in radians) of the camera.
			  */
			GLfloat getYaw(void) const;

			/** @brief Set the yaw angle (in radians) of the camera.
			  */
			void setYaw(GLfloat yaw);

			/** @brief Get the pitch angle (in radians) of the camera.
			  */
			GLfloat getPitch(void) const;

			/** @brief Set the pitch angle (in radians) of the camera.
			  */
			void setPitch(GLfloat pitch);

			/** @brief Get the roll angle (in radians) of the camera.
			  */
			GLfloat getRoll(void) const;

			/** @brief Set the roll angle (in radians) of the camera.
			  */
			void setRoll(GLfloat roll);

		public:
			
			/** @brief Set the orientation of the camera.
			  * @param dYaw		The increment of yaw.
			  * @param dPitch	The increment of pitch.
			  * @param dRoll	The increment of roll.
			  */
			void turn(GLfloat dYaw, GLfloat dPitch, GLfloat dRoll);

			/** @brief Set the orientation of the camera.
			  * @param yaw		The new value of yaw.
			  * @param pitch	The new value of pitch.
			  * @param roll		The new value of roll.
			  */
			void turnTo(GLfloat yaw, GLfloat pitch, GLfloat roll);

			/** @brief Set the position of the camera.
			  * @param dPos		The increment of position.
			  */
			void move(const glm::vec3& dPos);

			/** @brief Set the position of the camera.
			  * @param dPosX	The x component of the increment of position.
			  * @param dPosY	The y component of the increment of position.
			  * @param dPosZ	The z component of the increment of position.
			  */
			void move(GLfloat dPosX, GLfloat dPosY, GLfloat dPosZ);

			/** @brief Set the position of the camera.
			  * @param pos		The new value of position.
			  */
			void moveTo(const glm::vec3& pos);

			/** @brief Set the position of the camera.
			  * @param posX		The x component of the new value of position.
			  * @param posY		The y component of the new value of position.
			  * @param posZ		The z component of the new value of position.
			  */
			void moveTo(GLfloat posX, GLfloat posY, GLfloat posZ);

			/** @brief Move the camera along the "front" direction.
			  * @param dist		The length of movement.
			  */
			void moveFront(GLfloat dist);

			/** @brief Move the camera along the "back" direction.
			  * @param dist		The length of movement.
			  */
			void moveBack(GLfloat dist);

			/** @brief Move the camera along the "left" direction.
			  * @param dist		The length of movement.
			  */
			void moveLeft(GLfloat dist);

			/** @brief Move the camera along the "right" direction.
			  * @param dist		The length of movement.
			  */
			void moveRight(GLfloat dist);

			/** @brief Move the camera along the "up" direction.
			  * @param dist		The length of movement.
			  */
			void moveUp(GLfloat dist);

			/** @brief Move the camera along the "down" direction.
			  * @param dist		The length of movement.
			  */
			void moveDown(GLfloat dist);

			/** @brief	Get the view matrix for passing to the OpenGL shader.
			  * @return	The view matrix for the current camera pose.
			  */
			virtual glm::mat4 getViewMatrix(void) const override;

		private:
			glm::vec3 pos;
			glm::vec3 front;
			glm::vec3 up;
			glm::vec3 left;
			GLfloat yaw, pitch, roll;
			void updateOrientation(void);
		};



		/***********************************************************************
		 * @class SceneView
		 * @brief Scene view class for computing the view matrix in OpenGL.
		 *
		 * This class provides C++ API for adjusting the pose of a camera and
		 * computing the corresponding view matrix for passing to the OpenGL shader.
		 * Different from first-person view, the translation and rotation of scene
		 * view are all performed w.r.t. the center point.
		 * By default, the camera is looking at `(0,0,0)` in the direction
		 * of `(0,0,-1)`, with zoom rate equals to 1.0. (The distance from the camera
		 * to the center point is 1.0 when zoom rate equals to 1.0.)
		 ***********************************************************************/
		class SceneView : public CameraView {

		public:

			/** @brief Default constructor.
			  *
			  * Construct and set the camera pose to default.
			  */
			SceneView(void);

			/** @brief Destructor.
			  *
			  * Destory the instance.
			  */
			virtual ~SceneView(void) override;

			/** @brief Reset the camera pose to default.
			  */
			virtual void reset(void) override;

		public:

			/** @brief Get the center point of the camera.
			  */
			const glm::vec3& getCenter(void) const;

			/** @brief Set the center point of the camera.
			  */
			void setCenter(const glm::vec3& center);

			/** @brief Get the position of the camera.
			  */
			const glm::vec3& getPos(void) const;

			/** @brief Get the "front" direction of the camera.
			  */
			const glm::vec3& getFront(void) const;

			/** @brief Get the "back" direction of the camera.
			  */
			glm::vec3 getBack(void) const;

			/** @brief Get the "up" direction of the camera.
			  */
			const glm::vec3& getUp(void) const;

			/** @brief Get the "down" direction of the camera.
			  */
			glm::vec3 getDown(void) const;

			/** @brief Get the "left" direction of the camera.
			  */
			const glm::vec3& getLeft(void) const;

			/** @brief Get the "right" direction of the camera.
			  */
			glm::vec3 getRight(void) const;

			/** @brief Get the yaw angle (in radians) of the camera.
			  */
			GLfloat getYaw(void) const;

			/** @brief Set the yaw angle (in radians) of the camera.
			  */
			void setYaw(GLfloat yaw);

			/** @brief Get the pitch angle (in radians) of the camera.
			  */
			GLfloat getPitch(void) const;

			/** @brief Set the pitch angle (in radians) of the camera.
			  */
			void setPitch(GLfloat pitch);

			/** @brief Get the roll angle (in radians) of the camera.
			  */
			GLfloat getRoll(void) const;

			/** @brief Set the roll angle (in radians) of the camera.
			  */
			void setRoll(GLfloat roll);

			/** @brief Get the zoom rate of the camera.
			  */
			GLfloat getZoomRate(void) const;

			/** @brief Set the zoom rate of the camera.
			  */
			void setZoomRate(GLfloat zoomRate);

		public:

			/** @brief Set the orientation of the camera.
			  * @param dYaw		The increment of yaw.
			  * @param dPitch	The increment of pitch.
			  * @param dRoll	The increment of roll.
			  */
			void turn(GLfloat dYaw, GLfloat dPitch, GLfloat dRoll);

			/** @brief Set the orientation of the camera.
			  * @param yaw		The new value of yaw.
			  * @param pitch	The new value of pitch.
			  * @param roll		The new value of roll.
			  */
			void turnTo(GLfloat yaw, GLfloat pitch, GLfloat roll);

			/** @brief Set the center point of the camera.
			  * @param dPos		The increment of center point.
			  */
			void move(const glm::vec3& dCenter);

			/** @brief Set the center point of the camera.
			  * @param dPosX	The x component of the increment of center point.
			  * @param dPosY	The y component of the increment of center point.
			  * @param dPosZ	The z component of the increment of center point.
			  */
			void move(GLfloat dCenterX, GLfloat dCenterY, GLfloat dCenterZ);

			/** @brief Set the center point of the camera.
			  * @param pos		The new value of center point.
			  */
			void moveTo(const glm::vec3& center);

			/** @brief Set the center point of the camera.
			  * @param posX		The x component of the new value of center point.
			  * @param posY		The y component of the new value of center point.
			  * @param posZ		The z component of the new value of center point.
			  */
			void moveTo(GLfloat centerX, GLfloat centerY, GLfloat centerZ);

			/** @brief Zoom the camera along the "front" direction.
			  * @param dist		The rate of zooming.
			  */
			void zoomIn(GLfloat dRate);

			/** @brief Zoom the camera along the "back" direction.
			  * @param dist		The rate of zooming.
			  */
			void zoomOut(GLfloat dRate);

			/** @brief Move the camera along the "left" direction (scaled by the zoom rate).
			  * @param dist		The length of movement.
			  */
			void moveLeft(GLfloat dist);

			/** @brief Move the camera along the "right" direction (scaled by the zoom rate).
			  * @param dist		The length of movement.
			  */
			void moveRight(GLfloat dist);

			/** @brief Move the camera along the "up" direction (scaled by the zoom rate).
			  * @param dist		The length of movement.
			  */
			void moveUp(GLfloat dist);

			/** @brief Move the camera along the "down" direction (scaled by the zoom rate).
			  * @param dist		The length of movement.
			  */
			void moveDown(GLfloat dist);

			/** @brief	Get the view matrix for passing to the OpenGL shader.
			  * @return	The view matrix for the current camera pose.
			  */
			virtual glm::mat4 getViewMatrix(void) const override;

		private:
			glm::vec3 center;
			glm::vec3 pos;
			glm::vec3 front;
			glm::vec3 up;
			glm::vec3 left;
			GLfloat yaw, pitch, roll;
			GLfloat zoomRate;
			inline static GLfloat minZoomRate = 1e-2f;
			inline static GLfloat maxZoomRate = 1e+2f;
			void updateOrientation(void);
			void updatePos(void);
		};
	}
}


/*======================================================================
 | Implementation
 ======================================================================*/
 /// @cond


namespace jjyou {
	namespace gl {

		inline CameraView::CameraView(void) {}

		inline CameraView::~CameraView(void) {}

		inline void CameraView::reset(void) {}

		inline FirstPersonView::FirstPersonView(void) :
			pos(glm::vec3(0.0f)),
			yaw(std::numbers::pi_v<GLfloat> * 1.5f),
			pitch(0.0f),
			roll(0.0f)
		{
			this->updateOrientation();
		}

		inline FirstPersonView::~FirstPersonView(void) {}

		inline void FirstPersonView::reset(void) {
			CameraView::reset();
			constexpr GLfloat pi = std::numbers::pi_v<GLfloat>;
			this->pos = glm::vec3(0.0f);
			this->yaw = pi * 1.5f;
			this->pitch = 0.0f;
			this->roll = 0.0f;
			this->updateOrientation();
		}

		inline const glm::vec3& FirstPersonView::getPos(void) const {
			return this->pos;
		}

		inline void FirstPersonView::setPos(const glm::vec3& pos) {
			this->pos = pos;
		}

		inline void FirstPersonView::setPos(GLfloat posX, GLfloat posY, GLfloat posZ) {
			this->pos.x = posX;
			this->pos.y = posY;
			this->pos.z = posZ;
		}

		inline const glm::vec3& FirstPersonView::getFront(void) const {
			return this->front;
		}

		inline glm::vec3 FirstPersonView::getBack(void) const {
			return -this->front;
		}

		inline const glm::vec3& FirstPersonView::getUp(void) const {
			return this->up;
		}

		inline glm::vec3 FirstPersonView::getDown(void) const {
			return -this->up;
		}

		inline const glm::vec3& FirstPersonView::getLeft(void) const {
			return this->left;
		}

		inline glm::vec3 FirstPersonView::getRight(void) const {
			return -this->left;
		}

		inline GLfloat FirstPersonView::getYaw(void) const {
			return this->yaw;
		}

		inline void FirstPersonView::setYaw(GLfloat yaw) {
			constexpr GLfloat pi = std::numbers::pi_v<GLfloat>;
			this->yaw = std::fmod(yaw, 2.0f * pi);
			this->updateOrientation();
		}

		inline GLfloat FirstPersonView::getPitch(void) const {
			return this->pitch;
		}

		inline void FirstPersonView::setPitch(GLfloat pitch) {
			constexpr GLfloat pi = std::numbers::pi_v<GLfloat>;
			this->pitch = std::clamp<GLfloat>(pitch, -pi / 2.0f, pi / 2.0f);
			this->updateOrientation();
		}

		inline GLfloat FirstPersonView::getRoll(void) const {
			return this->roll;
		}

		inline void FirstPersonView::setRoll(GLfloat roll) {
			constexpr GLfloat pi = std::numbers::pi_v<GLfloat>;
			this->roll = std::fmod(roll, 2.0f * pi);
			this->updateOrientation();
		}

		inline void FirstPersonView::turn(GLfloat dYaw, GLfloat dPitch, GLfloat dRoll) {
			constexpr GLfloat pi = std::numbers::pi_v<GLfloat>;
			this->yaw = std::fmod(this->yaw + dYaw, 2.0f * pi);
			this->pitch = std::clamp<GLfloat>(this->pitch + dPitch, -pi / 2.0f, pi / 2.0f);
			this->roll = std::fmod(this->roll + dRoll, 2.0f * pi);
			this->updateOrientation();
		}

		inline void FirstPersonView::turnTo(GLfloat yaw, GLfloat pitch, GLfloat roll) {
			GLfloat pi = std::numbers::pi_v<GLfloat>;
			this->yaw = std::fmod(yaw, 2.0f * pi);
			this->pitch = std::clamp<GLfloat>(pitch, -pi / 2.0f, pi / 2.0f);
			this->roll = std::fmod(roll, 2.0f * pi);
			this->updateOrientation();
		}

		inline void FirstPersonView::move(const glm::vec3& dPos) {
			this->pos += dPos;
		}

		inline void FirstPersonView::move(GLfloat dPosX, GLfloat dPosY, GLfloat dPosZ) {
			this->pos.x += dPosX;
			this->pos.y += dPosY;
			this->pos.z += dPosZ;
		}

		inline void FirstPersonView::moveTo(const glm::vec3& pos) {
			this->pos = pos;
		}

		inline void FirstPersonView::moveTo(GLfloat posX, GLfloat posY, GLfloat posZ) {
			this->pos.x = posX;
			this->pos.y = posY;
			this->pos.z = posZ;
		}

		inline void FirstPersonView::moveFront(GLfloat dist) {
			this->pos += dist * this->front;
		}

		inline void FirstPersonView::moveBack(GLfloat dist) {
			this->pos -= dist * this->front;
		}

		inline void FirstPersonView::moveLeft(GLfloat dist) {
			this->pos += dist * this->left;
		}

		inline void FirstPersonView::moveRight(GLfloat dist) {
			this->pos -= dist * this->left;
		}

		inline void FirstPersonView::moveUp(GLfloat dist) {
			this->pos += dist * this->up;
		}

		inline void FirstPersonView::moveDown(GLfloat dist) {
			this->pos -= dist * this->up;
		}

		inline glm::mat4 FirstPersonView::getViewMatrix(void) const {
			return glm::lookAt(
				this->pos,
				this->pos + this->front,
				this->up
			);
		}

		inline void FirstPersonView::updateOrientation(void) {
			this->front.x = std::cos(this->pitch) * std::cos(this->yaw);
			this->front.y = std::sin(this->pitch);
			this->front.z = std::cos(this->pitch) * std::sin(this->yaw);
			this->up.x = -std::sin(this->pitch) * std::cos(this->yaw);
			this->up.y = std::cos(this->pitch);
			this->up.z = -std::sin(this->pitch) * std::sin(this->yaw);
			this->up = glm::mat3(glm::rotate(glm::mat4(1.0f), this->roll, this->front)) * this->up;
			this->left = glm::cross(this->up, this->front);
		}

		inline SceneView::SceneView(void) :
			center(0.0f),
			yaw(std::numbers::pi_v<GLfloat> * 1.5f),
			pitch(0.0f),
			roll(0.0f),
			zoomRate(1.0f)
		{
			this->updateOrientation();
			this->updatePos();
		}

		inline SceneView::~SceneView(void) {}

		inline void SceneView::reset(void) {
			CameraView::reset();
			constexpr GLfloat pi = std::numbers::pi_v<GLfloat>;
			this->center = glm::vec3(0.0f);
			this->yaw = pi * 1.5f;
			this->pitch = 0.0f;
			this->roll = 0.0f;
			this->zoomRate = 1.0f;
			this->updateOrientation();
			this->updatePos();
		}

		inline const glm::vec3& SceneView::getCenter(void) const {
			return this->center;
		}

		inline void SceneView::setCenter(const glm::vec3& center) {
			this->center = center;
			this->updatePos();
		}

		inline const glm::vec3& SceneView::getPos(void) const {
			return this->pos;
		}

		inline const glm::vec3& SceneView::getFront(void) const {
			return this->front;
		}

		inline glm::vec3 SceneView::getBack(void) const {
			return -this->front;
		}

		inline const glm::vec3& SceneView::getUp(void) const {
			return this->up;
		}

		inline glm::vec3 SceneView::getDown(void) const {
			return -this->up;
		}

		inline const glm::vec3& SceneView::getLeft(void) const {
			return this->left;
		}

		inline glm::vec3 SceneView::getRight(void) const {
			return -this->left;
		}

		inline GLfloat SceneView::getYaw(void) const {
			return this->yaw;
		}

		inline void SceneView::setYaw(GLfloat yaw) {
			constexpr GLfloat pi = std::numbers::pi_v<GLfloat>;
			this->yaw = std::fmod(yaw, 2.0f * pi);
			this->updateOrientation();
			this->updatePos();
		}

		inline GLfloat SceneView::getPitch(void) const {
			return this->pitch;
		}

		inline void SceneView::setPitch(GLfloat pitch) {
			constexpr GLfloat pi = std::numbers::pi_v<GLfloat>;
			this->pitch = std::clamp<GLfloat>(pitch, -pi / 2.0f, pi / 2.0f);
			this->updateOrientation();
			this->updatePos();
		}

		inline GLfloat SceneView::getRoll(void) const {
			return this->roll;
		}

		inline void SceneView::setRoll(GLfloat roll) {
			constexpr GLfloat pi = std::numbers::pi_v<GLfloat>;
			this->roll = std::fmod(roll, 2.0f * pi);
			this->updateOrientation();
			this->updatePos();
		}

		inline GLfloat SceneView::getZoomRate(void) const {
			return this->zoomRate;
		}

		inline void SceneView::setZoomRate(GLfloat zoomRate) {
			this->zoomRate = std::clamp(zoomRate, this->minZoomRate, this->maxZoomRate);
			this->updatePos();
		}

		inline void SceneView::turn(GLfloat dYaw, GLfloat dPitch, GLfloat dRoll) {
			constexpr GLfloat pi = std::numbers::pi_v<GLfloat>;
			this->yaw = std::fmod(this->yaw + dYaw, 2.0f * pi);
			this->pitch = std::clamp<GLfloat>(this->pitch + dPitch, -pi / 2.0f, pi / 2.0f);
			this->roll = std::fmod(this->roll + dRoll, 2.0f * pi);
			this->updateOrientation();
			this->updatePos();
		}

		inline void SceneView::turnTo(GLfloat yaw, GLfloat pitch, GLfloat roll) {
			GLfloat pi = std::numbers::pi_v<GLfloat>;
			this->yaw = std::fmod(yaw, 2.0f * pi);
			this->pitch = std::clamp<GLfloat>(pitch, -pi / 2.0f, pi / 2.0f);
			this->roll = std::fmod(roll, 2.0f * pi);
			this->updateOrientation();
			this->updatePos();
		}

		inline void SceneView::move(const glm::vec3& dCenter) {
			this->center += dCenter;
			this->updatePos();
		}

		inline void SceneView::move(GLfloat dCenterX, GLfloat dCenterY, GLfloat dCenterZ) {
			this->center.x += dCenterX;
			this->center.y += dCenterY;
			this->center.z += dCenterZ;
			this->updatePos();
		}

		inline void SceneView::moveTo(const glm::vec3& center) {
			this->center = center;
			this->updatePos();
		}

		inline void SceneView::moveTo(GLfloat centerX, GLfloat centerY, GLfloat centerZ) {
			this->center.x = centerX;
			this->center.y = centerY;
			this->center.z = centerZ;
			this->updatePos();
		}

		inline void SceneView::zoomIn(GLfloat dRate) {
			this->zoomRate = std::clamp(this->zoomRate / dRate, this->minZoomRate, this->maxZoomRate);
			this->updatePos();
		}

		inline void SceneView::zoomOut(GLfloat dRate) {
			this->zoomRate = std::clamp(this->zoomRate * dRate, this->minZoomRate, this->maxZoomRate);
			this->updatePos();
		}

		inline void SceneView::moveLeft(GLfloat dist) {
			this->center += dist * this->left * this->zoomRate;
			this->updatePos();
		}

		inline void SceneView::moveRight(GLfloat dist) {
			this->center -= dist * this->left * this->zoomRate;
			this->updatePos();
		}

		inline void SceneView::moveUp(GLfloat dist) {
			this->center += dist * this->up * this->zoomRate;
			this->updatePos();
		}

		inline void SceneView::moveDown(GLfloat dist) {
			this->center -= dist * this->up * this->zoomRate;
			this->updatePos();
		}

		inline glm::mat4 SceneView::getViewMatrix(void) const {
			return glm::lookAt(
				this->pos,
				this->pos + this->front,
				this->up
			);
		}

		inline void SceneView::updateOrientation(void) {
			this->front.x = std::cos(this->pitch) * std::cos(this->yaw);
			this->front.y = std::sin(this->pitch);
			this->front.z = std::cos(this->pitch) * std::sin(this->yaw);
			this->up.x = -std::sin(this->pitch) * std::cos(this->yaw);
			this->up.y = std::cos(this->pitch);
			this->up.z = -std::sin(this->pitch) * std::sin(this->yaw);
			this->up = glm::mat3(glm::rotate(glm::mat4(1.0f), this->roll, this->front)) * this->up;
			this->left = glm::cross(this->up, this->front);
		}

		inline void SceneView::updatePos(void) {
			this->pos = this->center - this->front * this->zoomRate;
		}

	}
}
/// @endcond

#endif /* jjyou_gl_CameraView_hpp */
