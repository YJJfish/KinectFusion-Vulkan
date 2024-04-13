#pragma once
#include "TSDFVolume.hpp"
#include "Engine.hpp"

/***********************************************************************
 * @class	KinectFusion
 * @brief	KinectFusion class that manages the resources and performs
 *			computations related to the fusion.
 *
 * This class supports the following API:
 *  - Initialize the TSDF volume.
 *  - Perform ray casting to get a surface (color, depth, normal).
 *  - Estimate the relative transform of a new frame w.r.t. the last frame.
 *  - Fuse a new frame into the global model.
 * All computations are synchronized with the CPU. That is, after each
 * command buffer submission, the CPU waits for a fence.
 * I tried to make the computations asynchronous but found this will make
 * it difficult to decouple this class from the Vulkan Engine class.
 ***********************************************************************/
class KinectFusion {

public:

	/** @brief	Number of levels in vertex/normal map pyramid.
	  */
	static inline constexpr std::uint32_t NUM_PYRAMID_LEVELS = 3;

	/** @brief	Number of ICP iterations.
	  */
	static inline constexpr std::array<std::uint32_t, NUM_PYRAMID_LEVELS> NUM_ICP_ITERATIONS = { { 4, 5, 10 } };

	/** @brief	Constructor.
	  * @param	engine_				The Vulkan engine.
	  * @param	truncationWeight_	Truncation weight in Eq. 13.
	  * @param	colorFrameExtent_	The size of input color frames.
	  * @param	depthFrameExtent_	The size of input depth frames.
	  * @param	minDepth_			The lower bound of valid depth.
	  * @param	maxDepth_			The upper bound of valid depth.
	  * @param	invalidDepth_		The invalid depth value.
	  * @param	resolution_			Volume resolution.
	  * @param	size_				Voxel size.
	  * @param	corner_				The coordinate of the corner voxel's center point.
	  * @param	truncationDistance_	Truncation distance.
	  * 
	  * For more information about `minDepth_`, `maxDepth_`, `invalidDepth_`,
	  * refer to `DataLoader`.
	  * For more information about `resolution_`, `size_`, `corner_`, `truncationDistance_`,
	  * refer to `TSDFVolume`.
	  */
	KinectFusion(
		// Vulkan resources
		const Engine& engine_,

		// Data formats.
		vk::Extent2D colorFrameExtent_,
		vk::Extent2D depthFrameExtent_,

		// Fusion parameters.
		float truncationWeight_,
		float minDepth_,
		float maxDepth_,
		float invalidDepth_,

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

	/** @brief	Initialize/Reset the TSDF volume.
	  */
	void initTSDFVolume(void) const;

	/** @brief	Perform ray casting to get the color, depth, and normal map.
	  * @param	surface_		Surface made up of color, depth, and normal textures.
	  *							The extents of the 3 textures should be the same.
	  * @param	projection_		Camera projection matrix. Note that this matrix should use a
	  *							Computer Vision model (e.g. a pinhole camera). It is different
	  *							from the Compute Graphics projection matrix.
	  * @param	view_			Camera view matrix that transforms from world space to camera space.
	  * @param	minDepth_		Visible depth lower bound. Voxels outside of this range will not be considered.
	  * @param	maxDepth_		Visible depth upper bound. Voxels outside of this range will not be considered.
	  * @param	invalidDepth_	Value for invalid depth. If a ray does not intersect with a zero-surface,
	  *							the output pixel will be written this value.
	  * @param	marchingStep_	Minimal ray marching step. By default, it is set to be 0.5x voxel size.
	  * @note	The `minDepth_`, `maxDepth_`, `invalidDepth_` may be different from the parameters
	  *			in KinectFusion's constructor. These 3 parameters only control the ray casting process.
	  * @note	The extent of the surface may be different from the parameters in KinectFusion's constructor.
	  */
	void rayCasting(
		const Surface<Lambertian>& surface_,
		const jjyou::glsl::mat3& projection_,
		const jjyou::glsl::mat4& view_,
		float minDepth_,
		float maxDepth_,
		float invalidDepth_,
		std::optional<float> marchingStep_ = std::nullopt
		) const;

	/** @brief	Fuse a new frame (color + depth) into the TSDF volume.
	  * @param	surface_		Surface made up of color and depth maps.
	  *							In current version, color map is not used.
	  * @param	projection_		Camera projection matrix. Note that this matrix should use a
	  *							Computer Vision model (e.g. a pinhole camera). It is different
	  *							from the Compute Graphics projection matrix.
	  * @param	view_			Camera view matrix that transforms points from world space to camera space.
	  */
	void fuse(
		const Surface<Simple>& surface_,
		const jjyou::glsl::mat3& projection_,
		const jjyou::glsl::mat4& view_
	) const;

	/** @brief	Get the descriptor set layout for TSDF volume storage buffer.
	  */
	const vk::raii::DescriptorSetLayout& tsdfVolumeDescriptorSetLayout(void) const {
		return this->_tsdfVolumeDescriptorSetLayout;
	}

	/** @brief	Get the descriptor set layout for ray casting uniform buffer.
	  */
	const vk::raii::DescriptorSetLayout& rayCastingDescriptorSetLayout(void) const {
		return this->_rayCastingDescriptorSetLayout;
	}

	/** @brief	Get the descriptor set layout for fusion uniform buffer.
	  */
	const vk::raii::DescriptorSetLayout& fusionDescriptorSetLayout(void) const {
		return this->_fusionDescriptorSetLayout;
	}

private:

	const Engine* _pEngine = nullptr;
	const vk::Extent2D _colorFrameExtent{};
	const vk::Extent2D _depthFrameExtent{};
	const float _truncationWeight;
	const float _minDepth;
	const float _maxDepth;
	const float _invalidDepth;
	vk::raii::DescriptorSetLayout _tsdfVolumeDescriptorSetLayout{ nullptr };
	vk::raii::DescriptorSetLayout _rayCastingDescriptorSetLayout{ nullptr };
	vk::raii::DescriptorSetLayout _fusionDescriptorSetLayout{ nullptr };
	TSDFVolume _tsdfVolume{ nullptr };
	vk::raii::PipelineLayout _initVolumePipelineLayout{ nullptr };
	vk::raii::PipelineLayout _rayCastingPipelineLayout{ nullptr };
	vk::raii::PipelineLayout _fusionPipelineLayout{ nullptr };
	vk::raii::Pipeline _initVolumePipeline{ nullptr };
	vk::raii::Pipeline _rayCastingPipeline{ nullptr };
	vk::raii::Pipeline _fusionPipeline{ nullptr };

	struct _InitVolumeAlgorithmData {
		vk::raii::CommandBuffer commandBuffer{ nullptr };
		vk::raii::Fence fence{ nullptr };
		vk::SubmitInfo submitInfo{};
	} _initVolumeAlgorithmData{};

	struct _RayCastingAlgorithmData {
		RayCastingDescriptorSet descriptorSet{ nullptr };
		vk::raii::CommandBuffer commandBuffer{ nullptr };
		vk::raii::Fence fence{ nullptr };
		vk::SubmitInfo submitInfo{};
	} _rayCastingAlgorithmData{};

	struct _FusionAlgorithmData {
		FusionDescriptorSet descriptorSet{ nullptr };
		vk::raii::CommandBuffer commandBuffer{ nullptr };
		vk::raii::Fence fence{ nullptr };
		vk::SubmitInfo submitInfo{};
	} _fusionAlgorithmData{};

	void _createDescriptorSetLayouts(void);
	void _createPipelineLayouts(void);
	void _createPipelines(void);
	void _createAlgorithmData(void);
};