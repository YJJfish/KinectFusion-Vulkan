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

	void visualizeTSDFVolume() const;

private:

	const Engine* _pEngine;
	TSDFVolume _tsdfVolume;
	vk::Extent2D _colorFrameExtent;
	vk::Extent2D _depthFrameExtent;
};