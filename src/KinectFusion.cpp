#include "KinectFusion.hpp"
#include <exception>
#include <stdexcept>
#include <Eigen/Eigen>

#define VK_THROW(err) \
	throw std::runtime_error("[KinectFusion] Vulkan error in file " + std::string(__FILE__) + " line " + std::to_string(__LINE__) + ": " + vk::to_string(err))

#define VK_CHECK(value) \
	if (vk::Result err = (value); err != vk::Result::eSuccess) { VK_THROW(err); }

KinectFusion::KinectFusion(
	const Engine & engine_,
	vk::Extent2D colorFrameExtent_,
	vk::Extent2D depthFrameExtent_,
	std::int16_t truncationWeight_,
	float minDepth_,
	float maxDepth_,
	float invalidDepth_,
	const jjyou::glsl::uvec3 & resolution_,
	float size_,
	std::optional<jjyou::glsl::vec3> corner_,
	std::optional<float> truncationDistance_
) : 
	_pEngine(&engine_),
	_colorFrameExtent(colorFrameExtent_),
	_depthFrameExtent(depthFrameExtent_),
	_truncationWeight(truncationWeight_),
	_minDepth(minDepth_),
	_maxDepth(maxDepth_),
	_invalidDepth(invalidDepth_)
{
	if (depthFrameExtent_.width % (1U << KinectFusion::NUM_PYRAMID_LEVELS) != 0) {
		throw std::logic_error("The width of depth frame is " + std::to_string(depthFrameExtent_.width) + " which is not a multiple of " + std::to_string(1U << KinectFusion::NUM_PYRAMID_LEVELS) + ".");
	}
	if (depthFrameExtent_.height % (1U << KinectFusion::NUM_PYRAMID_LEVELS) != 0) {
		throw std::logic_error("The height of depth frame is " + std::to_string(depthFrameExtent_.height) + " which is not a multiple of " + std::to_string(1U << KinectFusion::NUM_PYRAMID_LEVELS) + ".");
	}
	this->_createDescriptorSetLayouts();
	this->_tsdfVolume = TSDFVolume(*this->_pEngine, *this, resolution_, size_, corner_, truncationDistance_);
	this->_createPipelineLayouts();
	this->_createPipelines();
	this->_createAlgorithmData();
	this->initTSDFVolume();
}

void KinectFusion::initTSDFVolume(void) const {
	const vk::raii::CommandBuffer& commandBuffer = this->_initVolumeAlgorithmData.commandBuffer;
	const vk::raii::Fence& fence = this->_initVolumeAlgorithmData.fence;
	commandBuffer.begin(
		vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
		.setPInheritanceInfo(nullptr)
	);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *this->_initVolumePipeline);
	this->_tsdfVolume.bind(commandBuffer, vk::PipelineBindPoint::eCompute, this->_initVolumePipelineLayout, 0);
	commandBuffer.dispatch(
		(this->_tsdfVolume.resolution().x + KinectFusion::_initVolumeWorkGroupSize.x - 1U) / KinectFusion::_initVolumeWorkGroupSize.x,
		(this->_tsdfVolume.resolution().y + KinectFusion::_initVolumeWorkGroupSize.y - 1U) / KinectFusion::_initVolumeWorkGroupSize.y,
		1U
	);
	commandBuffer.end();
	this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Compute)->submit(
		vk::SubmitInfo()
		.setWaitSemaphores(nullptr)
		.setWaitDstStageMask(nullptr)
		.setCommandBuffers(*commandBuffer)
		.setSignalSemaphores(nullptr),
		*fence
	);
	vk::Result waitResult = this->_pEngine->context().device().waitForFences(*fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
	VK_CHECK(waitResult);
	this->_pEngine->context().device().resetFences(*fence);
	commandBuffer.reset(vk::CommandBufferResetFlags(0));
}

void KinectFusion::rayCasting(
	const Surface<Lambertian>& surface_,
	const Camera& camera_,
	const jjyou::glsl::mat4& view_,
	float minDepth_,
	float maxDepth_,
	float invalidDepth_,
	std::optional<float> marchingStep_
) const {
	const RayCastingDescriptorSet& rayCastingDescriptorSet = this->_rayCastingAlgorithmData.descriptorSet;
	const vk::raii::CommandBuffer& commandBuffer = this->_rayCastingAlgorithmData.commandBuffer;
	const vk::raii::Fence& fence = this->_rayCastingAlgorithmData.fence;
	commandBuffer.begin(
		vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
		.setPInheritanceInfo(nullptr)
	);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *this->_rayCastingPipeline);
	this->_tsdfVolume.bind(commandBuffer, vk::PipelineBindPoint::eCompute, this->_rayCastingPipelineLayout, 0);
	jjyou::glsl::mat3 projection = camera_.getVisionProjection();
	rayCastingDescriptorSet.rayCastingParameters().fx = projection[0][0];
	rayCastingDescriptorSet.rayCastingParameters().fy = projection[1][1];
	rayCastingDescriptorSet.rayCastingParameters().cx = projection[2][0];
	rayCastingDescriptorSet.rayCastingParameters().cy = projection[2][1];
	rayCastingDescriptorSet.rayCastingParameters().invView = jjyou::glsl::inverse(view_);
	rayCastingDescriptorSet.rayCastingParameters().minDepth = minDepth_;
	rayCastingDescriptorSet.rayCastingParameters().maxDepth = maxDepth_;
	rayCastingDescriptorSet.rayCastingParameters().invalidDepth = invalidDepth_;
	rayCastingDescriptorSet.rayCastingParameters().marchingStep = marchingStep_.has_value() ? *marchingStep_ : (0.5f * this->_tsdfVolume.size());
	rayCastingDescriptorSet.bind(commandBuffer, vk::PipelineBindPoint::eCompute, this->_rayCastingPipelineLayout, 1);
	surface_.bindStorage(commandBuffer, vk::PipelineBindPoint::eCompute, this->_rayCastingPipelineLayout, 2);
	commandBuffer.dispatch(
		(surface_.texture(0).extent().width + KinectFusion::_rayCastingWorkGroupSize.x - 1U) / KinectFusion::_rayCastingWorkGroupSize.x,
		(surface_.texture(0).extent().height + KinectFusion::_rayCastingWorkGroupSize.y - 1U) / KinectFusion::_rayCastingWorkGroupSize.y,
		1U
	);
	commandBuffer.end();
	this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Compute)->submit(
		vk::SubmitInfo()
		.setWaitSemaphores(nullptr)
		.setWaitDstStageMask(nullptr)
		.setCommandBuffers(*commandBuffer)
		.setSignalSemaphores(nullptr),
		*fence
	);
	vk::Result waitResult = this->_pEngine->context().device().waitForFences(*fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
	VK_CHECK(waitResult);
	this->_pEngine->context().device().resetFences(*fence);
	commandBuffer.reset(vk::CommandBufferResetFlags(0));
}

