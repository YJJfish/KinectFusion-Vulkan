#include "DataLoader.hpp"

VirtualDataLoader::VirtualDataLoader(
	vk::Extent2D extent_,
	jjyou::glsl::vec3 center_,
	float length_
) : DataLoader(), _extent(extent_), _center(center_), _length(length_)
{
	this->_projection = jjyou::glsl::pinhole(std::numbers::pi_v<float> / 3.0f, this->_extent.width, this->_extent.height);
	this->_sceneViewer.reset();
	this->_sceneViewer.setCenter(this->_center);
	this->_sceneViewer.setZoomRate(std::max(2.0f, 3.0f * this->_length));
	this->_initialPose = this->_sceneViewer.getViewMatrix();
	this->_colorMap.reset(new FrameData::ColorPixel[this->_extent.width * this->_extent.height]{});
	this->_depthMap.reset(new FrameData::DepthPixel[this->_extent.width * this->_extent.height]{});
}

FrameData VirtualDataLoader::getFrame(void) {
	FrameData res;
	res.state = FrameState::Valid;
	res.frameIndex = this->_frameIndex;
	res.colorMap = this->_colorMap.get();
	res.depthMap = this->_depthMap.get();
	res.projection = this->_projection;
	res.view = this->_sceneViewer.getViewMatrix();
	jjyou::glsl::mat3 invProjection = jjyou::glsl::inverse(res.projection);
	jjyou::glsl::mat4 invView = jjyou::glsl::inverse(*res.view);
	jjyou::glsl::vec3 minCorner = this->_center - this->_length * 0.5f;
	jjyou::glsl::vec3 maxCorner = this->_center + this->_length * 0.5f;
	for (std::uint32_t r = 0; r < this->_extent.height; ++r)
		for (std::uint32_t c = 0; c < this->_extent.width; ++c) {
			FrameData::ColorPixel& colorPixel = this->_colorMap[r * this->_extent.width + c];
			FrameData::DepthPixel& depthPixel = this->_depthMap[r * this->_extent.width + c];
			jjyou::glsl::vec3 rayOrigin = jjyou::glsl::vec3(invView[3]);
			jjyou::glsl::vec3 rayDir(static_cast<float>(c) + 0.5f, static_cast<float>(r) + 0.5f, 1.0f);
			rayDir = invProjection * rayDir;
			float scaleFactor = jjyou::glsl::norm(rayDir);
			rayDir = jjyou::glsl::normalized(jjyou::glsl::mat3(invView) * rayDir);
			rayDir.x = (rayDir.x == 0.0f) ? 1e-5f : rayDir.x;
			rayDir.y = (rayDir.y == 0.0f) ? 1e-5f : rayDir.y;
			rayDir.z = (rayDir.z == 0.0f) ? 1e-5f : rayDir.z;
			float xMin = ((rayDir.x > 0.0f ? minCorner.x : maxCorner.x) - rayOrigin.x) / rayDir.x;
			float yMin = ((rayDir.y > 0.0f ? minCorner.y : maxCorner.y) - rayOrigin.y) / rayDir.y;
			float zMin = ((rayDir.z > 0.0f ? minCorner.z : maxCorner.z) - rayOrigin.z) / rayDir.z;
			float minT = std::max(std::max(xMin, yMin), zMin);
			float xMax = ((rayDir.x > 0.0f ? maxCorner.x : minCorner.x) - rayOrigin.x) / rayDir.x;
			float yMax = ((rayDir.y > 0.0f ? maxCorner.y : minCorner.y) - rayOrigin.y) / rayDir.y;
			float zMax = ((rayDir.z > 0.0f ? maxCorner.z : minCorner.z) - rayOrigin.z) / rayDir.z;
			float maxT = std::min(std::min(xMax, yMax), zMax);
			float hitDepth = minT / scaleFactor;
			if (minT >= maxT || hitDepth < this->minDepth() || hitDepth > this->maxDepth()) {
				colorPixel = FrameData::ColorPixel(0, 0, 0, 0);
				depthPixel = this->invalidDepth();
			}
			else {
				colorPixel = FrameData::ColorPixel(255, 255, 255, 255);
				depthPixel = hitDepth;
			}
		}
	float dYaw = 0.015f;
	float dPitch = (((this->_frameIndex + 20) / 40) % 2) ? 0.05f : -0.05f;
	this->_sceneViewer.turn(dYaw, dPitch, 0.0f);
	++this->_frameIndex;
	return res;
}