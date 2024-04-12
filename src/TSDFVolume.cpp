#include "TSDFVolume.hpp"
#include "KinectFusion.hpp"

#define VK_THROW(err) \
	throw std::runtime_error("[TSDFVolume] Vulkan error in file " + std::string(__FILE__) + " line " + std::to_string(__LINE__) + ": " + vk::to_string(err))

#define VK_CHECK(value) \
	if (vk::Result err = (value); err != vk::Result::eSuccess) { VK_THROW(err); }

TSDFVolume::TSDFVolume(
	const Engine& engine_,
	const KinectFusion& kinectFusion_,
	const jjyou::glsl::uvec3& resolution_,
	float size_,
	std::optional<jjyou::glsl::vec3> corner_,
	std::optional<float> truncationDistance_
) :
	_pEngine(&engine_),
	_pKinectFusion(&kinectFusion_),
	_descriptorSetLayout(*kinectFusion_.tsdfVolumeDescriptorSetLayout()),
	_resolution(resolution_),
	_size(size_),
	_corner(corner_.has_value() ? (*corner_) : (-(resolution_ - 1U).cast<float>() * size_ / 2.0f)),
	_truncationDistance(truncationDistance_.has_value() ? (*truncationDistance_) : (3.0f * size_)),
	_bufferSize(sizeof(TSDFVolume::TSDFParams) + sizeof(jjyou::glsl::vec2) * this->_resolution.x * this->_resolution.y * this->_resolution.z)
{
	this->_createStorageBuffer();
	this->_createDescriptorSet();
}

void TSDFVolume::_createStorageBuffer(void) {
	// Create a storage buffer.
	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setFlags(vk::BufferCreateFlags(0))
			.setSize(this->_bufferSize)
			.setUsage(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setQueueFamilyIndices(nullptr);
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
	// Create a staging buffer and copy the header information.
	// Since the storage buffer is not large, we will do the copy on the compute queue.
	vk::raii::Buffer stagingBuffer{ nullptr };
	jjyou::vk::VmaAllocation stagingBufferMemory{ nullptr };
	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setFlags(vk::BufferCreateFlags(0))
			.setSize(sizeof(TSDFVolume::TSDFParams))
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setQueueFamilyIndices(nullptr);
		VmaAllocationCreateInfo vmaAllocationCreateInfo{
			.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT,
			.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO,
			.requiredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			.preferredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			.memoryTypeBits = 0,
			.pool = nullptr,
			.pUserData = nullptr,
			.priority = 0.0f,
		};
		VkBuffer pStagingBuffer = nullptr;
		VmaAllocation pStagingBufferMemory = nullptr;
		VmaAllocationInfo allocationInfo{};
		vmaCreateBuffer(*this->_pEngine->allocator(), reinterpret_cast<VkBufferCreateInfo*>(&bufferCreateInfo), &vmaAllocationCreateInfo, &pStagingBuffer, &pStagingBufferMemory, &allocationInfo);
		stagingBuffer = vk::raii::Buffer(this->_pEngine->context().device(), pStagingBuffer);
		stagingBufferMemory = jjyou::vk::VmaAllocation(this->_pEngine->allocator(), pStagingBufferMemory);
		TSDFVolume::TSDFParams params {
			.resolution = this->_resolution,
			.size = this->_size,
			.corner = this->_corner,
			.truncationDistance = this->_truncationDistance
		};
		memcpy(allocationInfo.pMappedData, &params, sizeof(TSDFVolume::TSDFParams));
	}
	// Create a compute command buffer and copy staging buffer to the final storage buffer
	{
		vk::raii::CommandBuffer computeCommandBuffer = std::move(this->_pEngine->context().device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(*this->_pEngine->commandPool(jjyou::vk::Context::QueueType::Compute))
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
		)[0]);
		vk::raii::Fence fence = vk::raii::Fence(this->_pEngine->context().device(), vk::FenceCreateInfo(vk::FenceCreateFlags(0)));
		computeCommandBuffer.begin(vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
			.setPInheritanceInfo(nullptr)
		);
		computeCommandBuffer.copyBuffer(*stagingBuffer, *this->_volume, vk::BufferCopy(0, 0, sizeof(TSDFVolume::TSDFParams)));
		computeCommandBuffer.end();
		this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Compute)->submit(
			vk::SubmitInfo()
			.setWaitSemaphores(nullptr)
			.setWaitDstStageMask(nullptr)
			.setCommandBuffers(*computeCommandBuffer)
			.setSignalSemaphores(nullptr),
			*fence
		);
		vk::Result waitResult = this->_pEngine->context().device().waitForFences(*fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
		VK_CHECK(waitResult);
	}
}

void TSDFVolume::_createDescriptorSet(void) {
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(*this->_pEngine->descriptorPool())
		.setSetLayouts(this->_descriptorSetLayout);
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