std::optional<jjyou::glsl::mat4> KinectFusion::estimatePose(
	const Surface<Simple>& surface_,
	const Camera& camera_,
	const jjyou::glsl::mat4& initialView_,
	float sigmaColor_,
	float sigmaSpace_,
	int filterKernelSize_,
	float distanceThreshold_,
	float angleThreshold_
) const {
	vk::Result waitResult{};
	// Prepare memory barriers for sychronizaton use.
	vk::BufferMemoryBarrier readAfterWriteBufferMemoryBarrier = vk::BufferMemoryBarrier()
		.setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
		.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		//.setBuffer()
		.setOffset(0ULL)
		.setSize(VK_WHOLE_SIZE);
	vk::ImageMemoryBarrier readAfterWriteImageMemoryBarrier = vk::ImageMemoryBarrier()
		.setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
		.setDstAccessMask(vk::AccessFlagBits::eShaderWrite)
		.setOldLayout(vk::ImageLayout::eGeneral)
		.setNewLayout(vk::ImageLayout::eGeneral)
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		//.setImage()
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0U, 1U, 0U, 1U));
	// 1. Build pyramid.
	const vk::raii::CommandBuffer& buildPyramidCommandBuffer = this->_poseEstimationAlgorithmData.buildPyramidCommandBuffer;
	const vk::raii::Fence& buildPyramidFence = this->_poseEstimationAlgorithmData.buildPyramidFence;
	const std::array<PyramidData, KinectFusion::NUM_PYRAMID_LEVELS>& framePyramid = this->_poseEstimationAlgorithmData.framePyramid;
	// Apply bilateral filtering to the input depth map.
	buildPyramidCommandBuffer.begin(
		vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
		.setPInheritanceInfo(nullptr)
	);
	buildPyramidCommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *this->_bilateralFilteringPipeline);
	surface_.bindStorage(buildPyramidCommandBuffer, vk::PipelineBindPoint::eCompute, this->_bilateralFilteringPipelineLayout, 0);
	framePyramid[0].bind(buildPyramidCommandBuffer, vk::PipelineBindPoint::eCompute, this->_bilateralFilteringPipelineLayout, 1);
	_BilateralFilteringParameters bilateralFilteringParameters{
		.sigmaColor = sigmaColor_,
		.sigmaSpace = sigmaSpace_,
		.d = filterKernelSize_,
		.minDepth = this->_minDepth,
		.maxDepth = this->_maxDepth,
		.invalidDepth = this->_invalidDepth
	};
	buildPyramidCommandBuffer.pushConstants<_BilateralFilteringParameters>(*this->_bilateralFilteringPipelineLayout, vk::ShaderStageFlagBits::eCompute, 0U, bilateralFilteringParameters);
	buildPyramidCommandBuffer.dispatch(
		(surface_.texture(0).extent().width + KinectFusion::_bilateralFilteringWorkGroupSize.x - 1U) / KinectFusion::_bilateralFilteringWorkGroupSize.x,
		(surface_.texture(0).extent().height + KinectFusion::_bilateralFilteringWorkGroupSize.y - 1U) / KinectFusion::_bilateralFilteringWorkGroupSize.y,
		1U
	);
	// Push constant to the pipeline layout of half-sampling.
	_HalfSamplingParameters halfSamplingParameters{
		.sigmaColor = sigmaColor_
	};
	buildPyramidCommandBuffer.pushConstants<_HalfSamplingParameters>(*this->_halfSamplingPipelineLayout, vk::ShaderStageFlagBits::eCompute, 0U, halfSamplingParameters);
	// Half-sample depth maps & generate vertex maps and normals.
	for (std::uint32_t level = 0; level < KinectFusion::NUM_PYRAMID_LEVELS; ++level) {
		// Barrier for bilateral filtering / half-sampling that writes to current level's depth map.
		readAfterWriteImageMemoryBarrier.setImage(*framePyramid[level].texture(0).image());
		buildPyramidCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(0), nullptr, nullptr, readAfterWriteImageMemoryBarrier);
		// Half-sampling to next level's depth map.
		if (level != KinectFusion::NUM_PYRAMID_LEVELS - 1) {
			framePyramid[level].bind(buildPyramidCommandBuffer, vk::PipelineBindPoint::eCompute, this->_halfSamplingPipelineLayout, 0);
			framePyramid[level + 1].bind(buildPyramidCommandBuffer, vk::PipelineBindPoint::eCompute, this->_halfSamplingPipelineLayout, 1);
			buildPyramidCommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *this->_halfSamplingPipeline);
			buildPyramidCommandBuffer.dispatch(
				(framePyramid[level + 1].texture(0).extent().width + KinectFusion::_halfSamplingWorkGroupSize.x - 1U) / KinectFusion::_halfSamplingWorkGroupSize.x,
				(framePyramid[level + 1].texture(0).extent().height + KinectFusion::_halfSamplingWorkGroupSize.y - 1U) / KinectFusion::_halfSamplingWorkGroupSize.y,
				1U
			);
		}
		// Bind descriptor set to the pipeline layout of computing vertex / normal map.
		framePyramid[level].bind(buildPyramidCommandBuffer, vk::PipelineBindPoint::eCompute, this->_computeVertexNormalMapPipelineLayout, 0);
		// Push constant to the pipeline layout of computing vertex / normal map.
		Camera levelCamera = camera_;
		levelCamera.resize(framePyramid[level].texture(0).extent());
		jjyou::glsl::mat3 projection = levelCamera.getVisionProjection();
		_CameraIntrinsics cameraIntrinsics{
			.fx = projection[0][0],
			.fy = projection[1][1],
			.cx = projection[2][0],
			.cy = projection[2][1]
		};
		buildPyramidCommandBuffer.pushConstants<_CameraIntrinsics>(*this->_computeVertexNormalMapPipelineLayout, vk::ShaderStageFlagBits::eCompute, 0U, cameraIntrinsics);
		// Compute vertex map.
		buildPyramidCommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *this->_computeVertexMapPipeline);
		buildPyramidCommandBuffer.dispatch(
			(framePyramid[level].texture(0).extent().width + KinectFusion::_computeVertexMapWorkGroupSize.x - 1U) / KinectFusion::_computeVertexMapWorkGroupSize.x,
			(framePyramid[level].texture(0).extent().height + KinectFusion::_computeVertexMapWorkGroupSize.y - 1U) / KinectFusion::_computeVertexMapWorkGroupSize.y,
			1U
		);
		// Barrier for computing vertex map.
		readAfterWriteImageMemoryBarrier.setImage(*framePyramid[level].texture(1).image());
		buildPyramidCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(0), nullptr, nullptr, readAfterWriteImageMemoryBarrier);
		// Compute normal map.
		buildPyramidCommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *this->_computeNormalMapPipeline);
		buildPyramidCommandBuffer.dispatch(
			(framePyramid[level].texture(0).extent().width + KinectFusion::_computeNormalMapWorkGroupSize.x - 1U) / KinectFusion::_computeNormalMapWorkGroupSize.x,
			(framePyramid[level].texture(0).extent().height + KinectFusion::_computeNormalMapWorkGroupSize.y - 1U) / KinectFusion::_computeNormalMapWorkGroupSize.y,
			1U
		);
	}
	buildPyramidCommandBuffer.end();
	this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Compute)->submit(
		vk::SubmitInfo()
		.setWaitSemaphores(nullptr)
		.setWaitDstStageMask(nullptr)
		.setCommandBuffers(*buildPyramidCommandBuffer)
		.setSignalSemaphores(nullptr),
		*buildPyramidFence
	);
	// 2. Perform ray casting to generate vertex maps and normals.
	const std::array<RayCastingDescriptorSet, KinectFusion::NUM_PYRAMID_LEVELS>& rayCastingDescriptorSets = this->_poseEstimationAlgorithmData.rayCastingDescriptorSets;
	const vk::raii::CommandBuffer& rayCastingCommandBuffer = this->_poseEstimationAlgorithmData.rayCastingCommandBuffer;
	const vk::raii::Fence& rayCastingFence = this->_poseEstimationAlgorithmData.rayCastingFence;
	const std::array<PyramidData, KinectFusion::NUM_PYRAMID_LEVELS>& modelPyramid = this->_poseEstimationAlgorithmData.modelPyramid;
	rayCastingCommandBuffer.begin(
		vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
		.setPInheritanceInfo(nullptr)
	);
	rayCastingCommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *this->_rayCastingICPPipeline);
	this->_tsdfVolume.bind(rayCastingCommandBuffer, vk::PipelineBindPoint::eCompute, this->_rayCastingICPPipelineLayout, 0);
	for (std::uint32_t level = 0; level < KinectFusion::NUM_PYRAMID_LEVELS; ++level) {
		Camera levelCamera = camera_;
		levelCamera.resize(modelPyramid[level].texture(0).extent());
		jjyou::glsl::mat3 projection = levelCamera.getVisionProjection();
		rayCastingDescriptorSets[level].rayCastingParameters().fx = projection[0][0];
		rayCastingDescriptorSets[level].rayCastingParameters().fy = projection[1][1];
		rayCastingDescriptorSets[level].rayCastingParameters().cx = projection[2][0];
		rayCastingDescriptorSets[level].rayCastingParameters().cy = projection[2][1];
		rayCastingDescriptorSets[level].rayCastingParameters().invView = jjyou::glsl::inverse(initialView_);
		rayCastingDescriptorSets[level].rayCastingParameters().minDepth = this->_minDepth;
		rayCastingDescriptorSets[level].rayCastingParameters().maxDepth = this->_maxDepth;
		rayCastingDescriptorSets[level].rayCastingParameters().invalidDepth = this->_invalidDepth;
		rayCastingDescriptorSets[level].rayCastingParameters().marchingStep = 0.5f * this->_tsdfVolume.size();
		rayCastingDescriptorSets[level].bind(rayCastingCommandBuffer, vk::PipelineBindPoint::eCompute, this->_rayCastingICPPipelineLayout, 1);
		modelPyramid[level].bind(rayCastingCommandBuffer, vk::PipelineBindPoint::eCompute, this->_rayCastingICPPipelineLayout, 2);
		rayCastingCommandBuffer.dispatch(
			(modelPyramid[level].texture(0).extent().width + KinectFusion::_rayCastingICPWorkGroupSize.x - 1U) / KinectFusion::_rayCastingICPWorkGroupSize.x,
			(modelPyramid[level].texture(0).extent().height + KinectFusion::_rayCastingICPWorkGroupSize.y - 1U) / KinectFusion::_rayCastingICPWorkGroupSize.y,
			1U
		);
	}
	rayCastingCommandBuffer.end();
	this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Compute)->submit(
		vk::SubmitInfo()
		.setWaitSemaphores(nullptr)
		.setWaitDstStageMask(nullptr)
		.setCommandBuffers(*rayCastingCommandBuffer)
		.setSignalSemaphores(nullptr),
		*rayCastingFence
	);
	waitResult = this->_pEngine->context().device().waitForFences({ *buildPyramidFence, *rayCastingFence }, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
	VK_CHECK(waitResult);
	this->_pEngine->context().device().resetFences(*buildPyramidFence);
	this->_pEngine->context().device().resetFences(*rayCastingFence);
	buildPyramidCommandBuffer.reset(vk::CommandBufferResetFlags(0));
	rayCastingCommandBuffer.reset(vk::CommandBufferResetFlags(0));
	// 3. Perform ICP, from coarse to fine.
	const ICPDescriptorSet& icpDescriptorSet = this->_poseEstimationAlgorithmData.icpDescriptorSet;
	const vk::raii::CommandBuffer& icpCommandBuffer = this->_poseEstimationAlgorithmData.icpCommandBuffer;
	const vk::raii::Fence& icpFence = this->_poseEstimationAlgorithmData.icpFence;
	jjyou::glsl::mat4 estimatedInvView = jjyou::glsl::inverse(initialView_);
	Eigen::Map<Eigen::Matrix4f> estimatedInvViewEigen(estimatedInvView.data.data());
	// Starting with the coarsest level.
	for (std::uint32_t reverseLevel = 0; reverseLevel < KinectFusion::NUM_PYRAMID_LEVELS; ++reverseLevel) {
		std::uint32_t level = KinectFusion::NUM_PYRAMID_LEVELS - 1U - reverseLevel;
		Camera levelCamera = camera_;
		levelCamera.resize(framePyramid[level].texture(0).extent());
		jjyou::glsl::mat3 projection = levelCamera.getVisionProjection();
		icpDescriptorSet.icpParameters().modelView = initialView_;
		icpDescriptorSet.icpParameters().fx = projection[0][0];
		icpDescriptorSet.icpParameters().fy = projection[1][1];
		icpDescriptorSet.icpParameters().cx = projection[2][0];
		icpDescriptorSet.icpParameters().cy = projection[2][1];
		icpDescriptorSet.icpParameters().distanceThreshold = distanceThreshold_;
		icpDescriptorSet.icpParameters().angleThreshold = angleThreshold_;
		// Iteratively build and solve linear functions.
		for (std::uint32_t icpIteration = 0; icpIteration < KinectFusion::NUM_ICP_ITERATIONS[level]; ++icpIteration) {
			// Build linear function
			icpCommandBuffer.begin(
				vk::CommandBufferBeginInfo()
				.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
				.setPInheritanceInfo(nullptr)
			);
			framePyramid[level].bind(icpCommandBuffer, vk::PipelineBindPoint::eCompute, this->_buildLinearFunctionPipelineLayout, 0);
			modelPyramid[level].bind(icpCommandBuffer, vk::PipelineBindPoint::eCompute, this->_buildLinearFunctionPipelineLayout, 1);
			icpDescriptorSet.icpParameters().frameInvView = estimatedInvView;
			icpDescriptorSet.bind(icpCommandBuffer, vk::PipelineBindPoint::eCompute, this->_buildLinearFunctionPipelineLayout, 2);
			icpCommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *this->_buildLinearFunctionPipeline);
			jjyou::glsl::uvec3 numWorkGroups(
				(modelPyramid[level].texture(0).extent().width + KinectFusion::_buildLinearFunctionWorkGroupSize.x - 1U) / KinectFusion::_buildLinearFunctionWorkGroupSize.x,
				(modelPyramid[level].texture(0).extent().height + KinectFusion::_buildLinearFunctionWorkGroupSize.y - 1U) / KinectFusion::_buildLinearFunctionWorkGroupSize.y,
				1U
			);
			icpCommandBuffer.dispatch(numWorkGroups.x, numWorkGroups.y, numWorkGroups.z);
			// Insert a buffer memory barrier.
			readAfterWriteBufferMemoryBarrier.setBuffer(*icpDescriptorSet.globalSumBufferBuffer());
			icpCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(0), nullptr, readAfterWriteBufferMemoryBarrier, nullptr);
			// Sum reduction.
			_GlobalSumBufferLength globalSumBufferLength{
				.len = numWorkGroups.x * numWorkGroups.y * numWorkGroups.z
			};
			icpCommandBuffer.pushConstants<_GlobalSumBufferLength>(*this->_buildLinearFunctionReductionPipelineLayout, vk::ShaderStageFlagBits::eCompute, 0U, globalSumBufferLength);
			icpDescriptorSet.bind(icpCommandBuffer, vk::PipelineBindPoint::eCompute, this->_buildLinearFunctionReductionPipelineLayout, 0);
			icpCommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *this->_buildLinearFunctionReductionPipeline);
			icpCommandBuffer.dispatch(27U, 1U, 1U);
			icpCommandBuffer.end();
			this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Compute)->submit(
				vk::SubmitInfo()
				.setWaitSemaphores(nullptr)
				.setWaitDstStageMask(nullptr)
				.setCommandBuffers(*icpCommandBuffer)
				.setSignalSemaphores(nullptr),
				*icpFence
			);
			waitResult = this->_pEngine->context().device().waitForFences(*icpFence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
			VK_CHECK(waitResult);
			this->_pEngine->context().device().resetFences(*icpFence);
			icpCommandBuffer.reset(vk::CommandBufferResetFlags(0));
			// Download data.
			int counter = 0;
			Eigen::Matrix<float, 6, 6> A{};
			Eigen::Vector<float, 6> b{};
			for (int i = 0; i < 6; ++i) {
				for (int j = i; j < 7; ++j) {
					if (j == 6)
						b[i] = icpDescriptorSet.reductionResult().data[counter];
					else
						A(i, j) = A(j, i) = icpDescriptorSet.reductionResult().data[counter];
					++counter;
				}
			}
			// Solve the function
			float det = A.determinant();
			if (std::isnan(det) || std::abs(det) < 100000.0f)
				return std::nullopt;
			Eigen::Matrix<float, 6, 1> x{ A.fullPivLu().solve(b).cast<float>() };
			Eigen::Quaternionf rotation =
				Eigen::AngleAxisf(x(2), Eigen::Vector3f::UnitZ()) *
				Eigen::AngleAxisf(x(1), Eigen::Vector3f::UnitY()) *
				Eigen::AngleAxisf(x(0), Eigen::Vector3f::UnitX());
			Eigen::Vector3f translation = x.tail<3>();
			Eigen::Matrix4f deltaTransform = Eigen::Matrix4f::Identity();
			deltaTransform.topLeftCorner<3, 3>() = rotation.matrix();
			deltaTransform.topRightCorner<3, 1>() = translation;
			estimatedInvViewEigen = deltaTransform * estimatedInvViewEigen;
		}
	}
	return jjyou::glsl::inverse(estimatedInvView);
}

