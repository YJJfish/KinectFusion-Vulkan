#include "TSDFVolume.hpp"

#define VK_THROW(err) \
	throw std::runtime_error("[TSDFVolume] Vulkan error in file " + std::string(__FILE__) + " line " + std::to_string(__LINE__) + ": " + vk::to_string(err))

#define VK_CHECK(value) \
	if (vk::Result err = (value); err != vk::Result::eSuccess) { VK_THROW(err); }

TSDFVolume::TSDFVolume(
	const Engine& engine_,
	const jjyou::glsl::uvec3& resolution_,
	float size_,
	std::optional<jjyou::glsl::vec3> corner_,
	std::optional<float> truncationDistance_
) :
	_pEngine(&engine_),
	_resolution(resolution_),
	_size(size_),
	_corner(corner_.has_value() ? (*corner_) : ((-resolution_ + 1U).cast<float>() * size_ / 2.0f)),
	_truncationDistance(truncationDistance_.has_value() ? (*truncationDistance_) : (3.0f * size_)),
	_bufferSize(32 + 8 * this->_resolution.x * this->_resolution.y * this->_resolution.z)
{
	this->_createDescriptorSetLayout();
	this->_createStorageBuffer();
	this->_createDescriptorSet();
	this->_createPipelineLayout();
	this->_createPipeline();
	this->reset();
}

void TSDFVolume::reset(void) const {
	vk::raii::Fence fence(
		this->_pEngine->context().device(),
		vk::FenceCreateInfo(vk::FenceCreateFlags(0))
	);
	vk::raii::CommandBuffer commandBuffer = std::move(this->_pEngine->context().device().allocateCommandBuffers(
		vk::CommandBufferAllocateInfo()
		.setCommandPool(*this->_pEngine->commandPool(jjyou::vk::Context::QueueType::Compute))
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1)
	)[0]);
	commandBuffer.begin(vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
		.setPInheritanceInfo(nullptr)
	);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *this->_initVolumePipeline);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *this->_pipelineLayout, 0, *this->_descriptorSet, nullptr);
	constexpr jjyou::glsl::uvec2 localSize(32U, 32U);
	commandBuffer.dispatch((this->_resolution.x + localSize.x - 1U) / localSize.x, (this->_resolution.y + localSize.y - 1U) / localSize.y, 1U);
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
}

void TSDFVolume::_createDescriptorSetLayout(void) {
	std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
		vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorType(vk::DescriptorType::eStorageBuffer)
		.setDescriptorCount(1)
		.setStageFlags(vk::ShaderStageFlagBits::eCompute)
		.setPImmutableSamplers(nullptr)
	};
	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
		.setFlags(vk::DescriptorSetLayoutCreateFlags(0))
		.setBindings(descriptorSetLayoutBindings);
	this->_descriptorSetLayout = vk::raii::DescriptorSetLayout(this->_pEngine->context().device(), descriptorSetLayoutCreateInfo);
}

void TSDFVolume::_createStorageBuffer(void) {
	vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
		.setFlags(vk::BufferCreateFlags(0))
		.setSize(this->_bufferSize)
		.setUsage(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setQueueFamilyIndices(*this->_pEngine->context().queueFamilyIndex(jjyou::vk::Context::QueueType::Compute));
	VmaAllocationCreateInfo vmaAllocationCreateInfo{
		.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
		.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		.requiredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.preferredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.memoryTypeBits = 0,
		.pool = nullptr,
		.pUserData = nullptr,
		.priority = 0.0f,
	};
	VkBuffer storageBuffer = nullptr;
	VmaAllocation storageBufferMemory = nullptr;
	vmaCreateBuffer(*this->_pEngine->allocator(), reinterpret_cast<VkBufferCreateInfo*>(&bufferCreateInfo), &vmaAllocationCreateInfo, &storageBuffer, &storageBufferMemory, nullptr);
	this->_volume = vk::raii::Buffer(this->_pEngine->context().device(), storageBuffer);
	this->_volumeMemory = jjyou::vk::VmaAllocation(this->_pEngine->allocator(), storageBufferMemory);
}

void TSDFVolume::_createDescriptorSet(void) {
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(*this->_pEngine->descriptorPool())
		.setSetLayouts(*this->_descriptorSetLayout);
	this->_descriptorSet = std::move(this->_pEngine->context().device().allocateDescriptorSets(descriptorSetAllocateInfo)[0]);
	vk::DescriptorBufferInfo descriptorBufferInfo(*this->_volume, 0, this->_bufferSize);
	vk::WriteDescriptorSet writeDescriptorSet = vk::WriteDescriptorSet()
		.setDstSet(*this->_descriptorSet)
		.setDstBinding(0)
		.setDstArrayElement(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eStorageBuffer)
		.setBufferInfo(descriptorBufferInfo);
	this->_pEngine->context().device().updateDescriptorSets({ writeDescriptorSet }, {});
}

void TSDFVolume::_createPipelineLayout(void) {
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
		*this->_descriptorSetLayout
	};
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
		.setFlags(vk::PipelineLayoutCreateFlags(0))
		.setSetLayouts(descriptorSetLayouts)
		.setPushConstantRanges(nullptr);
	this->_pipelineLayout = vk::raii::PipelineLayout(this->_pEngine->context().device(), pipelineLayoutCreateInfo);
}

void TSDFVolume::_createPipeline(void) {
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
		.setLayout(*this->_pipelineLayout)
		.setBasePipelineHandle(nullptr)
		.setBasePipelineIndex(0);
	this->_initVolumePipeline = vk::raii::Pipeline(this->_pEngine->context().device(), nullptr, computePipelineCreateInfo);
}