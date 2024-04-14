/***********************************************************************
 * @file	CameraView.hpp
 * @author	jjyou
 * @date	2023-12-31
 * @brief	This file implements CameraView class.
***********************************************************************/
#ifndef jjyou_vis_CameraView_hpp
#define jjyou_vis_CameraView_hpp

#if !defined(JJYOU_USE_OPENGL) && !defined(JJYOU_USE_VULKAN)
static_assert(0, "Please specify the API you use. E.g. define JJYOU_USE_OPENGL or JJYOU_USE_VULKAN");
#endif

#include <numbers>
#include <algorithm>
#include "../glsl/glsl.hpp"

namespace jjyou {

	namespace vis {

		/***********************************************************************
		 * @class CameraView
		 * @brief Base class for computing the view matrix in shader.
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

			/** @brief	Get the view matrix for passing to the shader.
			  * @return	The view matrix for the current camera pose.
			  */
			virtual glsl::mat4 getViewMatrix(void) const = 0;

		};

		/***********************************************************************
		 * @class FirstPersonView
		 * @brief First-person view class for computing the view matrix in shader.
		 *
		 * This class provides C++ API for adjusting the pose of a camera and
		 * computing the corresponding view matrix for passing to the shader.
		 * The position of the camera is determined by a 3-dimensional vector, and
		 * the orientation of the camera is determined by yaw, pitch and roll. \n
		 * Yaw, pitch and roll are defined as follows:
		 * \code
		 * Yaw:
		 *  +z(Vulkan)
		 *  -z(OpenGL)
		 *  |   /
		 *  |  /
		 *  | /
		 *  |/ yaw
		 *  +---------- +x
		 * 
		 * Pitch:
		 *  -y(Vulkan)
		 *  +y(OpenGL)
		 *      front
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
		 * By default, the camera is located at `(0,0,0)`,
		 * looking in the direction of `(0,0,+1)` (for Vulkan) or `(0,0,-1)` (for OpenGL)
		 * with its "up" vector equal to `(0,-1,0)` (for Vulkan) or `(0,+1,0)` (for OpenGL)
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
			const glsl::vec3& getPos(void) const;

			/** @brief Set the position of the camera.
			  */
			void setPos(const glsl::vec3& pos);

			/** @brief Set the position of the camera.
			  */
			void setPos(float posX, float posY, float posZ);

			/** @brief Get the "front" direction of the camera.
			  */
			const glsl::vec3& getFront(void) const;

			/** @brief Get the "back" direction of the camera.
			  */
			glsl::vec3 getBack(void) const;

			/** @brief Get the "up" direction of the camera.
			  */
			const glsl::vec3& getUp(void) const;

			/** @brief Get the "down" direction of the camera.
			  */
			glsl::vec3 getDown(void) const;

			/** @brief Get the "left" direction of the camera.
			  */
			const glsl::vec3& getLeft(void) const;

			/** @brief Get the "right" direction of the camera.
			  */
			glsl::vec3 getRight(void) const;

			/** @brief Get the yaw angle (in radians) of the camera.
			  */
			float getYaw(void) const;

			/** @brief Set the yaw angle (in radians) of the camera.
			  */
			void setYaw(float yaw);

			/** @brief Get the pitch angle (in radians) of the camera.
			  */
			float getPitch(void) const;

			/** @brief Set the pitch angle (in radians) of the camera.
			  */
			void setPitch(float pitch);

			/** @brief Get the roll angle (in radians) of the camera.
			  */
			float getRoll(void) const;

			/** @brief Set the roll angle (in radians) of the camera.
			  */
			void setRoll(float roll);

		public:
			
			/** @brief Set the orientation of the camera.
			  * @param dYaw		The increment of yaw.
			  * @param dPitch	The increment of pitch.
			  * @param dRoll	The increment of roll.
			  */
			void turn(float dYaw, float dPitch, float dRoll);