void KinectFusion::fuse(
	const Surface<Simple>& surface_,
	const Camera& camera_,
	const jjyou::glsl::mat4& view_
) const {
	const FusionDescriptorSet& fusionDescriptorSet = this->_fusionAlgorithmData.descriptorSet;
	const vk::raii::CommandBuffer& commandBuffer = this->_fusionAlgorithmData.commandBuffer;
	const vk::raii::Fence& fence = this->_fusionAlgorithmData.fence;
	commandBuffer.begin(
		vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
		.setPInheritanceInfo(nullptr)
	);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *this->_fusionPipeline);
	this->_tsdfVolume.bind(commandBuffer, vk::PipelineBindPoint::eCompute, this->_fusionPipelineLayout, 0);
	jjyou::glsl::mat3 projection = camera_.getVisionProjection();
	fusionDescriptorSet.fusionParameters().fx = projection[0][0];
	fusionDescriptorSet.fusionParameters().fy = projection[1][1];
	fusionDescriptorSet.fusionParameters().cx = projection[2][0];
	fusionDescriptorSet.fusionParameters().cy = projection[2][1];
	fusionDescriptorSet.fusionParameters().view = view_;
	fusionDescriptorSet.fusionParameters().viewPos = jjyou::glsl::vec4(-jjyou::glsl::transpose(jjyou::glsl::mat3(view_)) * jjyou::glsl::vec3(view_[3]), 1.0f);
	fusionDescriptorSet.fusionParameters().truncationWeight = static_cast<int>(this->_truncationWeight);
	fusionDescriptorSet.fusionParameters().minDepth = this->_minDepth;
	fusionDescriptorSet.fusionParameters().maxDepth = this->_maxDepth;
	fusionDescriptorSet.fusionParameters().invalidDepth = this->_invalidDepth;
	fusionDescriptorSet.bind(commandBuffer, vk::PipelineBindPoint::eCompute, this->_fusionPipelineLayout, 1);
	surface_.bindStorage(commandBuffer, vk::PipelineBindPoint::eCompute, this->_fusionPipelineLayout, 2);
	commandBuffer.dispatch(
		(this->_tsdfVolume.resolution().x + KinectFusion::_fusionWorkGroupSize.x - 1U) / KinectFusion::_fusionWorkGroupSize.x,
		(this->_tsdfVolume.resolution().y + KinectFusion::_fusionWorkGroupSize.y - 1U) / KinectFusion::_fusionWorkGroupSize.y,
		1U
	);
	commandBuffer.end();
	this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Compute)->submit(
		vk::SubmitInfo()
		.setWaitSemaphores(nullptr)
		.setWaitDstStageMask(nullptr)
		.setCommandBuffers(*commandBuffer)
		.setSignalSemaphores(nullptr),
		*fence
	);
	vk::Result waitResult = this->_pEngine->context().device().waitForFences(*fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
	VK_CHECK(waitResult);
	this->_pEngine->context().device().resetFences(*fence);
	commandBuffer.reset(vk::CommandBufferResetFlags(0));
}

