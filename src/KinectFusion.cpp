#include "KinectFusion.hpp"

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
	const jjyou::glsl::mat3& projection_,
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
	rayCastingDescriptorSet.rayCastingParameters().invProjection = jjyou::glsl::mat4(jjyou::glsl::inverse(projection_));
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

void KinectFusion::fuse(
	const Surface<Simple>& surface_,
	const jjyou::glsl::mat3& projection_,
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
	fusionDescriptorSet.fusionParameters().projection = projection_;
	fusionDescriptorSet.fusionParameters().invProjection = jjyou::glsl::mat4(jjyou::glsl::inverse(projection_));
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

	// Ray casting
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
}