			/** @brief Set the orientation of the camera.
			  * @param yaw		The new value of yaw.
			  * @param pitch	The new value of pitch.
			  * @param roll		The new value of roll.
			  */
			void turnTo(float yaw, float pitch, float roll);

			/** @brief Set the position of the camera.
			  * @param dPos		The increment of position.
			  */
			void move(const glsl::vec3& dPos);

			/** @brief Set the position of the camera.
			  * @param dPosX	The x component of the increment of position.
			  * @param dPosY	The y component of the increment of position.
			  * @param dPosZ	The z component of the increment of position.
			  */
			void move(float dPosX, float dPosY, float dPosZ);

			/** @brief Set the position of the camera.
			  * @param pos		The new value of position.
			  */
			void moveTo(const glsl::vec3& pos);

			/** @brief Set the position of the camera.
			  * @param posX		The x component of the new value of position.
			  * @param posY		The y component of the new value of position.
			  * @param posZ		The z component of the new value of position.
			  */
			void moveTo(float posX, float posY, float posZ);

			/** @brief Move the camera along the "front" direction.
			  * @param dist		The length of movement.
			  */
			void moveFront(float dist);

			/** @brief Move the camera along the "back" direction.
			  * @param dist		The length of movement.
			  */
			void moveBack(float dist);

			/** @brief Move the camera along the "left" direction.
			  * @param dist		The length of movement.
			  */
			void moveLeft(float dist);

			/** @brief Move the camera along the "right" direction.
			  * @param dist		The length of movement.
			  */
			void moveRight(float dist);

			/** @brief Move the camera along the "up" direction.
			  * @param dist		The length of movement.
			  */
			void moveUp(float dist);

			/** @brief Move the camera along the "down" direction.
			  * @param dist		The length of movement.
			  */
			void moveDown(float dist);

			/** @brief	Get the view matrix for passing to the shader.
			  * @return	The view matrix for the current camera pose.
			  */
			virtual glsl::mat4 getViewMatrix(void) const override;

