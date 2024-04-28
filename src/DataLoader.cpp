#include "DataLoader.hpp"
#include <exception>
#include <stdexcept>
#include <numbers>
#include <fstream>
#include <stb_image.h>

VirtualDataLoader::VirtualDataLoader(
	vk::Extent2D extent_,
	jjyou::glsl::vec3 center_,
	float length_
) : DataLoader(), _extent(extent_), _center(center_), _length(length_)
{
	this->_camera = Camera::fromGraphics(std::nullopt, std::numbers::pi_v<float> / 3.0f, this->minDepth(), this->maxDepth(), this->_extent.width, this->_extent.height);
	this->_sceneViewer.reset();
	this->_sceneViewer.setCenter(this->_center);
	this->_sceneViewer.setZoomRate(1.5f * this->_length);
	// dYaw: [pi*(5.0/24.0), pi*(7.0/24.0)]
	this->_yawRange = jjyou::glsl::vec2(std::numbers::pi_v<float> * 5.0f / 24.0f, std::numbers::pi_v<float> * 7.0f / 24.0f);
	this->_yawHalfPeriod = 120U;
	// dPitch: [pi/6.0, pi/4.0]
	this->_pitchRange = jjyou::glsl::vec2(std::numbers::pi_v<float> / 6.0f, std::numbers::pi_v<float> / 4.0f);
	this->_pitchHalfPeriod = 50U;
	this->_sceneViewer.turn(this->_yawRange.x, this->_pitchRange.x, 0.0f);
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
	res.camera = this->_camera;
	res.view = this->_sceneViewer.getViewMatrix();
	jjyou::glsl::mat3 invProjection = jjyou::glsl::inverse(this->_camera.getVisionProjection());
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
	float dYaw = (this->_yawRange.y - this->_yawRange.x) / static_cast<float>(this->_yawHalfPeriod);
	dYaw *= ((this->_frameIndex / this->_yawHalfPeriod) % 2) ? -1.0f : 1.0f;
	float dPitch = (this->_pitchRange.y - this->_pitchRange.x) / static_cast<float>(this->_pitchHalfPeriod);
	dPitch *= ((this->_frameIndex / this->_pitchHalfPeriod) % 2) ? -1.0f : 1.0f;
	this->_sceneViewer.turn(dYaw, dPitch, 0.0f);
	++this->_frameIndex;
	return res;
}

