#include "DescriptorSet.hpp"
#include "Engine.hpp"
#include "KinectFusion.hpp"

#define VK_THROW(err) \
	throw std::runtime_error("[DescriptorSet] Vulkan error in file " + std::string(__FILE__) + " line " + std::to_string(__LINE__) + ": " + vk::to_string(err))

#define VK_CHECK(value) \
	if (vk::Result err = (value); err != vk::Result::eSuccess) { VK_THROW(err); }

ViewLevelDescriptorSet::ViewLevelDescriptorSet(
	const Engine & engine_
) :
	_pEngine(&engine_), _descriptorSetLayout(*engine_.viewLevelDescriptorSetLayout())
{
	// Create descriptor set
	{
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(*this->_pEngine->descriptorPool())
			.setDescriptorSetCount(1)
			.setSetLayouts(this->_descriptorSetLayout);
		this->_descriptorSet = std::move(this->_pEngine->context().device().allocateDescriptorSets(descriptorSetAllocateInfo)[0]);
	}
	// Create uniform buffer for binding 0
	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setFlags(vk::BufferCreateFlags(0))
			.setSize(sizeof(ViewLevelDescriptorSet::CameraParameters))
			.setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
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
		VkBuffer uniformBuffer = nullptr;
		VmaAllocation uniformBufferMemory = nullptr;
		VmaAllocationInfo allocationInfo{};
		vmaCreateBuffer(*this->_pEngine->allocator(), reinterpret_cast<VkBufferCreateInfo*>(&bufferCreateInfo), &vmaAllocationCreateInfo, &uniformBuffer, &uniformBufferMemory, &allocationInfo);
		this->_cameraParametersBuffer = vk::raii::Buffer(this->_pEngine->context().device(), uniformBuffer);
		this->_cameraParametersBufferMemory = jjyou::vk::VmaAllocation(this->_pEngine->allocator(), uniformBufferMemory);
		this->_cameraParametersBufferMemoryMappedAddress = allocationInfo.pMappedData;
	}
	// Update the descriptor set
	{
		vk::DescriptorBufferInfo descriptorBufferInfo = vk::DescriptorBufferInfo()
			.setBuffer(*this->_cameraParametersBuffer)
			.setOffset(0)
			.setRange(sizeof(ViewLevelDescriptorSet::CameraParameters));
		vk::WriteDescriptorSet writeDescriptorSet = vk::WriteDescriptorSet()
			.setDstSet(*this->_descriptorSet)
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setBufferInfo(descriptorBufferInfo);
		this->_pEngine->context().device().updateDescriptorSets(writeDescriptorSet, nullptr);
	}
}

InstanceLevelDescriptorSet::InstanceLevelDescriptorSet(
	const Engine& engine_,
	std::uint32_t numModelTransforms_
) :
	_pEngine(&engine_), _descriptorSetLayout(*engine_.instanceLevelDescriptorSetLayout()), _numModelTransforms(numModelTransforms_)
{
	// Create descriptor set
	{
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(*this->_pEngine->descriptorPool())
			.setDescriptorSetCount(1)
			.setSetLayouts(this->_descriptorSetLayout);
		this->_descriptorSet = std::move(this->_pEngine->context().device().allocateDescriptorSets(descriptorSetAllocateInfo)[0]);
	}
	// Create uniform buffer for binding 0
	{
		vk::DeviceSize minAlignment = this->_pEngine->context().physicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
		this->_modelTransformsBufferOffset = sizeof(InstanceLevelDescriptorSet::ModelTransforms);
		if (minAlignment > 0)
			this->_modelTransformsBufferOffset = (this->_modelTransformsBufferOffset + minAlignment - 1) & ~(minAlignment - 1);
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setFlags(vk::BufferCreateFlags(0))
			.setSize(this->_modelTransformsBufferOffset * static_cast<vk::DeviceSize>(this->_numModelTransforms))
			.setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
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
		VkBuffer uniformBuffer = nullptr;
		VmaAllocation uniformBufferMemory = nullptr;
		VmaAllocationInfo allocationInfo{};
		vmaCreateBuffer(*this->_pEngine->allocator(), reinterpret_cast<VkBufferCreateInfo*>(&bufferCreateInfo), &vmaAllocationCreateInfo, &uniformBuffer, &uniformBufferMemory, &allocationInfo);
		this->_modelTransformsBuffer = vk::raii::Buffer(this->_pEngine->context().device(), uniformBuffer);
		this->_modelTransformsBufferMemory = jjyou::vk::VmaAllocation(this->_pEngine->allocator(), uniformBufferMemory);
		this->_modelTransformsBufferMemoryMappedAddress = allocationInfo.pMappedData;
	}
	// Update the descriptor set
	{
		vk::DescriptorBufferInfo descriptorBufferInfo = vk::DescriptorBufferInfo()
			.setBuffer(*this->_modelTransformsBuffer)
			.setOffset(0)
			.setRange(sizeof(InstanceLevelDescriptorSet::ModelTransforms));
		vk::WriteDescriptorSet writeDescriptorSet = vk::WriteDescriptorSet()
			.setDstSet(*this->_descriptorSet)
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
			.setBufferInfo(descriptorBufferInfo);
		this->_pEngine->context().device().updateDescriptorSets(writeDescriptorSet, nullptr);
	}
}