		private:
			glsl::vec3 pos;
			glsl::vec3 front;
			glsl::vec3 up;
			glsl::vec3 left;
			float yaw, pitch, roll;
			void updateOrientation(void);
		};



		/***********************************************************************
		 * @class SceneView
		 * @brief Scene view class for computing the view matrix in shader.
		 *
		 * This class provides C++ API for adjusting the pose of a camera and
		 * computing the corresponding view matrix for passing to the shader.
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
			const glsl::vec3& getCenter(void) const;

			/** @brief Set the center point of the camera.
			  */
			void setCenter(const glsl::vec3& center);

			/** @brief Get the position of the camera.
			  */
			const glsl::vec3& getPos(void) const;

			/** @brief Get the "front" direction of the camera.
			  */
			const glsl::vec3& getFront(void) const;

			/** @brief Get the "back" direction of the camera.
			  */
			glsl::vec3 getBack(void) const;

			/** @brief Get the "up" direction of the camera.
			  */
			const glsl::vec3& getUp(void) const;

			/** @brief Get the "down" direction of the camera.
			  */
			glsl::vec3 getDown(void) const;

			/** @brief Get the "left" direction of the camera.
			  */
			const glsl::vec3& getLeft(void) const;

			/** @brief Get the "right" direction of the camera.
			  */
			glsl::vec3 getRight(void) const;

			/** @brief Get the yaw angle (in radians) of the camera.
			  */
			float getYaw(void) const;

			/** @brief Set the yaw angle (in radians) of the camera.
			  */
			void setYaw(float yaw);

			/** @brief Get the pitch angle (in radians) of the camera.
			  */
			float getPitch(void) const;

			/** @brief Set the pitch angle (in radians) of the camera.
			  */
			void setPitch(float pitch);

			/** @brief Get the roll angle (in radians) of the camera.
			  */
			float getRoll(void) const;

			/** @brief Set the roll angle (in radians) of the camera.
			  */
			void setRoll(float roll);

			/** @brief Get the zoom rate of the camera.
			  */
			float getZoomRate(void) const;

			/** @brief Set the zoom rate of the camera.
			  */
			void setZoomRate(float zoomRate);

		public:

			/** @brief Set the orientation of the camera.
			  * @param dYaw		The increment of yaw.
			  * @param dPitch	The increment of pitch.
			  * @param dRoll	The increment of roll.
			  */
			void turn(float dYaw, float dPitch, float dRoll);

			/** @brief Set the orientation of the camera.
			  * @param yaw		The new value of yaw.
			  * @param pitch	The new value of pitch.
			  * @param roll		The new value of roll.
			  */
			void turnTo(float yaw, float pitch, float roll);

			/** @brief Set the center point of the camera.
			  * @param dPos		The increment of center point.
			  */
			void move(const glsl::vec3& dCenter);

			/** @brief Set the center point of the camera.
			  * @param dPosX	The x component of the increment of center point.
			  * @param dPosY	The y component of the increment of center point.
			  * @param dPosZ	The z component of the increment of center point.
			  */
			void move(float dCenterX, float dCenterY, float dCenterZ);

			/** @brief Set the center point of the camera.
			  * @param pos		The new value of center point.
			  */
			void moveTo(const glsl::vec3& center);

			/** @brief Set the center point of the camera.
			  * @param posX		The x component of the new value of center point.
			  * @param posY		The y component of the new value of center point.
			  * @param posZ		The z component of the new value of center point.
			  */
			void moveTo(float centerX, float centerY, float centerZ);

			/** @brief Zoom the camera along the "front" direction.
			  * @param dist		The rate of zooming.
			  */
			void zoomIn(float dRate);

			/** @brief Zoom the camera along the "back" direction.
			  * @param dist		The rate of zooming.
			  */
			void zoomOut(float dRate);

			/** @brief Move the camera along the "left" direction (scaled by the zoom rate).
			  * @param dist		The length of movement.
			  */
			void moveLeft(float dist);

			/** @brief Move the camera along the "right" direction (scaled by the zoom rate).
			  * @param dist		The length of movement.
			  */
			void moveRight(float dist);

			/** @brief Move the camera along the "up" direction (scaled by the zoom rate).
			  * @param dist		The length of movement.
			  */
			void moveUp(float dist);

			/** @brief Move the camera along the "down" direction (scaled by the zoom rate).
			  * @param dist		The length of movement.
			  */
			void moveDown(float dist);

			/** @brief	Get the view matrix for passing to the shader.
			  * @return	The view matrix for the current camera pose.
			  */
			virtual glsl::mat4 getViewMatrix(void) const override;

		private:
			glsl::vec3 center;
			glsl::vec3 pos;
			glsl::vec3 front;
			glsl::vec3 up;
			glsl::vec3 left;
			float yaw, pitch, roll;
			float zoomRate;
			inline static float minZoomRate = 1e-2f;
			inline static float maxZoomRate = 1e+2f;
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
	namespace vis {

		inline CameraView::CameraView(void) {}

		inline CameraView::~CameraView(void) {}

		inline void CameraView::reset(void) {}

		inline FirstPersonView::FirstPersonView(void) :
			pos(glsl::vec3(0.0f)),
			yaw(std::numbers::pi_v<float> / 2.0f),
			pitch(0.0f),
			roll(0.0f)
		{
			this->updateOrientation();
		}

		inline FirstPersonView::~FirstPersonView(void) {}

		inline void FirstPersonView::reset(void) {
			CameraView::reset();
			constexpr float pi = std::numbers::pi_v<float>;
			this->pos = glsl::vec3(0.0f);
			this->yaw = pi / 2.0f;
			this->pitch = 0.0f;
			this->roll = 0.0f;
			this->updateOrientation();
		}

		inline const glsl::vec3& FirstPersonView::getPos(void) const {
			return this->pos;
		}

		inline void FirstPersonView::setPos(const glsl::vec3& pos) {
			this->pos = pos;
		}

		inline void FirstPersonView::setPos(float posX, float posY, float posZ) {
			this->pos.x = posX;
			this->pos.y = posY;
			this->pos.z = posZ;
		}

		inline const glsl::vec3& FirstPersonView::getFront(void) const {
			return this->front;
		}

		inline glsl::vec3 FirstPersonView::getBack(void) const {
			return -this->front;
		}

		inline const glsl::vec3& FirstPersonView::getUp(void) const {
			return this->up;
		}

		inline glsl::vec3 FirstPersonView::getDown(void) const {
			return -this->up;
		}

		inline const glsl::vec3& FirstPersonView::getLeft(void) const {
			return this->left;
		}

		inline glsl::vec3 FirstPersonView::getRight(void) const {
			return -this->left;
		}

		inline float FirstPersonView::getYaw(void) const {
			return this->yaw;
		}

		inline void FirstPersonView::setYaw(float yaw) {
			constexpr float pi = std::numbers::pi_v<float>;
			this->yaw = std::fmod(yaw, 2.0f * pi);
			this->updateOrientation();
		}

		inline float FirstPersonView::getPitch(void) const {
			return this->pitch;
		}

		inline void FirstPersonView::setPitch(float pitch) {
			constexpr float pi = std::numbers::pi_v<float>;
			this->pitch = std::clamp<float>(pitch, -pi / 2.0f, pi / 2.0f);
			this->updateOrientation();
		}

		inline float FirstPersonView::getRoll(void) const {
			return this->roll;
		}

		inline void FirstPersonView::setRoll(float roll) {
			constexpr float pi = std::numbers::pi_v<float>;
			this->roll = std::fmod(roll, 2.0f * pi);
			this->updateOrientation();
		}

		inline void FirstPersonView::turn(float dYaw, float dPitch, float dRoll) {
			constexpr float pi = std::numbers::pi_v<float>;
			this->yaw = std::fmod(this->yaw + dYaw, 2.0f * pi);
			this->pitch = std::clamp<float>(this->pitch + dPitch, -pi / 2.0f, pi / 2.0f);
			this->roll = std::fmod(this->roll + dRoll, 2.0f * pi);
			this->updateOrientation();
		}

		inline void FirstPersonView::turnTo(float yaw, float pitch, float roll) {
			float pi = std::numbers::pi_v<float>;
			this->yaw = std::fmod(yaw, 2.0f * pi);
			this->pitch = std::clamp<float>(pitch, -pi / 2.0f, pi / 2.0f);
			this->roll = std::fmod(roll, 2.0f * pi);
			this->updateOrientation();
		}

		inline void FirstPersonView::move(const glsl::vec3& dPos) {
			this->pos += dPos;
		}

		inline void FirstPersonView::move(float dPosX, float dPosY, float dPosZ) {
			this->pos.x += dPosX;
			this->pos.y += dPosY;
			this->pos.z += dPosZ;
		}

		inline void FirstPersonView::moveTo(const glsl::vec3& pos) {
			this->pos = pos;
		}

		inline void FirstPersonView::moveTo(float posX, float posY, float posZ) {
			this->pos.x = posX;
			this->pos.y = posY;
			this->pos.z = posZ;
		}

		inline void FirstPersonView::moveFront(float dist) {
			this->pos += dist * this->front;
		}

		inline void FirstPersonView::moveBack(float dist) {
			this->pos -= dist * this->front;
		}

		inline void FirstPersonView::moveLeft(float dist) {
			this->pos += dist * this->left;
		}

		inline void FirstPersonView::moveRight(float dist) {
			this->pos -= dist * this->left;
		}

		inline void FirstPersonView::moveUp(float dist) {
			this->pos += dist * this->up;
		}

		inline void FirstPersonView::moveDown(float dist) {
			this->pos -= dist * this->up;
		}

		inline glsl::mat4 FirstPersonView::getViewMatrix(void) const {
			return glsl::lookAt(
				this->pos,
				this->pos + this->front,
				this->up
			);
		}

		inline void FirstPersonView::updateOrientation(void) {
#if defined(JJYOU_USE_OPENGL)
			this->front.x = std::cos(this->pitch) * std::cos(this->yaw);
			this->front.y = std::sin(this->pitch);
			this->front.z = -std::cos(this->pitch) * std::sin(this->yaw);
			this->up.x = -std::sin(this->pitch) * std::cos(this->yaw);
			this->up.y = std::cos(this->pitch);
			this->up.z = std::sin(this->pitch) * std::sin(this->yaw);
			this->up = glsl::rodrigues(this->front * this->roll) * this->up;
			this->left = glsl::cross(this->up, this->front);
#elif defined(JJYOU_USE_VULKAN)
			this->front.x = std::cos(this->pitch) * std::cos(this->yaw);
			this->front.y = -std::sin(this->pitch);
			this->front.z = std::cos(this->pitch) * std::sin(this->yaw);
			this->up.x = -std::sin(this->pitch) * std::cos(this->yaw);
			this->up.y = -std::cos(this->pitch);
			this->up.z = -std::sin(this->pitch) * std::sin(this->yaw);
			this->up = glsl::rodrigues(this->front * this->roll) * this->up;
			this->left = glsl::cross(this->up, this->front);
#endif
		}

		inline SceneView::SceneView(void) :
			center(0.0f),
			yaw(std::numbers::pi_v<float> / 2.0f),
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
			constexpr float pi = std::numbers::pi_v<float>;
			this->center = glsl::vec3(0.0f);
			this->yaw = pi / 2.0f;
			this->pitch = 0.0f;
			this->roll = 0.0f;
			this->zoomRate = 1.0f;
			this->updateOrientation();
			this->updatePos();
		}

		inline const glsl::vec3& SceneView::getCenter(void) const {
			return this->center;
		}

		inline void SceneView::setCenter(const glsl::vec3& center) {
			this->center = center;
			this->updatePos();
		}

		inline const glsl::vec3& SceneView::getPos(void) const {
			return this->pos;
		}

		inline const glsl::vec3& SceneView::getFront(void) const {
			return this->front;
		}

		inline glsl::vec3 SceneView::getBack(void) const {
			return -this->front;
		}

		inline const glsl::vec3& SceneView::getUp(void) const {
			return this->up;
		}

		inline glsl::vec3 SceneView::getDown(void) const {
			return -this->up;
		}

		inline const glsl::vec3& SceneView::getLeft(void) const {
			return this->left;
		}

		inline glsl::vec3 SceneView::getRight(void) const {
			return -this->left;
		}

		inline float SceneView::getYaw(void) const {
			return this->yaw;
		}

		inline void SceneView::setYaw(float yaw) {
			constexpr float pi = std::numbers::pi_v<float>;
			this->yaw = std::fmod(yaw, 2.0f * pi);
			this->updateOrientation();
			this->updatePos();
		}

		inline float SceneView::getPitch(void) const {
			return this->pitch;
		}

		inline void SceneView::setPitch(float pitch) {
			constexpr float pi = std::numbers::pi_v<float>;
			this->pitch = std::clamp<float>(pitch, -pi / 2.0f, pi / 2.0f);
			this->updateOrientation();
			this->updatePos();
		}

		inline float SceneView::getRoll(void) const {
			return this->roll;
		}

		inline void SceneView::setRoll(float roll) {
			constexpr float pi = std::numbers::pi_v<float>;
			this->roll = std::fmod(roll, 2.0f * pi);
			this->updateOrientation();
			this->updatePos();
		}

		inline float SceneView::getZoomRate(void) const {
			return this->zoomRate;
		}

		inline void SceneView::setZoomRate(float zoomRate) {
			this->zoomRate = std::clamp(zoomRate, this->minZoomRate, this->maxZoomRate);
			this->updatePos();
		}

		inline void SceneView::turn(float dYaw, float dPitch, float dRoll) {
			constexpr float pi = std::numbers::pi_v<float>;
			this->yaw = std::fmod(this->yaw + dYaw, 2.0f * pi);
			this->pitch = std::clamp<float>(this->pitch + dPitch, -pi / 2.0f, pi / 2.0f);
			this->roll = std::fmod(this->roll + dRoll, 2.0f * pi);
			this->updateOrientation();
			this->updatePos();
		}

		inline void SceneView::turnTo(float yaw, float pitch, float roll) {
			float pi = std::numbers::pi_v<float>;
			this->yaw = std::fmod(yaw, 2.0f * pi);
			this->pitch = std::clamp<float>(pitch, -pi / 2.0f, pi / 2.0f);
			this->roll = std::fmod(roll, 2.0f * pi);
			this->updateOrientation();
			this->updatePos();
		}

		inline void SceneView::move(const glsl::vec3& dCenter) {
			this->center += dCenter;
			this->updatePos();
		}

		inline void SceneView::move(float dCenterX, float dCenterY, float dCenterZ) {
			this->center.x += dCenterX;
			this->center.y += dCenterY;
			this->center.z += dCenterZ;
			this->updatePos();
		}

		inline void SceneView::moveTo(const glsl::vec3& center) {
			this->center = center;
			this->updatePos();
		}

		inline void SceneView::moveTo(float centerX, float centerY, float centerZ) {
			this->center.x = centerX;
			this->center.y = centerY;
			this->center.z = centerZ;
			this->updatePos();
		}

		inline void SceneView::zoomIn(float dRate) {
			this->zoomRate = std::clamp(this->zoomRate / dRate, this->minZoomRate, this->maxZoomRate);
			this->updatePos();
		}

		inline void SceneView::zoomOut(float dRate) {
			this->zoomRate = std::clamp(this->zoomRate * dRate, this->minZoomRate, this->maxZoomRate);
			this->updatePos();
		}

		inline void SceneView::moveLeft(float dist) {
			this->center += dist * this->left * this->zoomRate;
			this->updatePos();
		}

		inline void SceneView::moveRight(float dist) {
			this->center -= dist * this->left * this->zoomRate;
			this->updatePos();
		}

		inline void SceneView::moveUp(float dist) {
			this->center += dist * this->up * this->zoomRate;
			this->updatePos();
		}

		inline void SceneView::moveDown(float dist) {
			this->center -= dist * this->up * this->zoomRate;
			this->updatePos();
		}

		inline glsl::mat4 SceneView::getViewMatrix(void) const {
			return glsl::lookAt(
				this->pos,
				this->pos + this->front,
				this->up
			);
		}

		inline void SceneView::updateOrientation(void) {
#if defined(JJYOU_USE_OPENGL)
			this->front.x = std::cos(this->pitch) * std::cos(this->yaw);
			this->front.y = std::sin(this->pitch);
			this->front.z = -std::cos(this->pitch) * std::sin(this->yaw);
			this->up.x = -std::sin(this->pitch) * std::cos(this->yaw);
			this->up.y = std::cos(this->pitch);
			this->up.z = std::sin(this->pitch) * std::sin(this->yaw);
			this->up = glsl::rodrigues(this->front * this->roll) * this->up;
			this->left = glsl::cross(this->up, this->front);
#elif defined(JJYOU_USE_VULKAN)
			this->front.x = std::cos(this->pitch) * std::cos(this->yaw);
			this->front.y = -std::sin(this->pitch);
			this->front.z = std::cos(this->pitch) * std::sin(this->yaw);
			this->up.x = -std::sin(this->pitch) * std::cos(this->yaw);
			this->up.y = -std::cos(this->pitch);
			this->up.z = -std::sin(this->pitch) * std::sin(this->yaw);
			this->up = glsl::rodrigues(this->front * this->roll) * this->up;
			this->left = glsl::cross(this->up, this->front);
#endif
		}

		inline void SceneView::updatePos(void) {
			this->pos = this->center - this->front * this->zoomRate;
		}

	}
}
/// @endcond

#endif /* jjyou_gl_CameraView_hpp */
