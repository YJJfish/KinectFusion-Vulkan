#pragma once
#include "TSDFVolume.hpp"
#include "Engine.hpp"

class KinectFusion {

public:

	/** @brief	Constructor.
	  */
	KinectFusion(
		// Vulkan resources
		const Engine& engine_,

		// Data parameters
		vk::Extent2D colorFrameExtent_,
		vk::Extent2D depthFrameExtent_,

		// Volume parameters
		const jjyou::glsl::uvec3& resolution_,
		float size_,
		std::optional<jjyou::glsl::vec3> corner_ = std::nullopt,
		std::optional<float> truncationDistance_ = std::nullopt
	);

	/** @brief	Disable copy/move constructor/assignment.
	  */
	KinectFusion(const KinectFusion&) = delete;
	KinectFusion(KinectFusion&&) = delete;
	KinectFusion& operator=(const KinectFusion&) = delete;
	KinectFusion& operator=(KinectFusion&&) = delete;

	/** @brief	Destructor.
	  */
	~KinectFusion(void) = default;

	/** @brief	Perform ray casting to get the color, depth, and normal map.
	  * @param	surface_		Surface made up of color, depth, and normal textures.
	  *							The extents of the 3 textures should be the same.
	  * @param	projection_		Camera projection matrix. Note that this matrix should use a
	  *							Computer Vision model (e.g. a pinhole camera). It is different
	  *							from the Compute Graphics projection matrix.
	  * @param	view_			Camera view matrix that transforms from world space to camera space.
	  * @param	marchingStep_	Ray marching step. By default, it is set to be 0.5x voxel size.
	  * @param	invalidDepth_	Value for invalid depth. By default, it is set to be 0.0f.
	  */
	void rayCasting(
		const Surface<Lambertian>& surface_,
		const jjyou::glsl::mat3& projection_,
		const jjyou::glsl::mat4& view_,
		std::optional<float> marchingStep_ = std::nullopt,
		std::optional<float> invalidDepth_ = std::nullopt
	) const;

private:

	const Engine* _pEngine;
	TSDFVolume _tsdfVolume;
	vk::Extent2D _colorFrameExtent;
	vk::Extent2D _depthFrameExtent;
};