void KinectFusion::_createDescriptorSetLayouts(void) {
	// TSDF volume storage buffer
	this->_tsdfVolumeDescriptorSetLayout = TSDFVolume::createDescriptorSetLayout(this->_pEngine->context().device());

	// Ray casting uniform block
	this->_rayCastingDescriptorSetLayout = RayCastingDescriptorSet::createDescriptorSetLayout(this->_pEngine->context().device());
	
	// Fusion uniform block
	this->_fusionDescriptorSetLayout = FusionDescriptorSet::createDescriptorSetLayout(this->_pEngine->context().device());

	// Pyramid data
	this->_pyramidDataDescriptorSetLayout = PyramidData::createDescriptorSetLayout(this->_pEngine->context().device());

	// ICP
	this->_icpDescriptorSetLayout = ICPDescriptorSet::createDescriptorSetLayout(this->_pEngine->context().device());
}

void KinectFusion::_createPipelineLayouts(void) {
	// Init volume
	{
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_tsdfVolumeDescriptorSetLayout
		};
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setFlags(vk::PipelineLayoutCreateFlags(0))
			.setSetLayouts(descriptorSetLayouts)
			.setPushConstantRanges(nullptr);
		this->_initVolumePipelineLayout = vk::raii::PipelineLayout(this->_pEngine->context().device(), pipelineLayoutCreateInfo);
	}

	// Ray casting
	{
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_tsdfVolumeDescriptorSetLayout,
			*this->_rayCastingDescriptorSetLayout,
			*this->_pEngine->surfaceStorageDescriptorSetLayout(MaterialType::Lambertian)
		};
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setFlags(vk::PipelineLayoutCreateFlags(0))
			.setSetLayouts(descriptorSetLayouts)
			.setPushConstantRanges(nullptr);
		this->_rayCastingPipelineLayout = vk::raii::PipelineLayout(this->_pEngine->context().device(), pipelineLayoutCreateInfo);
	}

	// Fusion
	{
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_tsdfVolumeDescriptorSetLayout,
			*this->_fusionDescriptorSetLayout,
			*this->_pEngine->surfaceStorageDescriptorSetLayout(MaterialType::Simple)
		};
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setFlags(vk::PipelineLayoutCreateFlags(0))
			.setSetLayouts(descriptorSetLayouts)
			.setPushConstantRanges(nullptr);
		this->_fusionPipelineLayout = vk::raii::PipelineLayout(this->_pEngine->context().device(), pipelineLayoutCreateInfo);
	}

	// Bilateral filtering
	{
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_pEngine->surfaceStorageDescriptorSetLayout(MaterialType::Simple),
			*this->_pyramidDataDescriptorSetLayout
		};
		vk::PushConstantRange pushConstantRange = vk::PushConstantRange()
			.setStageFlags(vk::ShaderStageFlagBits::eCompute)
			.setOffset(0U)
			.setSize(sizeof(KinectFusion::_BilateralFilteringParameters));
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setFlags(vk::PipelineLayoutCreateFlags(0))
			.setSetLayouts(descriptorSetLayouts)
			.setPushConstantRanges(pushConstantRange);
		this->_bilateralFilteringPipelineLayout = vk::raii::PipelineLayout(this->_pEngine->context().device(), pipelineLayoutCreateInfo);
	}

	// Ray casting ICP
	{
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_tsdfVolumeDescriptorSetLayout,
			*this->_rayCastingDescriptorSetLayout,
			*this->_pyramidDataDescriptorSetLayout
		};
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setFlags(vk::PipelineLayoutCreateFlags(0))
			.setSetLayouts(descriptorSetLayouts)
			.setPushConstantRanges(nullptr);
		this->_rayCastingICPPipelineLayout = vk::raii::PipelineLayout(this->_pEngine->context().device(), pipelineLayoutCreateInfo);
	}

	// Compute vertex/normal map
	{
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_pyramidDataDescriptorSetLayout
		};
		vk::PushConstantRange pushConstantRange = vk::PushConstantRange()
			.setStageFlags(vk::ShaderStageFlagBits::eCompute)
			.setOffset(0U)
			.setSize(sizeof(KinectFusion::_CameraIntrinsics));
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setFlags(vk::PipelineLayoutCreateFlags(0))
			.setSetLayouts(descriptorSetLayouts)
			.setPushConstantRanges(pushConstantRange);
		this->_computeVertexNormalMapPipelineLayout = vk::raii::PipelineLayout(this->_pEngine->context().device(), pipelineLayoutCreateInfo);
	}

	// Half-sampling
	{
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_pyramidDataDescriptorSetLayout,
			*this->_pyramidDataDescriptorSetLayout
		};
		vk::PushConstantRange pushConstantRange = vk::PushConstantRange()
			.setStageFlags(vk::ShaderStageFlagBits::eCompute)
			.setOffset(0U)
			.setSize(sizeof(KinectFusion::_HalfSamplingParameters));
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setFlags(vk::PipelineLayoutCreateFlags(0))
			.setSetLayouts(descriptorSetLayouts)
			.setPushConstantRanges(pushConstantRange);
		this->_halfSamplingPipelineLayout = vk::raii::PipelineLayout(this->_pEngine->context().device(), pipelineLayoutCreateInfo);
	}

	// Build linear function
	{
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_pyramidDataDescriptorSetLayout,
			*this->_pyramidDataDescriptorSetLayout,
			*this->_icpDescriptorSetLayout
		};
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setFlags(vk::PipelineLayoutCreateFlags(0))
			.setSetLayouts(descriptorSetLayouts)
			.setPushConstantRanges(nullptr);
		this->_buildLinearFunctionPipelineLayout = vk::raii::PipelineLayout(this->_pEngine->context().device(), pipelineLayoutCreateInfo);
	}

	// Build linear function redunction
	{
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_icpDescriptorSetLayout
		};
		vk::PushConstantRange pushConstantRange = vk::PushConstantRange()
			.setStageFlags(vk::ShaderStageFlagBits::eCompute)
			.setOffset(0U)
			.setSize(sizeof(KinectFusion::_GlobalSumBufferLength));
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setFlags(vk::PipelineLayoutCreateFlags(0))
			.setSetLayouts(descriptorSetLayouts)
			.setPushConstantRanges(pushConstantRange);
		this->_buildLinearFunctionReductionPipelineLayout = vk::raii::PipelineLayout(this->_pEngine->context().device(), pipelineLayoutCreateInfo);
	}
}

