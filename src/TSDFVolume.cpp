#include "TSDFVolume.hpp"

TSDFVolume::TSDFVolume(
	const jjyou::vk::Context& context_,
	const jjyou::vk::VmaAllocator& allocator_,
	const vk::raii::DescriptorPool& descriptorPool_,
	const jjyou::glsl::uvec3& resolution_,
	float size_,
	std::optional<jjyou::glsl::vec3> corner_,
	std::optional<float> truncationDistance_
) :
	_pContext(&context_),
	_pAllocator(&allocator_),
	_descriptorPool(*descriptorPool_),
	_resolution(resolution_),
	_size(size_),
	_corner(corner_.has_value() ? (*corner_) : (resolution_.cast<float>() * size_ / 2.0f)),
	_truncationDistance(truncationDistance_.has_value() ? (*truncationDistance_) : (3.0f * size_)),
	_bufferSize(32 + 8 * this->_resolution.x * this->_resolution.y * this->_resolution.z)
{
	this->_createDescriptorSetLayout();
	this->_createStorageBuffer();
	this->_createDescriptorSet();
}

void TSDFVolume::_createDescriptorSetLayout(void) {
	std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
			vk::DescriptorSetLayoutBinding(
				0,
				vk::DescriptorType::eStorageBuffer,
				1,
				vk::ShaderStageFlagBits::eCompute,
				nullptr
			)
	};
	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
		vk::DescriptorSetLayoutCreateFlags(0),
		descriptorSetLayoutBindings
	);
	this->_descriptorSetLayout = vk::raii::DescriptorSetLayout(this->_pContext->device(), descriptorSetLayoutCreateInfo);
}

void TSDFVolume::_createStorageBuffer(void) {
	vk::BufferCreateInfo bufferCreateInfo(
		vk::BufferCreateFlags(0),
		this->_bufferSize,
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
		vk::SharingMode::eExclusive,
		1U, &*this->_pContext->queueFamilyIndex(jjyou::vk::Context::QueueType::Compute)
	);
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
	vmaCreateBuffer(**this->_pAllocator, reinterpret_cast<VkBufferCreateInfo*>(&bufferCreateInfo), &vmaAllocationCreateInfo, &storageBuffer, &storageBufferMemory, nullptr);
	this->_volume = vk::raii::Buffer(this->_pContext->device(), storageBuffer);
	this->_volumeMemory = jjyou::vk::VmaAllocation(*this->_pAllocator, storageBufferMemory);
}

void TSDFVolume::_createDescriptorSet(void) {
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(
		this->_descriptorPool,
		1U,
		&*this->_descriptorSetLayout
	);
	this->_descriptorSet = std::move(this->_pContext->device().allocateDescriptorSets(descriptorSetAllocateInfo)[0]);
	vk::DescriptorBufferInfo descriptorBufferInfo(*this->_volume, 0, this->_bufferSize);
	vk::WriteDescriptorSet writeDescriptorSet(
		*this->_descriptorSet,
		0,
		0,
		1,
		vk::DescriptorType::eStorageBuffer,
		nullptr,
		&descriptorBufferInfo,
		nullptr
	);
	this->_pContext->device().updateDescriptorSets({ writeDescriptorSet }, {});
}