TUMDataset::TUMDataset(
	const std::filesystem::path& path_
) :
	DataLoader()
{
	// https://cvg.cit.tum.de/data/datasets/rgbd-dataset/file_formats
	this->_camera = Camera::fromVision(
		525.0f,
		525.0f,
		319.5f,
		239.5f,
		this->minDepth(),
		this->maxDepth(),
		this->depthFrameExtent().width,
		this->depthFrameExtent().height
	);
	this->_colorMap.reset(new FrameData::ColorPixel[this->colorFrameExtent().width * this->colorFrameExtent().height]{});
	this->_depthMap.reset(new FrameData::DepthPixel[this->depthFrameExtent().width * this->depthFrameExtent().height]{});
	std::ifstream inputFile;
	std::string inputBuffer;
	std::stringstream lineStream;
	// Read RGB image names and timestamps.
	std::vector<double> rgbTimestamps;
	std::vector<std::filesystem::path> rgbImageNames;
	inputFile.open(path_ / "rgb.txt", std::ios::in);
	if (!inputFile.is_open())
		throw std::runtime_error("[TUMDataset] Cannot open " + (path_ / "rgb.txt").string() + ".");
	while (std::getline(inputFile, inputBuffer)) {
		if (inputBuffer.empty() || inputBuffer.front() == '#')
			continue;
		lineStream.clear();
		lineStream << inputBuffer;
		double rgbTimestamp{};
		std::string rgbImageName;
		lineStream >> rgbTimestamp >> rgbImageName;
		rgbTimestamps.push_back(rgbTimestamp);
		rgbImageNames.emplace_back(rgbImageName);
	}
	inputFile.close();
	if (rgbImageNames.empty())
		throw std::runtime_error("[TUMDataset] No rgb data in " + (path_ / "rgb.txt").string() + ".");
	// Read depth image names and timestamps.
	std::vector<double> depthTimestamps;
	std::vector<std::filesystem::path> depthImageNames;
	inputFile.open(path_ / "depth.txt", std::ios::in);
	if (!inputFile.is_open())
		throw std::runtime_error("[TUMDataset] Cannot open " + (path_ / "depth.txt").string() + ".");
	while (std::getline(inputFile, inputBuffer)) {
		if (inputBuffer.empty() || inputBuffer.front() == '#')
			continue;
		lineStream.clear();
		lineStream << inputBuffer;
		double depthTimestamp{};
		std::string depthImageName;
		lineStream >> depthTimestamp >> depthImageName;
		depthTimestamps.push_back(depthTimestamp);
		depthImageNames.emplace_back(depthImageName);
	}
	inputFile.close();
	if (depthImageNames.empty())
		throw std::runtime_error("[TUMDataset] No depth data in " + (path_ / "depth.txt").string() + ".");
	// Read groundtruth trajectory data and timestamps.
	std::vector<double> groundtruthTimestamps;
	std::vector<jjyou::glsl::mat4> groundtruthViews;
	inputFile.open(path_ / "groundtruth.txt", std::ios::in);
	if (!inputFile.is_open())
		throw std::runtime_error("[TUMDataset] Cannot open " + (path_ / "groundtruth.txt").string() + ".");
	jjyou::glsl::mat4 transform(
		0.0f, 0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	while (std::getline(inputFile, inputBuffer)) {
		if (inputBuffer.empty() || inputBuffer.front() == '#')
			continue;
		lineStream.clear();
		lineStream << inputBuffer;
		double groundtruthTimestamp{};
		jjyou::glsl::vec3 t;
		jjyou::glsl::quat q;
		lineStream >> groundtruthTimestamp >> t.x >> t.y >> t.z >> q.x >> q.y >> q.z >> q.w;
		groundtruthTimestamps.push_back(groundtruthTimestamp);
		jjyou::glsl::mat4 view = jjyou::glsl::mat3(q);
		view[3] = jjyou::glsl::vec4(t, 1.0f);
		view = jjyou::glsl::inverse(transform * view);
		groundtruthViews.emplace_back(view);
	}
	inputFile.close();
	if (groundtruthViews.empty())
		throw std::runtime_error("[TUMDataset] No groundtruth data in " + (path_ / "groundtruth.txt").string() + ".");
	// Match depth images with RGB images and groundtruth poses.
	this->_colorFrameNames.reserve(depthImageNames.size());
	std::size_t rgbCounter = 0;
	this->_depthFrameNames.reserve(depthImageNames.size());
	this->_views.reserve(depthImageNames.size());
	std::size_t groundtruthCounter = 0;
	for (std::size_t depthCounter = 0; depthCounter < depthImageNames.size(); ++depthCounter) {
		double depthTimestamp = depthTimestamps[depthCounter];
		this->_depthFrameNames.push_back(path_ / depthImageNames[depthCounter]);
		while (rgbCounter + 1ULL < rgbImageNames.size() && rgbTimestamps[rgbCounter + 1ULL] < depthTimestamp)
			++rgbCounter;
		if (rgbCounter + 1ULL == rgbImageNames.size() ||
			(std::abs(rgbTimestamps[rgbCounter] - depthTimestamp) < std::abs(rgbTimestamps[rgbCounter + 1ULL] - depthTimestamp))
			) {
			this->_colorFrameNames.push_back(path_ / rgbImageNames[rgbCounter]);
		}
		else {
			this->_colorFrameNames.push_back(path_ / rgbImageNames[rgbCounter + 1ULL]);
		}
		while (groundtruthCounter + 1ULL < groundtruthViews.size() && groundtruthTimestamps[groundtruthCounter + 1ULL] < depthTimestamp)
			++groundtruthCounter;
		if (groundtruthCounter + 1ULL == groundtruthViews.size() ||
			(std::abs(groundtruthTimestamps[groundtruthCounter] - depthTimestamp) < std::abs(groundtruthTimestamps[groundtruthCounter + 1ULL] - depthTimestamp))
			) {
			this->_views.push_back(groundtruthViews[groundtruthCounter]);
		}
		else {
			this->_views.push_back(groundtruthViews[groundtruthCounter + 1ULL]);
		}
	}
}
FrameData TUMDataset::getFrame(void) {
	if (this->_frameIndex == static_cast<std::uint32_t>(this->_colorFrameNames.size())) {
		FrameData res{};
		res.state = FrameState::Eof;
		res.frameIndex = this->_frameIndex;
		// Still return the data of the last frame.
		res.colorMap = this->_colorMap.get();
		res.depthMap = this->_depthMap.get();
		res.camera = this->_camera;
		res.view = this->_views.back();
		return res;
	}
	FrameData res{};
	res.state = FrameState::Valid;
	res.frameIndex = this->_frameIndex;
	res.colorMap = this->_colorMap.get();
	res.depthMap = this->_depthMap.get();
	res.camera = this->_camera;
	res.view = this->_views[this->_frameIndex];
	{
		int colorExtentX{}, colorExtentY{}, colorChannel{};
		std::uint8_t* colorPixels = stbi_load(this->_colorFrameNames[this->_frameIndex].string().c_str(), &colorExtentX, &colorExtentY, &colorChannel, STBI_rgb_alpha);
		if (colorPixels == nullptr) throw std::runtime_error("[TUMDataset] Failed to load " + this->_colorFrameNames[this->_frameIndex].string() + ".");
		if (static_cast<std::uint32_t>(colorExtentX) != this->colorFrameExtent().width || static_cast<std::uint32_t>(colorExtentY) != this->colorFrameExtent().height)
			throw std::runtime_error("[TUMDataset] The size of image " + this->_colorFrameNames[this->_frameIndex].string() + " does not match.");
		memcpy(this->_colorMap.get(), colorPixels, sizeof(FrameData::ColorPixel) * static_cast<std::size_t>(this->colorFrameExtent().width) * static_cast<std::size_t>(this->colorFrameExtent().height));
		stbi_image_free(colorPixels);
	}
	if (stbi_is_16_bit(this->_depthFrameNames[this->_frameIndex].string().c_str())) {
		int depthExtentX{}, depthExtentY{}, depthChannel{};
		std::uint16_t* depthPixels = stbi_load_16(this->_depthFrameNames[this->_frameIndex].string().c_str(), &depthExtentX, &depthExtentY, &depthChannel, STBI_grey);
		if (depthPixels == nullptr) throw std::runtime_error("[TUMDataset] Failed to load " + this->_depthFrameNames[this->_frameIndex].string() + ".");
		if (static_cast<std::uint32_t>(depthExtentX) != this->depthFrameExtent().width || static_cast<std::uint32_t>(depthExtentY) != this->depthFrameExtent().height)
			throw std::runtime_error("[TUMDataset] The size of image " + this->_depthFrameNames[this->_frameIndex].string() + " does not match.");
		for (std::size_t i = 0; i < static_cast<std::size_t>(this->depthFrameExtent().width) * static_cast<std::size_t>(this->depthFrameExtent().height); ++i)
			this->_depthMap[i] = static_cast<float>(depthPixels[i]) / 5000.0f;
		stbi_image_free(depthPixels);
	}
	else {
		throw std::runtime_error("[TUMDataset] The image format of " + this->_depthFrameNames[this->_frameIndex].string() + " is not 16-bit.");
	}
	++this->_frameIndex;
	return res;
}
