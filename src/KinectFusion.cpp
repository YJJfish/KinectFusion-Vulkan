#include "KinectFusion.hpp"

KinectFusion::KinectFusion(
	const Engine& engine_,
	vk::Extent2D colorFrameExtent_,
	vk::Extent2D depthFrameExtent_,
	const jjyou::glsl::uvec3& resolution_,
	float size_,
	std::optional<jjyou::glsl::vec3> corner_,
	std::optional<float> truncationDistance_
) : 
	_pEngine(&engine_),
	_tsdfVolume(engine_, resolution_, size_, corner_, truncationDistance_),
	_colorFrameExtent(colorFrameExtent_),
	_depthFrameExtent(depthFrameExtent_)
{

}

void KinectFusion::rayCasting(
	const Surface<Lambertian>& surface_,
	const jjyou::glsl::mat3& projection_,
	const jjyou::glsl::mat4& view_,
	std::optional<float> marchingStep_,
	std::optional<float> invalidDepth_
) const {

}