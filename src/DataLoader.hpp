#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <jjyou/vk/Vulkan.hpp>
#include <jjyou/glsl/glsl.hpp>
#include <jjyou/vis/CameraView.hpp>
#include <optional>
#include <memory>

/***********************************************************************
 * @enum	FrameState
 * @brief	Enum used to indicate the state of the current frame.
 ***********************************************************************/
enum class FrameState {
	Valid,		/**< A valid frame. */
	Invalid,	/**< An invalid frame. Should not be used for fusion. */
	Eof			/**< Sensor closed / Dataset reached the end. No more new frames. */
};

/***********************************************************************
 * @class	FrameData
 * @brief	A structure containing information about a single frame.
 ***********************************************************************/
struct FrameData {
	using ColorPixel = jjyou::glsl::vec<unsigned char, 4>;
	using DepthPixel = float;

	FrameState state = FrameState::Invalid;
	std::uint32_t frameIndex = 0U;
	const ColorPixel* colorMap = nullptr; // The memory should be valid until next `getFrame` call.
	const DepthPixel* depthMap = nullptr; // The memory should be valid until next `getFrame` call.
	jjyou::glsl::mat3 projection{ 1.0f };
	std::optional<jjyou::glsl::mat4> view = std::nullopt;
};

/***********************************************************************
 * @class	DataLoader
 * @brief	Pure virtual data loader class that provides APIs to load
 *			data from sensors / datasets / other sources.
 *
 * A data loader should provide the following APIs:
 * 
 * Input frame size:
 *  - `vk::Extent2D colorFrameExtent(void)`
 *  - `vk::Extent2D depthFrameExtent(void)`
 * The size of input frames should be fixed throughout the algorithm, as it is
 * used to pre-allocate vulkan memory.
 * 
 * Invalid measurement:
 *  - `float minDepth(void)`
 *  - `float maxDepth(void)`
 *  - `float invalidDepth(void)`
 * The user may use different sensors' data as input, and different sensors use different
 * conventions for invalid depth measurement. For example, some use 0 as invalid measurement,
 * while some use a very large depth value as invalid measurement.
 * To address this, the dataloader should tell `minDepth`, `maxDepth`, and `invalidDepth`.
 * The depth values in a depth map are considered to be valid only if they are within
 * [minDepth, maxDepth] \ {invalidDepth}. Invalid depth values will not be used during
 * pose estimation and fusion.
 * For example, if your sensor uses 0 as invalid measurement, you can set `invalidDepth=0`,
 * `minDepth=0`, and `maxDepth=1e100`.
 * If your sensor uses a large value like 1000 as invalid measurement, you can set `invalidDepth=1000`,
 * `minDepth=0`, and `maxDepth=1e100`.
 * If you think depth measurements only within a specific range like [0.4, 8] are accurate,
 * you can set `invalidDepth=-1`, `minDepth=0.4`, and `maxDepth=8`.
 * If your data are synthesized by a graphics rasterization pipeline, which means all depth
 * values within the clipped depth range [zNear, zFar) are accurate, you can set `invalidDepth=zFar`,
 * `minDepth=zNear`, and `maxDepth=zFar`.
 * 
 * Data fetching:
 *  - `FrameData getFrame(void)`
 * The dataloader can optionally provide a groundtruth camera extrinsics stored in `FrameData::view`.
 *  - `jjyou::glsl::mat4 initialPose(void)`
 * You may wish to set an initial pose (extrinsics for the first frame) so that the reconstructed scene
 * is centered in the TSDF volume. If you don't have any prior about the scene, just set it as identity.
 ***********************************************************************/
class DataLoader {

public:

	/** @brief	Default constructor.
	  */
	DataLoader(void) {}

	/** @brief	Disable copy/move constructor/assignment.
	  */
	DataLoader(const DataLoader&) = delete;
	DataLoader(DataLoader&&) = delete;
	DataLoader& operator=(const DataLoader&) = delete;
	DataLoader& operator=(DataLoader&&) = delete;

	/** @brief	Destructor.
	  */
	virtual ~DataLoader(void) {}

	/** @brief	Get the size of input color frames.
	  */
	virtual vk::Extent2D colorFrameExtent(void) = 0;

	/** @brief	Get the size of input depth frames.
	  */
	virtual vk::Extent2D depthFrameExtent(void) = 0;

	/** @brief	Get the lower bound of valid depth.
	  */
	virtual float minDepth(void) = 0;

	/** @brief	Get the upper bound of valid depth.
	  */
	virtual float maxDepth(void) = 0;

	/** @brief	Get the invalid depth value.
	  */
	virtual float invalidDepth(void) = 0;

	/** @brief	Get the initial pose for the first frame.
	  */
	virtual jjyou::glsl::mat4 initialPose(void) { return jjyou::glsl::mat4(1.0f); }

	/** @brief	Get a new frame.
	  */
	virtual FrameData getFrame(void) = 0;

};

/***********************************************************************
 * @class	VirtualDataLoader
 * @brief	Virtual data loader that synthesizes data of a square.
 ***********************************************************************/
class VirtualDataLoader : public DataLoader {

public:

	/** @brief	Constructor.
	  */
	VirtualDataLoader(
		vk::Extent2D extent_,
		jjyou::glsl::vec3 center_,
		float length_
	);

	/** @brief	Disable copy/move constructor/assignment.
	  */
	VirtualDataLoader(const VirtualDataLoader&) = delete;
	VirtualDataLoader(VirtualDataLoader&&) = delete;
	VirtualDataLoader& operator=(const VirtualDataLoader&) = delete;
	VirtualDataLoader& operator=(VirtualDataLoader&&) = delete;

	/** @brief	Destructor.
	  */
	virtual ~VirtualDataLoader(void) override {}

	/** @brief	Get the size of input color frames.
	  */
	virtual vk::Extent2D colorFrameExtent(void) override { return this->_extent; }

	/** @brief	Get the size of input depth frames.
	  */
	virtual vk::Extent2D depthFrameExtent(void) override { return this->_extent; }

	/** @brief	Get the lower bound of valid depth.
	  */
	virtual float minDepth(void) override { return 0.1f; }

	/** @brief	Get the upper bound of valid depth.
	  */
	virtual float maxDepth(void) override { return 10.0f; }

	/** @brief	Get the invalid depth value.
	  */
	virtual float invalidDepth(void) override { return this->maxDepth(); }

	/** @brief	Get the initial pose for the first frame.
	  */
	virtual jjyou::glsl::mat4 initialPose(void) override { return this->_initialPose; }

	/** @brief	Get a new frame.
	  */
	virtual FrameData getFrame(void) override;

private:

	vk::Extent2D _extent{};
	jjyou::glsl::vec3 _center{};
	float _length = 0.0f;
	std::uint32_t _frameIndex = 0;
	jjyou::glsl::mat3 _projection{};
	jjyou::glsl::mat4 _initialPose{};
	jjyou::vis::SceneView _sceneViewer{};
	std::unique_ptr<FrameData::ColorPixel[]> _colorMap{};
	std::unique_ptr<FrameData::DepthPixel[]> _depthMap{};

};