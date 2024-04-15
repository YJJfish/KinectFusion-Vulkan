#include "DataLoader.hpp"
#include <exception>
#include <stdexcept>
#include <stb_image.h>

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
	FrameData res{};
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

ImageFolder::ImageFolder(
	const std::filesystem::path& colorFolder_,
	const std::filesystem::path& depthFolder_,
	float depthScale_,
	const jjyou::glsl::mat3& projection_,
	std::optional<std::vector<jjyou::glsl::mat4>> views_,
	float minDepth_,
	float maxDepth_,
	float invalidDepth_
) :
	DataLoader(),
	_colorFrameNames(),
	_depthFrameNames(),
	_depthScale(depthScale_),
	_projection(projection_),
	_views(std::move(views_)),
	_colorFrameExtent(),
	_depthFrameExtent(),
	_minDepth(minDepth_),
	_maxDepth(maxDepth_),
	_invalidDepth(invalidDepth_),
	_frameIndex(0),
	_colorMap(),
	_depthMap()
{
	for (const auto& entry : std::filesystem::directory_iterator(colorFolder_))
		this->_colorFrameNames.push_back(entry.path());
	for (const auto& entry : std::filesystem::directory_iterator(depthFolder_))
		this->_depthFrameNames.push_back(entry.path());
	if (this->_colorFrameNames.empty())
		throw std::runtime_error("[ImageFolder] Color image folder is empty.");
	if (this->_colorFrameNames.size() != this->_depthFrameNames.size())
		throw std::runtime_error("[ImageFolder] The number of color frames does not equal to that of depth frames.");
	if (this->_views.has_value() && this->_views->size() != this->_colorFrameNames.size())
		throw std::runtime_error("[ImageFolder] The number of view matrices does not equal to that of color frames.");
	std::sort(this->_colorFrameNames.begin(), this->_colorFrameNames.end());
	std::sort(this->_depthFrameNames.begin(), this->_depthFrameNames.end());
	// Load the first frame and extract image sizes.
	{
		int colorExtentX{}, colorExtentY{}, colorChannel{};
		int result = stbi_info(this->_colorFrameNames.front().string().c_str(), &colorExtentX, &colorExtentY, &colorChannel);
		if (result == 0) throw std::runtime_error("[ImageFolder] Failed to load " + this->_colorFrameNames.front().string() + ".");
		this->_colorFrameExtent = vk::Extent2D(static_cast<std::uint32_t>(colorExtentX), static_cast<std::uint32_t>(colorExtentY));
		this->_colorMap.reset(new FrameData::ColorPixel[this->_colorFrameExtent.width * this->_colorFrameExtent.height]{});
	}
	{
		int depthExtentX{}, depthExtentY{}, depthChannel{};
		int result = stbi_info(this->_depthFrameNames.front().string().c_str(), &depthExtentX, &depthExtentY, &depthChannel);
		if (result == 0) throw std::runtime_error("[ImageFolder] Failed to load " + this->_depthFrameNames.front().string() + ".");
		this->_depthFrameExtent = vk::Extent2D(static_cast<std::uint32_t>(depthExtentX), static_cast<std::uint32_t>(depthExtentY));
		this->_depthMap.reset(new FrameData::DepthPixel[this->_depthFrameExtent.width * this->_depthFrameExtent.height]{});
	}
}

FrameData ImageFolder::getFrame(void) {
	if (this->_frameIndex == static_cast<std::uint32_t>(this->_colorFrameNames.size())) {
		FrameData res{};
		res.state = FrameState::Eof;
		res.frameIndex = this->_frameIndex;
		// Still return the data of the last frame.
		res.colorMap = this->_colorMap.get();
		res.depthMap = this->_depthMap.get();
		res.projection = this->_projection;
		if (this->_views.has_value())
			res.view = this->_views->back();
		return res;
	}
	FrameData res{};
	res.state = FrameState::Valid;
	res.frameIndex = this->_frameIndex;
	res.colorMap = this->_colorMap.get();
	res.depthMap = this->_depthMap.get();
	res.projection = this->_projection;
	if (this->_views.has_value())
		res.view = (*this->_views)[this->_frameIndex];
	{
		int colorExtentX{}, colorExtentY{}, colorChannel{};
		std::uint8_t* colorPixels = stbi_load(this->_colorFrameNames[this->_frameIndex].string().c_str(), &colorExtentX, &colorExtentY, &colorChannel, STBI_rgb_alpha);
		if (colorPixels == nullptr) throw std::runtime_error("[ImageFolder] Failed to load " + this->_colorFrameNames[this->_frameIndex].string() + ".");
		if (static_cast<std::uint32_t>(colorExtentX) != this->_colorFrameExtent.width || static_cast<std::uint32_t>(colorExtentY) != this->_colorFrameExtent.height)
			throw std::runtime_error("[ImageFolder] The size of image " + this->_colorFrameNames[this->_frameIndex].string() + " does not match.");
		memcpy(this->_colorMap.get(), colorPixels, sizeof(FrameData::ColorPixel) * static_cast<std::size_t>(this->_colorFrameExtent.width) * static_cast<std::size_t>(this->_colorFrameExtent.height));
		stbi_image_free(colorPixels);
	}
	if (stbi_is_16_bit(this->_depthFrameNames[this->_frameIndex].string().c_str())) {
		int depthExtentX{}, depthExtentY{}, depthChannel{};
		std::uint16_t* depthPixels = stbi_load_16(this->_depthFrameNames[this->_frameIndex].string().c_str(), &depthExtentX, &depthExtentY, &depthChannel, STBI_grey);
		if (depthPixels == nullptr) throw std::runtime_error("[ImageFolder] Failed to load " + this->_depthFrameNames[this->_frameIndex].string() + ".");
		if (static_cast<std::uint32_t>(depthExtentX) != this->_depthFrameExtent.width || static_cast<std::uint32_t>(depthExtentY) != this->_depthFrameExtent.height)
			throw std::runtime_error("[ImageFolder] The size of image " + this->_depthFrameNames[this->_frameIndex].string() + " does not match.");
		for (std::size_t i = 0; i < static_cast<std::size_t>(this->_depthFrameExtent.width) * static_cast<std::size_t>(this->_depthFrameExtent.height); ++i)
			this->_depthMap[i] = static_cast<float>(depthPixels[i]) / 65535.0f * this->_depthScale;
		stbi_image_free(depthPixels);
	}
	else {
		int depthExtentX{}, depthExtentY{}, depthChannel{};
		std::uint8_t* depthPixels = stbi_load(this->_depthFrameNames[this->_frameIndex].string().c_str(), &depthExtentX, &depthExtentY, &depthChannel, STBI_grey);
		if (depthPixels == nullptr) throw std::runtime_error("[ImageFolder] Failed to load " + this->_depthFrameNames[this->_frameIndex].string() + ".");
		if (static_cast<std::uint32_t>(depthExtentX) != this->_depthFrameExtent.width || static_cast<std::uint32_t>(depthExtentY) != this->_depthFrameExtent.height)
			throw std::runtime_error("[ImageFolder] The size of image " + this->_depthFrameNames[this->_frameIndex].string() + " does not match.");
		for (std::size_t i = 0; i < static_cast<std::size_t>(this->_depthFrameExtent.width) * static_cast<std::size_t>(this->_depthFrameExtent.height); ++i)
			this->_depthMap[i] = static_cast<float>(depthPixels[i]) / 255.0f * this->_depthScale;
		stbi_image_free(depthPixels);
	}
	++this->_frameIndex;
	return res;
}