RayCastingDescriptorSet::RayCastingDescriptorSet(
	const Engine& engine_,
	const KinectFusion& kinectFusion_
) :
	_pEngine(&engine_), _pKinectFusion(&kinectFusion_), _descriptorSetLayout(*kinectFusion_.rayCastingDescriptorSetLayout())
{
	// Create descriptor set
	{
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(*this->_pEngine->descriptorPool())
			.setDescriptorSetCount(1)
			.setSetLayouts(this->_descriptorSetLayout);
		this->_descriptorSet = std::move(this->_pEngine->context().device().allocateDescriptorSets(descriptorSetAllocateInfo)[0]);
	}
	// Create uniform buffer for binding 0
	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setFlags(vk::BufferCreateFlags(0))
			.setSize(sizeof(RayCastingDescriptorSet::RayCastingParameters))
			.setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
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
		VkBuffer uniformBuffer = nullptr;
		VmaAllocation uniformBufferMemory = nullptr;
		VmaAllocationInfo allocationInfo{};
		vmaCreateBuffer(*this->_pEngine->allocator(), reinterpret_cast<VkBufferCreateInfo*>(&bufferCreateInfo), &vmaAllocationCreateInfo, &uniformBuffer, &uniformBufferMemory, &allocationInfo);
		this->_rayCastingParametersBuffer = vk::raii::Buffer(this->_pEngine->context().device(), uniformBuffer);
		this->_rayCastingParametersBufferMemory = jjyou::vk::VmaAllocation(this->_pEngine->allocator(), uniformBufferMemory);
		this->_rayCastingParametersBufferMemoryMappedAddress = allocationInfo.pMappedData;
	}
	// Update the descriptor set
	{
		vk::DescriptorBufferInfo descriptorBufferInfo = vk::DescriptorBufferInfo()
			.setBuffer(*this->_rayCastingParametersBuffer)
			.setOffset(0)
			.setRange(sizeof(RayCastingDescriptorSet::RayCastingParameters));
		vk::WriteDescriptorSet writeDescriptorSet = vk::WriteDescriptorSet()
			.setDstSet(*this->_descriptorSet)
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setBufferInfo(descriptorBufferInfo);
		this->_pEngine->context().device().updateDescriptorSets(writeDescriptorSet, nullptr);
	}
}

FusionDescriptorSet::FusionDescriptorSet(
	const Engine& engine_,
	const KinectFusion& kinectFusion_
) :
	_pEngine(&engine_), _pKinectFusion(&kinectFusion_), _descriptorSetLayout(*kinectFusion_.fusionDescriptorSetLayout())
{
	// Create descriptor set
	{
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(*this->_pEngine->descriptorPool())
			.setDescriptorSetCount(1)
			.setSetLayouts(this->_descriptorSetLayout);
		this->_descriptorSet = std::move(this->_pEngine->context().device().allocateDescriptorSets(descriptorSetAllocateInfo)[0]);
	}
	// Create uniform buffer for binding 0
	{
		vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
			.setFlags(vk::BufferCreateFlags(0))
			.setSize(sizeof(FusionDescriptorSet::FusionParameters))
			.setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
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
		VkBuffer uniformBuffer = nullptr;
		VmaAllocation uniformBufferMemory = nullptr;
		VmaAllocationInfo allocationInfo{};
		vmaCreateBuffer(*this->_pEngine->allocator(), reinterpret_cast<VkBufferCreateInfo*>(&bufferCreateInfo), &vmaAllocationCreateInfo, &uniformBuffer, &uniformBufferMemory, &allocationInfo);
		this->_fusionParametersBuffer = vk::raii::Buffer(this->_pEngine->context().device(), uniformBuffer);
		this->_fusionParametersBufferMemory = jjyou::vk::VmaAllocation(this->_pEngine->allocator(), uniformBufferMemory);
		this->_fusionParametersBufferMemoryMappedAddress = allocationInfo.pMappedData;
	}
	// Update the descriptor set
	{
		vk::DescriptorBufferInfo descriptorBufferInfo = vk::DescriptorBufferInfo()
			.setBuffer(*this->_fusionParametersBuffer)
			.setOffset(0)
			.setRange(sizeof(FusionDescriptorSet::FusionParameters));
		vk::WriteDescriptorSet writeDescriptorSet = vk::WriteDescriptorSet()
			.setDstSet(*this->_descriptorSet)
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setBufferInfo(descriptorBufferInfo);
		this->_pEngine->context().device().updateDescriptorSets(writeDescriptorSet, nullptr);
	}
}

ICPDescriptorSet::ICPDescriptorSet(
	const Engine& engine_,
	const KinectFusion& kinectFusion_
) :
	_pEngine(&engine_), _pKinectFusion(&kinectFusion_), _descriptorSetLayout(*kinectFusion_.fusionDescriptorSetLayout())
{

}