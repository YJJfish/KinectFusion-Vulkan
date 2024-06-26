#pragma once
#include "TSDFVolume.hpp"
#include "Engine.hpp"
#include "Camera.hpp"
#include "PyramidData.hpp"

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
		std::int16_t truncationWeight_,
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

	/** @brief	Perform ray casting to get the color, depth, and normal map for visualization.
	  * @param	surface_		Surface made up of color, depth, and normal textures.
	  *							The extents of the 3 textures should be the same.
	  * @param	camera_			Camera instance for computing the projection matrix.
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
		const Camera& camera_,
		const jjyou::glsl::mat4& view_,
		float minDepth_,
		float maxDepth_,
		float invalidDepth_,
		std::optional<float> marchingStep_ = std::nullopt
	) const;

	/** @brief	Estimate the view matrix of a new frame using frame-to-model tracking.
	  * @param	surface_			Surface made up of color and depth maps. The color map is not used in this step.
	  * @param	camera_				Camera instance for computing projection matrices. The matrices will be different for different levels.
	  * @param	initialView_		The initial view matrix (from world space to camera space).
	  *								This is usually set as the view matrix of the last frame.
	  * @param	sigmaColor_			Bilateral filtering parameter.
	  * @param	sigmaSpace_			Bilateral filtering parameter.
	  * @param	filterKernelSize_	Bilateral filtering kernel size. Must be an odd number.
	  * @param	distanceThreshold_	Distance threshold used in projective correspondence search. In meters.
	  * @param	angleThreshold_		Angle threshold used in projective correspondence search. In radians.
	  * @return	The esimated view matrix for the frame. If the ICP failed, std::nullopt will be returned.
	  */
	std::optional<jjyou::glsl::mat4> estimatePose(
		const Surface<Simple>& surface_,
		const Camera& camera_,
		const jjyou::glsl::mat4& initialView_,
		float sigmaColor_,
		float sigmaSpace_,
		int filterKernelSize_,
		float distanceThreshold_,
		float angleThreshold_
	) const;

	/** @brief	Fuse a new frame (color + depth) into the TSDF volume.
	  * @param	surface_		Surface made up of color and depth maps.
	  * @param	camera_			Camera instance for computing the projection matrix.
	  * @param	view_			Camera view matrix that transforms points from world space to camera space.
	  */
	void fuse(
		const Surface<Simple>& surface_,
		const Camera& camera_,
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

	/** @brief	Get the descriptor set layout for pyramid data.
	  */
	const vk::raii::DescriptorSetLayout& pyramidDataDescriptorSetLayout(void) const {
		return this->_pyramidDataDescriptorSetLayout;
	}

	/** @brief	Get the descriptor set layout for ICP.
	  */
	const vk::raii::DescriptorSetLayout& icpDescriptorSetLayout(void) const {
		return this->_icpDescriptorSetLayout;
	}

private:

	const Engine* _pEngine = nullptr;
	const vk::Extent2D _colorFrameExtent{};
	const vk::Extent2D _depthFrameExtent{};
	const std::int16_t _truncationWeight;
	const float _minDepth;
	const float _maxDepth;
	const float _invalidDepth;
	vk::raii::DescriptorSetLayout _tsdfVolumeDescriptorSetLayout{ nullptr };
	vk::raii::DescriptorSetLayout _rayCastingDescriptorSetLayout{ nullptr };
	vk::raii::DescriptorSetLayout _fusionDescriptorSetLayout{ nullptr };
	vk::raii::DescriptorSetLayout _pyramidDataDescriptorSetLayout{ nullptr };
	vk::raii::DescriptorSetLayout _icpDescriptorSetLayout{ nullptr };
	TSDFVolume _tsdfVolume{ nullptr };
	vk::raii::PipelineLayout _initVolumePipelineLayout{ nullptr };
	vk::raii::PipelineLayout _rayCastingPipelineLayout{ nullptr };
	vk::raii::PipelineLayout _fusionPipelineLayout{ nullptr };
	vk::raii::PipelineLayout _bilateralFilteringPipelineLayout{ nullptr };
	vk::raii::PipelineLayout _rayCastingICPPipelineLayout{ nullptr };
	vk::raii::PipelineLayout _computeVertexNormalMapPipelineLayout{ nullptr };
	vk::raii::PipelineLayout _halfSamplingPipelineLayout{ nullptr };
	vk::raii::PipelineLayout _buildLinearFunctionPipelineLayout{ nullptr };
	vk::raii::PipelineLayout _buildLinearFunctionReductionPipelineLayout{ nullptr };
	vk::raii::Pipeline _initVolumePipeline{ nullptr };
	vk::raii::Pipeline _rayCastingPipeline{ nullptr };
	vk::raii::Pipeline _fusionPipeline{ nullptr };
	vk::raii::Pipeline _bilateralFilteringPipeline{ nullptr };
	vk::raii::Pipeline _rayCastingICPPipeline{ nullptr };
	vk::raii::Pipeline _computeVertexMapPipeline{ nullptr };
	vk::raii::Pipeline _computeNormalMapPipeline{ nullptr };
	vk::raii::Pipeline _halfSamplingPipeline{ nullptr };
	vk::raii::Pipeline _buildLinearFunctionPipeline{ nullptr };
	vk::raii::Pipeline _buildLinearFunctionReductionPipeline{ nullptr };

	struct _InitVolumeAlgorithmData {
		vk::raii::CommandBuffer commandBuffer{ nullptr };
		vk::raii::Fence fence{ nullptr };
	} _initVolumeAlgorithmData{};

	struct _RayCastingAlgorithmData {
		RayCastingDescriptorSet descriptorSet{ nullptr };
		vk::raii::CommandBuffer commandBuffer{ nullptr };
		vk::raii::Fence fence{ nullptr };
	} _rayCastingAlgorithmData{};

	struct _FusionAlgorithmData {
		FusionDescriptorSet descriptorSet{ nullptr };
		vk::raii::CommandBuffer commandBuffer{ nullptr };
		vk::raii::Fence fence{ nullptr };
	} _fusionAlgorithmData{};

	struct _PoseEstimationAlgorithmData {
		std::array<PyramidData, KinectFusion::NUM_PYRAMID_LEVELS> framePyramid{ {PyramidData{nullptr}, PyramidData{nullptr}, PyramidData{nullptr}} };
		std::array<PyramidData, KinectFusion::NUM_PYRAMID_LEVELS> modelPyramid{ {PyramidData{nullptr}, PyramidData{nullptr}, PyramidData{nullptr}} };
		vk::raii::CommandBuffer buildPyramidCommandBuffer{ nullptr };
		vk::raii::Fence buildPyramidFence{ nullptr };
		std::array<RayCastingDescriptorSet, KinectFusion::NUM_PYRAMID_LEVELS> rayCastingDescriptorSets{ { RayCastingDescriptorSet{nullptr}, RayCastingDescriptorSet{nullptr}, RayCastingDescriptorSet{nullptr} } };
		vk::raii::CommandBuffer rayCastingCommandBuffer{ nullptr };
		vk::raii::Fence rayCastingFence{ nullptr };
		ICPDescriptorSet icpDescriptorSet{ nullptr };
		vk::raii::CommandBuffer icpCommandBuffer{ nullptr };
		vk::raii::Fence icpFence{ nullptr };
	} _poseEstimationAlgorithmData{};

	void _createDescriptorSetLayouts(void);
	void _createPipelineLayouts(void);
	void _createPipelines(void);
	void _createAlgorithmData(void);

	/** @brief	Push constants.
	  */
	struct _BilateralFilteringParameters {
		float sigmaColor;	//!< The sigma value controlling the color term.
		float sigmaSpace;	//!< The sigma value controlling the space term.
		int d;				//!< The diameter of the filter area. It should be an odd number.
		float minDepth;
		float maxDepth;
		float invalidDepth;
	};
	struct _HalfSamplingParameters {
		float sigmaColor;	//!< The sigma value controlling the color term in bilateral filtering.
	};
	struct _CameraIntrinsics {
		float fx, fy, cx, cy;
	};
	struct _GlobalSumBufferLength {
		std::uint32_t len;
	};

	/** @brief	Work group size (local size of compute shaders).
	  */
	static inline constexpr jjyou::glsl::uvec3 _initVolumeWorkGroupSize{ 32U, 32U, 1U };
	static inline constexpr jjyou::glsl::uvec3 _rayCastingWorkGroupSize{ 32U, 32U, 1U };
	static inline constexpr jjyou::glsl::uvec3 _fusionWorkGroupSize{ 32U, 32U, 1U };
	static inline constexpr jjyou::glsl::uvec3 _bilateralFilteringWorkGroupSize{ 32U, 32U, 1U };
	static inline constexpr jjyou::glsl::uvec3 _halfSamplingWorkGroupSize{ 32U, 32U, 1U };
	static inline constexpr jjyou::glsl::uvec3 _computeVertexMapWorkGroupSize{ 32U, 32U, 1U };
	static inline constexpr jjyou::glsl::uvec3 _computeNormalMapWorkGroupSize{ 32U, 32U, 1U };
	static inline constexpr jjyou::glsl::uvec3 _rayCastingICPWorkGroupSize{ 32U, 32U, 1U };
	static inline constexpr jjyou::glsl::uvec3 _buildLinearFunctionWorkGroupSize{ 32U, 32U, 1U };
	static inline constexpr jjyou::glsl::uvec3 _buildLinearFunctionReductionWorkGroupSize{ 1024U, 1U, 1U };
};