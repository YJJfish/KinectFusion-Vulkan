#include "KinectFusion.hpp"
#include <exception>
#include <stdexcept>

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
	const vk::raii::Fence& fence = this->_initVolumeAlgorithmData.fence;
	const vk::raii::CommandBuffer& commandBuffer = this->_initVolumeAlgorithmData.commandBuffer;
	const vk::SubmitInfo& submitInfo = this->_initVolumeAlgorithmData.submitInfo;
	this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Compute)->submit(
		submitInfo,
		*fence
	);
	vk::Result waitResult = this->_pEngine->context().device().waitForFences(*fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
	VK_CHECK(waitResult);
	this->_pEngine->context().device().resetFences(*fence);
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
	const vk::raii::Fence& fence = this->_rayCastingAlgorithmData.fence;
	const vk::raii::CommandBuffer& commandBuffer = this->_rayCastingAlgorithmData.commandBuffer;
	const vk::SubmitInfo& submitInfo = this->_rayCastingAlgorithmData.submitInfo;
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
	constexpr jjyou::glsl::uvec2 localSize(32U, 32U);
	commandBuffer.dispatch(
		(surface_.texture(0).extent().width + localSize.x - 1U) / localSize.x,
		(surface_.texture(0).extent().height + localSize.y - 1U) / localSize.y,
		1U
	);
	commandBuffer.end();
	this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Compute)->submit(
		submitInfo,
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
	// 1. Apply bilateral filtering to the input depth map. Build pyramid. Generate vertex maps and normals.
	
	// 2. Perform ray casting to generate vertex maps and normals.

	return std::nullopt;
}

void KinectFusion::fuse(
	const Surface<Simple>& surface_,
	const Camera& camera_,
	const jjyou::glsl::mat4& view_
) const {
	const FusionDescriptorSet& fusionDescriptorSet = this->_fusionAlgorithmData.descriptorSet;
	const vk::raii::Fence& fence = this->_fusionAlgorithmData.fence;
	const vk::raii::CommandBuffer& commandBuffer = this->_fusionAlgorithmData.commandBuffer;
	const vk::SubmitInfo& submitInfo = this->_fusionAlgorithmData.submitInfo;
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
	constexpr jjyou::glsl::uvec2 localSize(32U, 32U);
	commandBuffer.dispatch(
		(this->_tsdfVolume.resolution().x + localSize.x - 1U) / localSize.x,
		(this->_tsdfVolume.resolution().y + localSize.y - 1U) / localSize.y,
		1U
	);
	commandBuffer.end();
	this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Compute)->submit(
		submitInfo,
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
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setFlags(vk::PipelineLayoutCreateFlags(0))
			.setSetLayouts(descriptorSetLayouts)
			.setPushConstantRanges(nullptr);
		this->_buildLinearFunctionReductionPipelineLayout = vk::raii::PipelineLayout(this->_pEngine->context().device(), pipelineLayoutCreateInfo);
	}
}

