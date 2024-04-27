#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <jjyou/vk/Vulkan.hpp>
#include <jjyou/glsl/glsl.hpp>
#include <optional>
#include <stdexcept>
#include <exception>

/***********************************************************************
 * @class	Camera
 * @brief	Perspective camera for both graphics rendering and vision-related
 *			data processing.
 ***********************************************************************/
class Camera {

public:

	float xFov{};
	float yFov{};
	float xOffset{};
	float yOffset{};
	float zNear{};
	float zFar{};
	std::uint32_t width{};
	std::uint32_t height{};

	/** @brief	Default constructor.
	  */ 
	Camera(void) = default;

	/** @brief	Default copy/move constructor/assignment.
	  */
	Camera(const Camera&) = default;
	Camera(Camera&&) = default;
	Camera& operator=(const Camera&) = default;
	Camera& operator=(Camera&&) = default;

	/** @brief	Create a camera using computer graphics parameters.
	  * @param	xFov_	(Optional) Field of view in radian in x direction.
	  * @param	yFov_	(Optional) Field of view in radian in y direction.
	  * @param	zNear_	Near clipping plane.
	  * @param	zFar_	Far clipping plane.
	  * @param	width_	Frame width.
	  * @param	height_	Frame height.
	  * 
	  *	At least one of `xFov_` and `yFov_` should be provided.
	  *	When only one of them is provided, the other one is computed
	  * using the aspect ratio.
	  * The principal points of the camera created by this method is
	  * placed at the image center.
	  */
	static Camera fromGraphics(
		std::optional<float> xFov_,
		std::optional<float> yFov_,
		float zNear_,
		float zFar_,
		std::uint32_t width_,
		std::uint32_t height_
	) {
		Camera res{};
		res.xOffset = 0.0f;
		res.yOffset = 0.0f;
		res.zNear = zNear_;
		res.zFar = zFar_;
		res.width = width_;
		res.height = height_;
		if (xFov_ == std::nullopt && yFov_ == std::nullopt)
			throw std::logic_error("[Camera] At least one of xFov_ and yFov_ should be specified.");
		if (xFov_ != std::nullopt && yFov_ != std::nullopt) {
			res.xFov = *xFov_;
			res.yFov = *yFov_;
		}
		else if (xFov_ != std::nullopt) {
			res.xFov = *xFov_;
			float tanHalfXFov = std::tan(*xFov_ / 2.0f);
			float tanHalfYFov = static_cast<float>(height_) / static_cast<float>(width_) * tanHalfXFov;
			res.yFov = std::atan(tanHalfYFov) * 2.0f;
		}
		else if (yFov_ != std::nullopt) {
			res.yFov = *yFov_;
			float tanHalfYFov = std::tan(*yFov_ / 2.0f);
			float tanHalfXFov = static_cast<float>(width_) / static_cast<float>(height_) * tanHalfYFov;
			res.xFov = std::atan(tanHalfXFov) * 2.0f;
		}
		return res;
	}

	/** @brief	Create a camera using computer vision parameters.
	  * @param	fx_		Focal length in x direction.
	  * @param	fy_		Focal length in y direction.
	  * @param	cx_		Principal point in x direction.
	  * @param	cy_		Principal point in y direction.
	  * @param	zNear_	Near clipping plane.
	  * @param	zFar_	Far clipping plane.
	  * @param	width_	Frame width.
	  * @param	height_	Frame height.
	  */
	static Camera fromVision(
		float fx_,
		float fy_,
		float cx_,
		float cy_,
		float zNear_,
		float zFar_,
		std::uint32_t width_,
		std::uint32_t height_
	) {
		Camera res{};
		res.xFov = std::atan(static_cast<float>(width_) / 2.0f / fx_) * 2.0f;
		res.yFov = std::atan(static_cast<float>(height_) / 2.0f / fy_) * 2.0f;
		res.xOffset = (cx_ - static_cast<float>(width_) / 2.0f + 0.5f) / static_cast<float>(width_) * 2.0f;
		res.yOffset = (cy_ - static_cast<float>(height_) / 2.0f + 0.5f) / static_cast<float>(height_) * 2.0f;;
		res.zNear = zNear_;
		res.zFar = zFar_;
		res.width = width_;
		res.height = height_;
		return res;
	}

	/** @brief	Get the 4x4 projection matrix for graphics rendering.
	  */
	jjyou::glsl::mat4 getGraphicsProjection(void) const {
		float m00 = 1.0f / std::tan(this->xFov / 2.0f);
		float m02 = this->xOffset;
		float m11 = 1.0f / std::tan(this->yFov / 2.0f);
		float m12 = this->yOffset;
		float m22 = this->zFar / (this->zFar - this->zNear);
		float m23 = -(this->zFar * this->zNear) / (this->zFar - this->zNear);
		return jjyou::glsl::mat4(
			m00, 0.0f, 0.0f, 0.0f,
			0.0f, m11, 0.0f, 0.0f,
			m02, m12, m22, 1.0f,
			0.0f, 0.0f, m23, 0.0f
		);
	}

	/** @brief	Get the 3x3 pinhole camera projection matrix.
	  */
	jjyou::glsl::mat3 getVisionProjection(void) const {
		float fx = static_cast<float>(this->width) / 2.0f / std::tan(this->xFov / 2.0f);
		float fy = static_cast<float>(this->height) / 2.0f / std::tan(this->yFov / 2.0f);
		float cx = this->xOffset * static_cast<float>(this->width) / 2.0f + static_cast<float>(this->width) / 2.0f - 0.5f;
		float cy = this->yOffset * static_cast<float>(this->height) / 2.0f + static_cast<float>(this->height) / 2.0f - 0.5f;
		return jjyou::glsl::mat3(
			fx, 0.0f, 0.0f,
			0.0f, fy, 0.0f,
			cx, cy, 1.0f
		);
	}

	/** @brief	Scale to fit a new frame size
	  */
	void scaleToFit(std::uint32_t width_, std::uint32_t height_) {
		float aspectRatioRestrict = static_cast<float>(width_) / static_cast<float>(height_);
		float aspectRatioCamera = static_cast<float>(this->width) / static_cast<float>(this->height);
		if (aspectRatioRestrict >= aspectRatioCamera) {
			this->height = height_;
			this->width = static_cast<std::uint32_t>(static_cast<float>(height_) * aspectRatioCamera);
		}
		else {
			this->width = width_;
			this->height = static_cast<std::uint32_t>(static_cast<float>(width_) / aspectRatioCamera);
		}
	}

	/** @brief	Scale to fill a new frame size
	  */
	void scaleToFill(std::uint32_t width_, std::uint32_t height_) {
		float aspectRatioRestrict = static_cast<float>(width_) / static_cast<float>(height_);
		float aspectRatioCamera = static_cast<float>(this->width) / static_cast<float>(this->height);
		if (aspectRatioRestrict < aspectRatioCamera) {
			this->height = height_;
			this->width = static_cast<std::uint32_t>(static_cast<float>(height_) * aspectRatioCamera);
		}
		else {
			this->width = width_;
			this->height = static_cast<std::uint32_t>(static_cast<float>(width_) / aspectRatioCamera);
		}
	}

	/** @brief	Update the camera parameters for a new image size.
	  * @note	Make sure that the width and height are scaled by the same factor.
	  *			If they are not, you may wish to call `fromGraphics` to recompute
	  *			the x/y field of view.
	  */
	void resize(vk::Extent2D extent_) {
		this->width = extent_.width;
		this->height = extent_.height;
	}
	void resize(std::uint32_t width_, std::uint32_t height_) {
		this->width = width_;
		this->height = height_;
	}

};