void KinectFusion::_createPipelines(void) {
	// Init volume
	{
#include "./shader/spv/initVolume.comp.spv.h"
		vk::raii::ShaderModule shaderModule(this->_pEngine->context().device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(initVolume_comp_spv))
			.setCodeSize(sizeof(initVolume_comp_spv))
		);
		vk::ComputePipelineCreateInfo computePipelineCreateInfo = vk::ComputePipelineCreateInfo()
			.setFlags(vk::PipelineCreateFlags(0))
			.setStage(
				vk::PipelineShaderStageCreateInfo()
				.setFlags(vk::PipelineShaderStageCreateFlags(0))
				.setStage(vk::ShaderStageFlagBits::eCompute)
				.setModule(*shaderModule)
				.setPName("main")
				.setPSpecializationInfo(nullptr)
			)
			.setLayout(*this->_initVolumePipelineLayout)
			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(0);
		this->_initVolumePipeline = vk::raii::Pipeline(this->_pEngine->context().device(), nullptr, computePipelineCreateInfo);
	}

	// Ray casting
	{
#include "./shader/spv/rayCasting.comp.spv.h"
		vk::raii::ShaderModule shaderModule(this->_pEngine->context().device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(rayCasting_comp_spv))
			.setCodeSize(sizeof(rayCasting_comp_spv))
		);
		vk::ComputePipelineCreateInfo computePipelineCreateInfo = vk::ComputePipelineCreateInfo()
			.setFlags(vk::PipelineCreateFlags(0))
			.setStage(
				vk::PipelineShaderStageCreateInfo()
				.setFlags(vk::PipelineShaderStageCreateFlags(0))
				.setStage(vk::ShaderStageFlagBits::eCompute)
				.setModule(*shaderModule)
				.setPName("main")
				.setPSpecializationInfo(nullptr)
			)
			.setLayout(*this->_rayCastingPipelineLayout)
			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(0);
		this->_rayCastingPipeline = vk::raii::Pipeline(this->_pEngine->context().device(), nullptr, computePipelineCreateInfo);
	}

	// Fusion
	{
#include "./shader/spv/fusion.comp.spv.h"
		vk::raii::ShaderModule shaderModule(this->_pEngine->context().device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(fusion_comp_spv))
			.setCodeSize(sizeof(fusion_comp_spv))
		);
		vk::ComputePipelineCreateInfo computePipelineCreateInfo = vk::ComputePipelineCreateInfo()
			.setFlags(vk::PipelineCreateFlags(0))
			.setStage(
				vk::PipelineShaderStageCreateInfo()
				.setFlags(vk::PipelineShaderStageCreateFlags(0))
				.setStage(vk::ShaderStageFlagBits::eCompute)
				.setModule(*shaderModule)
				.setPName("main")
				.setPSpecializationInfo(nullptr)
			)
			.setLayout(*this->_fusionPipelineLayout)
			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(0);
		this->_fusionPipeline = vk::raii::Pipeline(this->_pEngine->context().device(), nullptr, computePipelineCreateInfo);
	}

	// Bilateral filtering
	{
#include "./shader/spv/bilateralFiltering.comp.spv.h"
		vk::raii::ShaderModule shaderModule(this->_pEngine->context().device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(bilateralFiltering_comp_spv))
			.setCodeSize(sizeof(bilateralFiltering_comp_spv))
		);
		vk::ComputePipelineCreateInfo computePipelineCreateInfo = vk::ComputePipelineCreateInfo()
			.setFlags(vk::PipelineCreateFlags(0))
			.setStage(
				vk::PipelineShaderStageCreateInfo()
				.setFlags(vk::PipelineShaderStageCreateFlags(0))
				.setStage(vk::ShaderStageFlagBits::eCompute)
				.setModule(*shaderModule)
				.setPName("main")
				.setPSpecializationInfo(nullptr)
			)
			.setLayout(*this->_bilateralFilteringPipelineLayout)
			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(0);
		this->_bilateralFilteringPipeline = vk::raii::Pipeline(this->_pEngine->context().device(), nullptr, computePipelineCreateInfo);
	}

	// Ray casting for ICP
	{
#include "./shader/spv/rayCastingICP.comp.spv.h"
		vk::raii::ShaderModule shaderModule(this->_pEngine->context().device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(rayCastingICP_comp_spv))
			.setCodeSize(sizeof(rayCastingICP_comp_spv))
		);
		vk::ComputePipelineCreateInfo computePipelineCreateInfo = vk::ComputePipelineCreateInfo()
			.setFlags(vk::PipelineCreateFlags(0))
			.setStage(
				vk::PipelineShaderStageCreateInfo()
				.setFlags(vk::PipelineShaderStageCreateFlags(0))
				.setStage(vk::ShaderStageFlagBits::eCompute)
				.setModule(*shaderModule)
				.setPName("main")
				.setPSpecializationInfo(nullptr)
			)
			.setLayout(*this->_rayCastingICPPipelineLayout)
			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(0);
		this->_rayCastingICPPipeline = vk::raii::Pipeline(this->_pEngine->context().device(), nullptr, computePipelineCreateInfo);
	}

	// Compute vertex map
	{
#include "./shader/spv/computeVertexMap.comp.spv.h"
		vk::raii::ShaderModule shaderModule(this->_pEngine->context().device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(computeVertexMap_comp_spv))
			.setCodeSize(sizeof(computeVertexMap_comp_spv))
		);
		vk::ComputePipelineCreateInfo computePipelineCreateInfo = vk::ComputePipelineCreateInfo()
			.setFlags(vk::PipelineCreateFlags(0))
			.setStage(
				vk::PipelineShaderStageCreateInfo()
				.setFlags(vk::PipelineShaderStageCreateFlags(0))
				.setStage(vk::ShaderStageFlagBits::eCompute)
				.setModule(*shaderModule)
				.setPName("main")
				.setPSpecializationInfo(nullptr)
			)
			.setLayout(*this->_computeVertexNormalMapPipelineLayout)
			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(0);
		this->_computeVertexMapPipeline = vk::raii::Pipeline(this->_pEngine->context().device(), nullptr, computePipelineCreateInfo);
	}

	// Compute normal map
	{
#include "./shader/spv/computeNormalMap.comp.spv.h"
		vk::raii::ShaderModule shaderModule(this->_pEngine->context().device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(computeNormalMap_comp_spv))
			.setCodeSize(sizeof(computeNormalMap_comp_spv))
		);
		vk::ComputePipelineCreateInfo computePipelineCreateInfo = vk::ComputePipelineCreateInfo()
			.setFlags(vk::PipelineCreateFlags(0))
			.setStage(
				vk::PipelineShaderStageCreateInfo()
				.setFlags(vk::PipelineShaderStageCreateFlags(0))
				.setStage(vk::ShaderStageFlagBits::eCompute)
				.setModule(*shaderModule)
				.setPName("main")
				.setPSpecializationInfo(nullptr)
			)
			.setLayout(*this->_computeVertexNormalMapPipelineLayout)
			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(0);
		this->_computeNormalMapPipeline = vk::raii::Pipeline(this->_pEngine->context().device(), nullptr, computePipelineCreateInfo);
	}

	// Half-sampling
	{
#include "./shader/spv/halfSampling.comp.spv.h"
		vk::raii::ShaderModule shaderModule(this->_pEngine->context().device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(halfSampling_comp_spv))
			.setCodeSize(sizeof(halfSampling_comp_spv))
		);
		vk::ComputePipelineCreateInfo computePipelineCreateInfo = vk::ComputePipelineCreateInfo()
			.setFlags(vk::PipelineCreateFlags(0))
			.setStage(
				vk::PipelineShaderStageCreateInfo()
				.setFlags(vk::PipelineShaderStageCreateFlags(0))
				.setStage(vk::ShaderStageFlagBits::eCompute)
				.setModule(*shaderModule)
				.setPName("main")
				.setPSpecializationInfo(nullptr)
			)
			.setLayout(*this->_halfSamplingPipelineLayout)
			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(0);
		this->_halfSamplingPipeline = vk::raii::Pipeline(this->_pEngine->context().device(), nullptr, computePipelineCreateInfo);
	}

	// Build linear function
	{
#include "./shader/spv/buildLinearFunction.comp.spv.h"
		vk::raii::ShaderModule shaderModule(this->_pEngine->context().device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(buildLinearFunction_comp_spv))
			.setCodeSize(sizeof(buildLinearFunction_comp_spv))
		);
		vk::ComputePipelineCreateInfo computePipelineCreateInfo = vk::ComputePipelineCreateInfo()
			.setFlags(vk::PipelineCreateFlags(0))
			.setStage(
				vk::PipelineShaderStageCreateInfo()
				.setFlags(vk::PipelineShaderStageCreateFlags(0))
				.setStage(vk::ShaderStageFlagBits::eCompute)
				.setModule(*shaderModule)
				.setPName("main")
				.setPSpecializationInfo(nullptr)
			)
			.setLayout(*this->_buildLinearFunctionPipelineLayout)
			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(0);
		this->_buildLinearFunctionPipeline = vk::raii::Pipeline(this->_pEngine->context().device(), nullptr, computePipelineCreateInfo);
	}

	// Build linear function reduction
	{
#include "./shader/spv/buildLinearFunctionReduction.comp.spv.h"
		vk::raii::ShaderModule shaderModule(this->_pEngine->context().device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(buildLinearFunctionReduction_comp_spv))
			.setCodeSize(sizeof(buildLinearFunctionReduction_comp_spv))
		);
		vk::ComputePipelineCreateInfo computePipelineCreateInfo = vk::ComputePipelineCreateInfo()
			.setFlags(vk::PipelineCreateFlags(0))
			.setStage(
				vk::PipelineShaderStageCreateInfo()
				.setFlags(vk::PipelineShaderStageCreateFlags(0))
				.setStage(vk::ShaderStageFlagBits::eCompute)
				.setModule(*shaderModule)
				.setPName("main")
				.setPSpecializationInfo(nullptr)
			)
			.setLayout(*this->_buildLinearFunctionReductionPipelineLayout)
			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(0);
		this->_buildLinearFunctionReductionPipeline = vk::raii::Pipeline(this->_pEngine->context().device(), nullptr, computePipelineCreateInfo);
	}
}