void KinectFusion::_createPipelines(void) {
	// Init volume
	{
#include "./shader/initVolume.comp.spv.h"
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
#include "./shader/rayCasting.comp.spv.h"
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
#include "./shader/fusion.comp.spv.h"
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
#include "./shader/bilateralFiltering.comp.spv.h"
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
#include "./shader/rayCastingICP.comp.spv.h"
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
			.setLayout(*this->_rayCastingPipelineLayout) // Reuse the ray casting pipeline layout because `PyramidData` has the same descriptor layout as `Surface<MaterialType::Lambertian>`.
			.setBasePipelineHandle(nullptr)
			.setBasePipelineIndex(0);
		this->_rayCastingICPPipeline = vk::raii::Pipeline(this->_pEngine->context().device(), nullptr, computePipelineCreateInfo);
	}

	// Compute vertex map
	{
#include "./shader/computeVertexMap.comp.spv.h"
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
#include "./shader/computeNormalMap.comp.spv.h"
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

	// Build linear function
	{
#include "./shader/buildLinearFunction.comp.spv.h"
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
#include "./shader/buildLinearFunctionReduction.comp.spv.h"
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
		vk::raii::Fence& fence = this->_initVolumeAlgorithmData.fence;
		vk::raii::CommandBuffer& commandBuffer = this->_initVolumeAlgorithmData.commandBuffer;
		vk::SubmitInfo& submitInfo = this->_initVolumeAlgorithmData.submitInfo;
		fence = vk::raii::Fence(
			this->_pEngine->context().device(),
			vk::FenceCreateInfo(vk::FenceCreateFlags(0))
		);
		commandBuffer = std::move(this->_pEngine->context().device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(*this->_pEngine->commandPool(jjyou::vk::Context::QueueType::Compute))
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
		)[0]);
		commandBuffer.begin(
			vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlags(0))
			.setPInheritanceInfo(nullptr)
		);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *this->_initVolumePipeline);
		this->_tsdfVolume.bind(commandBuffer, vk::PipelineBindPoint::eCompute, this->_initVolumePipelineLayout, 0);
		constexpr jjyou::glsl::uvec2 localSize(32U, 32U);
		commandBuffer.dispatch(
			(this->_tsdfVolume.resolution().x + localSize.x - 1U) / localSize.x,
			(this->_tsdfVolume.resolution().y + localSize.y - 1U) / localSize.y,
			1U
		);
		commandBuffer.end();
		submitInfo
			.setWaitSemaphores(nullptr)
			.setWaitDstStageMask(nullptr)
			.setCommandBuffers(*commandBuffer)
			.setSignalSemaphores(nullptr);
	}

	// Ray casting
	{
		RayCastingDescriptorSet& rayCastingDescriptorSet = this->_rayCastingAlgorithmData.descriptorSet;
		vk::raii::Fence& fence = this->_rayCastingAlgorithmData.fence;
		vk::raii::CommandBuffer& commandBuffer = this->_rayCastingAlgorithmData.commandBuffer;
		vk::SubmitInfo& submitInfo = this->_rayCastingAlgorithmData.submitInfo;
		rayCastingDescriptorSet = RayCastingDescriptorSet(*this->_pEngine, *this);
		fence = vk::raii::Fence(
			this->_pEngine->context().device(),
			vk::FenceCreateInfo(vk::FenceCreateFlags(0))
		);
		commandBuffer = std::move(this->_pEngine->context().device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(*this->_pEngine->commandPool(jjyou::vk::Context::QueueType::Compute))
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
		)[0]);
		// Cannot record the command buffer in advance, because the ray casting
		// resolution differs in each call.
		submitInfo
			.setWaitSemaphores(nullptr)
			.setWaitDstStageMask(nullptr)
			.setCommandBuffers(*commandBuffer)
			.setSignalSemaphores(nullptr);
	}

	// Fusion
	{
		FusionDescriptorSet& fusionDescriptorSet = this->_fusionAlgorithmData.descriptorSet;
		vk::raii::Fence& fence = this->_fusionAlgorithmData.fence;
		vk::raii::CommandBuffer& commandBuffer = this->_fusionAlgorithmData.commandBuffer;
		vk::SubmitInfo& submitInfo = this->_fusionAlgorithmData.submitInfo;
		fusionDescriptorSet = FusionDescriptorSet(*this->_pEngine, *this);
		fence = vk::raii::Fence(
			this->_pEngine->context().device(),
			vk::FenceCreateInfo(vk::FenceCreateFlags(0))
		);
		commandBuffer = std::move(this->_pEngine->context().device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(*this->_pEngine->commandPool(jjyou::vk::Context::QueueType::Compute))
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
		)[0]);
		// Cannot record the command buffer in advance, because the fusion
		// may use different input textures in each call.
		submitInfo
			.setWaitSemaphores(nullptr)
			.setWaitDstStageMask(nullptr)
			.setCommandBuffers(*commandBuffer)
			.setSignalSemaphores(nullptr);
	}

	// Pose estimation
	{
		vk::Extent2D levelExtent = _depthFrameExtent;
		for (std::uint32_t level = 0; level < KinectFusion::NUM_PYRAMID_LEVELS; ++level) {
			this->_poseEstimationAlgorithmData.modelPyramid[level] = PyramidData(*this->_pEngine, *this, levelExtent);
			this->_poseEstimationAlgorithmData.framePyramid[level] = PyramidData(*this->_pEngine, *this, levelExtent);
			levelExtent.width /= 2U;
			levelExtent.height /= 2U;
		}
	}
}