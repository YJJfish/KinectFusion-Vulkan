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
 * All computations are synchronized with the CPU. That is, after each
 * command buffer submission, the CPU waits for a fence.
 * I tried to make the computations asynchronous but found this will make
 * it difficult to decouple this class from the Vulkan Engine class.
 ***********************************************************************/
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
	  * @param	minDepth_		Visible depth lower bound.
	  * @param	maxDepth_		Visible depth upper bound.
	  * @param	marchingStep_	Minimal ray marching step. By default, it is set to be 0.5x voxel size.
	  * @param	invalidDepth_	Value for invalid depth. By default, it is set to be 0.0f.
	  */
	void rayCasting(
		const Surface<Lambertian>& surface_,
		const jjyou::glsl::mat3& projection_,
		const jjyou::glsl::mat4& view_,
		float minDepth_,
		float maxDepth_,
		std::optional<float> marchingStep_ = std::nullopt,
		std::optional<float> invalidDepth_ = std::nullopt
	) const;

private:

	const Engine* _pEngine = nullptr;
	vk::raii::DescriptorSetLayout _tsdfVolumeDescriptorSetLayout{ nullptr };
	vk::raii::DescriptorSetLayout _rayCastingDescriptorSetLayout{ nullptr };
	TSDFVolume _tsdfVolume{ nullptr };
	vk::raii::PipelineLayout _initVolumePipelineLayout{ nullptr };
	vk::raii::PipelineLayout _rayCastingPipelineLayout{ nullptr };
	vk::raii::Pipeline _initVolumePipeline{ nullptr };
	vk::raii::Pipeline _rayCastingPipeline{ nullptr };
	vk::raii::CommandBuffer _initVolumeCommandBuffer{ nullptr };
	vk::raii::CommandBuffer _rayCastingCommandBuffer{ nullptr };
	vk::Extent2D _colorFrameExtent{};
	vk::Extent2D _depthFrameExtent{};

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

	void _createDescriptorSetLayouts(void);
	void _createPipelineLayouts(void);
	void _createPipelines(void);
	void _createAlgorithmData(void);
};