void KinectFusion::_createAlgorithmData(void) {
	// Init volume
	{
		vk::raii::CommandBuffer& commandBuffer = this->_initVolumeAlgorithmData.commandBuffer;
		vk::raii::Fence& fence = this->_initVolumeAlgorithmData.fence;
		commandBuffer = std::move(this->_pEngine->context().device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(*this->_pEngine->commandPool(jjyou::vk::Context::QueueType::Compute))
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
		)[0]);
		fence = vk::raii::Fence(
			this->_pEngine->context().device(),
			vk::FenceCreateInfo(vk::FenceCreateFlags(0))
		);
	}

	// Ray casting
	{
		RayCastingDescriptorSet& rayCastingDescriptorSet = this->_rayCastingAlgorithmData.descriptorSet;
		vk::raii::CommandBuffer& commandBuffer = this->_rayCastingAlgorithmData.commandBuffer;
		vk::raii::Fence& fence = this->_rayCastingAlgorithmData.fence;
		rayCastingDescriptorSet = RayCastingDescriptorSet(*this->_pEngine, *this);
		commandBuffer = std::move(this->_pEngine->context().device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(*this->_pEngine->commandPool(jjyou::vk::Context::QueueType::Compute))
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
		)[0]);
		fence = vk::raii::Fence(
			this->_pEngine->context().device(),
			vk::FenceCreateInfo(vk::FenceCreateFlags(0))
		);
	}

	// Fusion
	{
		FusionDescriptorSet& fusionDescriptorSet = this->_fusionAlgorithmData.descriptorSet;
		vk::raii::Fence& fence = this->_fusionAlgorithmData.fence;
		vk::raii::CommandBuffer& commandBuffer = this->_fusionAlgorithmData.commandBuffer;
		fusionDescriptorSet = FusionDescriptorSet(*this->_pEngine, *this);
		commandBuffer = std::move(this->_pEngine->context().device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(*this->_pEngine->commandPool(jjyou::vk::Context::QueueType::Compute))
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
		)[0]);
		fence = vk::raii::Fence(
			this->_pEngine->context().device(),
			vk::FenceCreateInfo(vk::FenceCreateFlags(0))
		);
	}

	// Pose estimation
	{
		std::array<PyramidData, KinectFusion::NUM_PYRAMID_LEVELS>& framePyramid = this->_poseEstimationAlgorithmData.framePyramid;
		std::array<PyramidData, KinectFusion::NUM_PYRAMID_LEVELS>& modelPyramid = this->_poseEstimationAlgorithmData.modelPyramid;
		vk::raii::CommandBuffer& buildPyramidCommandBuffer = this->_poseEstimationAlgorithmData.buildPyramidCommandBuffer;
		vk::raii::Fence& buildPyramidFence = this->_poseEstimationAlgorithmData.buildPyramidFence;
		std::array<RayCastingDescriptorSet, KinectFusion::NUM_PYRAMID_LEVELS>& rayCastingDescriptorSets = this->_poseEstimationAlgorithmData.rayCastingDescriptorSets;
		vk::raii::CommandBuffer& rayCastingCommandBuffer = this->_poseEstimationAlgorithmData.rayCastingCommandBuffer;
		vk::raii::Fence& rayCastingFence = this->_poseEstimationAlgorithmData.rayCastingFence;
		ICPDescriptorSet& icpDescriptorSet = this->_poseEstimationAlgorithmData.icpDescriptorSet;
		vk::raii::CommandBuffer& icpCommandBuffer = this->_poseEstimationAlgorithmData.icpCommandBuffer;
		vk::raii::Fence& icpFence = this->_poseEstimationAlgorithmData.icpFence;
		vk::Extent2D levelExtent = _depthFrameExtent;
		for (std::uint32_t level = 0; level < KinectFusion::NUM_PYRAMID_LEVELS; ++level) {
			this->_poseEstimationAlgorithmData.modelPyramid[level] = PyramidData(*this->_pEngine, *this, levelExtent);
			this->_poseEstimationAlgorithmData.framePyramid[level] = PyramidData(*this->_pEngine, *this, levelExtent);
			levelExtent.width /= 2U;
			levelExtent.height /= 2U;
		}
		buildPyramidCommandBuffer = std::move(this->_pEngine->context().device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(*this->_pEngine->commandPool(jjyou::vk::Context::QueueType::Compute))
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
		)[0]);
		buildPyramidFence = vk::raii::Fence(
			this->_pEngine->context().device(),
			vk::FenceCreateInfo(vk::FenceCreateFlags(0))
		);
		for (std::uint32_t level = 0; level < KinectFusion::NUM_PYRAMID_LEVELS; ++level)
			rayCastingDescriptorSets[level] = RayCastingDescriptorSet(*this->_pEngine, *this);
		rayCastingCommandBuffer = std::move(this->_pEngine->context().device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(*this->_pEngine->commandPool(jjyou::vk::Context::QueueType::Compute))
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
		)[0]);
		rayCastingFence = vk::raii::Fence(
			this->_pEngine->context().device(),
			vk::FenceCreateInfo(vk::FenceCreateFlags(0))
		);
		jjyou::glsl::uvec3 buildLinearFunctionWorkGroupCount(
			(framePyramid[0].texture(0).extent().width + KinectFusion::_buildLinearFunctionWorkGroupSize.x - 1U) / KinectFusion::_buildLinearFunctionWorkGroupSize.x,
			(framePyramid[0].texture(0).extent().height + KinectFusion::_buildLinearFunctionWorkGroupSize.y - 1U) / KinectFusion::_buildLinearFunctionWorkGroupSize.y,
			1U
		);
		icpDescriptorSet = ICPDescriptorSet(*this->_pEngine, *this, static_cast<vk::DeviceSize>(buildLinearFunctionWorkGroupCount.x * buildLinearFunctionWorkGroupCount.y * buildLinearFunctionWorkGroupCount.z));
		icpCommandBuffer = std::move(this->_pEngine->context().device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(*this->_pEngine->commandPool(jjyou::vk::Context::QueueType::Compute))
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
		)[0]);
		icpFence = vk::raii::Fence(
			this->_pEngine->context().device(),
			vk::FenceCreateInfo(vk::FenceCreateFlags(0))
		);
	}
}