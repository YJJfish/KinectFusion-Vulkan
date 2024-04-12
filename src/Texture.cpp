#include "Texture.hpp"
#include "Engine.hpp"
#include <stdexcept>

#define VK_THROW(err) \
	throw std::runtime_error("[Texture] Vulkan error in file " + std::string(__FILE__) + " line " + std::to_string(__LINE__) + ": " + vk::to_string(err))

#define VK_CHECK(value) \
	if (vk::Result err = (value); err != vk::Result::eSuccess) { VK_THROW(err); }

Texture2D::Texture2D(
	const Engine& engine_,
	vk::Format format_,
	vk::Extent2D extent_,
	vk::ImageUsageFlags usage_,
	const std::set<std::uint32_t>& queueFamilyIndices_
) : _pEngine(&engine_), _format(format_), _extent(extent_) {
	std::vector<std::uint32_t> queueFamilyIndices(queueFamilyIndices_.begin(), queueFamilyIndices_.end());
	vk::ImageCreateInfo imageCreateInfo = vk::ImageCreateInfo()
		.setFlags(vk::ImageCreateFlags(0))
		.setImageType(vk::ImageType::e2D)
		.setFormat(this->_format)
		.setExtent(vk::Extent3D(this->_extent, 1))
		.setMipLevels(1)
		.setArrayLayers(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(usage_)
		.setSharingMode(queueFamilyIndices.size() >= 2 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive)
		.setQueueFamilyIndices(queueFamilyIndices)
		.setInitialLayout(vk::ImageLayout::eUndefined);
	VmaAllocationCreateInfo vmaAllocationCreateInfo{
		.flags = VmaAllocationCreateFlags(0),
		.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		.requiredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.preferredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.memoryTypeBits = 0,
		.pool = nullptr,
		.pUserData = nullptr,
		.priority = 0.0f,
	};
	VkImage image = nullptr;
	VmaAllocation imageMemory = nullptr;
	vmaCreateImage(*this->_pEngine->allocator(), reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo), &vmaAllocationCreateInfo, &image, &imageMemory, nullptr);
	this->_image = vk::raii::Image(this->_pEngine->context().device(), image);
	this->_imageMemory = jjyou::vk::VmaAllocation(this->_pEngine->allocator(), imageMemory);
	vk::ImageAspectFlags aspectMask{ 0 };
	if (this->_format >= vk::Format::eD16Unorm)
		aspectMask = vk::ImageAspectFlagBits::eDepth;
	else
		aspectMask = vk::ImageAspectFlagBits::eColor;
	vk::ImageViewCreateInfo imageViewCreateInfo = vk::ImageViewCreateInfo()
		.setFlags(vk::ImageViewCreateFlags(0))
		.setImage(*this->_image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(this->_format)
		.setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity))
		.setSubresourceRange(vk::ImageSubresourceRange(aspectMask, 0, 1, 0, 1));
	this->_imageView = vk::raii::ImageView(this->_pEngine->context().device(), imageViewCreateInfo);
}

template <MaterialType _materialType>
Surface<_materialType>::Surface(const Engine& engine_) :
	_pEngine(&engine_),
	_samplerDescriptorSetLayout(*engine_.surfaceSamplerDescriptorSetLayout(_materialType)),
	_storageDescriptorSetLayout(*engine_.surfaceStorageDescriptorSetLayout(_materialType))
{
	// Create empty textures
	{
		this->_textures.reserve(Surface::numTextures);
		for (std::uint32_t i = 0; i < Surface::numTextures; ++i) {
			this->_textures.emplace_back(nullptr);
		}
	}
	// Create sampler
	{
		vk::SamplerCreateInfo samplerCreateInfo = vk::SamplerCreateInfo()
			.setFlags(vk::SamplerCreateFlags(0))
			.setMagFilter(vk::Filter::eNearest)
			.setMinFilter(vk::Filter::eNearest)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
			.setMipLodBias(0.0f)
			.setAnisotropyEnable(VK_FALSE) // Ray casting maps won't be viewed at oblique angles
			.setMaxAnisotropy(0.0f)
			.setCompareEnable(VK_FALSE)
			.setCompareOp(vk::CompareOp::eNever)
			.setMinLod(0.0f)
			.setMaxLod(0.0f)
			.setBorderColor(vk::BorderColor::eFloatOpaqueBlack)
			.setUnnormalizedCoordinates(VK_FALSE);
		this->_sampler = vk::raii::Sampler(this->_pEngine->context().device(), samplerCreateInfo);
	}
	// Create descriptor set
	{
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(*this->_pEngine->descriptorPool())
			.setDescriptorSetCount(1)
			.setSetLayouts(this->_samplerDescriptorSetLayout);
		this->_samplerDescriptorSet = std::move(this->_pEngine->context().device().allocateDescriptorSets(descriptorSetAllocateInfo)[0]);
	}
	{
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(*this->_pEngine->descriptorPool())
			.setDescriptorSetCount(1)
			.setSetLayouts(this->_storageDescriptorSetLayout);
		this->_storageDescriptorSet = std::move(this->_pEngine->context().device().allocateDescriptorSets(descriptorSetAllocateInfo)[0]);
	}
}

template <MaterialType _materialType>
Surface<_materialType>& Surface<_materialType>::createTextures(
		std::array<vk::Extent2D, Surface::numTextures> extents_,
		std::optional<std::array<const void*, Surface::numTextures>> data_
) {
	// Wait graphics and compute queues to be idle.
	this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Main)->waitIdle();
	this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Compute)->waitIdle();
	constexpr std::array<vk::Format, 3> formats = { {
		vk::Format::eR8G8B8A8Unorm,
		vk::Format::eR32Sfloat,
		vk::Format::eR32G32B32A32Sfloat
	} };
	constexpr std::array<vk::DeviceSize, 3> elementSizes = { {
		4,
		4,
		16
	} };
	bool recreate = false;
	for (std::uint32_t i = 0; i < Surface::numTextures; ++i) {
		if (this->_textures[i].extent() != extents_[i]) {
			recreate = true;
			this->_textures[i].~Texture2D();
			this->_textures[i] = Texture2D(
				*this->_pEngine,
				formats[i],
				extents_[i],
				vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst,
				{
					*this->_pEngine->context().queueFamilyIndex(jjyou::vk::Context::QueueType::Main),
					*this->_pEngine->context().queueFamilyIndex(jjyou::vk::Context::QueueType::Compute),
					*this->_pEngine->context().queueFamilyIndex(jjyou::vk::Context::QueueType::Transfer)
				}
			);
		}
	}
	if (recreate) {
		// Update sampler descriptor set
		{
			std::array<vk::DescriptorImageInfo, Surface::numTextures> descriptorImageInfos{};
			std::array<vk::WriteDescriptorSet, Surface::numTextures> writeDescriptorSets{};
			for (std::uint32_t i = 0; i < Surface::numTextures; ++i) {
				descriptorImageInfos[i]
					.setSampler(*this->_sampler)
					.setImageView(*this->_textures[i].imageView())
					.setImageLayout(vk::ImageLayout::eGeneral);
				writeDescriptorSets[i]
					.setDstSet(*this->_samplerDescriptorSet)
					.setDstBinding(i)
					.setDstArrayElement(0)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
					.setImageInfo(descriptorImageInfos[i]);
			}
			this->_pEngine->context().device().updateDescriptorSets(writeDescriptorSets, nullptr);
		}
		// Update storage descriptor set
		{
			std::array<vk::DescriptorImageInfo, Surface::numTextures> descriptorImageInfos{};
			std::array<vk::WriteDescriptorSet, Surface::numTextures> writeDescriptorSets{};
			for (std::uint32_t i = 0; i < Surface::numTextures; ++i) {
				descriptorImageInfos[i]
					.setSampler(nullptr)
					.setImageView(*this->_textures[i].imageView())
					.setImageLayout(vk::ImageLayout::eGeneral);
				writeDescriptorSets[i]
					.setDstSet(*this->_storageDescriptorSet)
					.setDstBinding(i)
					.setDstArrayElement(0)
					.setDescriptorCount(1)
					.setDescriptorType(vk::DescriptorType::eStorageImage)
					.setImageInfo(descriptorImageInfos[i]);
			}
			this->_pEngine->context().device().updateDescriptorSets(writeDescriptorSets, nullptr);
		}
	}
	// Transfer data or transition texture layouts
	if (recreate || data_ != std::nullopt) {
		// Create a transfer command buffer and a fence
		vk::raii::CommandBuffer transferCommandBuffer = std::move(this->_pEngine->context().device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(*this->_pEngine->commandPool(jjyou::vk::Context::QueueType::Transfer))
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
		)[0]);
		vk::raii::Fence fence = vk::raii::Fence(this->_pEngine->context().device(), vk::FenceCreateInfo(vk::FenceCreateFlags(0)));
		transferCommandBuffer.begin(vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
			.setPInheritanceInfo(nullptr)
		);
		// Tranition texture layouts
		if (recreate) {
			vk::ImageMemoryBarrier imageMemoryBarrier = vk::ImageMemoryBarrier()
				.setSrcAccessMask(vk::AccessFlags(0))
				.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
				.setOldLayout(vk::ImageLayout::eUndefined)
				.setNewLayout(vk::ImageLayout::eGeneral)
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				//.setImage()
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
			for (std::uint32_t i = 0; i < Surface::numTextures; ++i) {
				imageMemoryBarrier.setImage(*this->_textures[i].image());
				transferCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(0), nullptr, nullptr, imageMemoryBarrier);
			}
		}
		// Create staging buffer and copy CPU data to it.
		std::vector<vk::raii::Buffer> stagingBuffers{};
		std::vector<jjyou::vk::VmaAllocation> stagingBufferMemorys{};
		if (data_ != std::nullopt) {
			for (std::uint32_t i = 0; i < Surface::numTextures; ++i) {
				vk::DeviceSize bufferSize = elementSizes[i] * static_cast<vk::DeviceSize>(extents_[i].width) * static_cast<vk::DeviceSize>(extents_[i].height);
				vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
					.setFlags(vk::BufferCreateFlags(0))
					.setSize(bufferSize)
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
				stagingBuffers.emplace_back(this->_pEngine->context().device(), pStagingBuffer);
				stagingBufferMemorys.emplace_back(this->_pEngine->allocator(), pStagingBufferMemory);
				memcpy(allocationInfo.pMappedData, (*data_)[i], bufferSize);
				vk::BufferImageCopy bufferImageCopy = vk::BufferImageCopy()
					.setBufferOffset(0)
					.setBufferRowLength(0)
					.setBufferImageHeight(0)
					.setImageSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
					.setImageOffset(vk::Offset3D(0, 0, 0))
					.setImageExtent(vk::Extent3D(this->_textures[i].extent(), 1));
				transferCommandBuffer.copyBufferToImage(*stagingBuffers[i], *this->_textures[i].image(), vk::ImageLayout::eGeneral, bufferImageCopy);
			}
		}
		// Transfer command buffer submits (signal fence)
		{
			transferCommandBuffer.end();
			this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Transfer)->submit(
				vk::SubmitInfo()
				.setWaitSemaphores(nullptr)
				.setWaitDstStageMask(nullptr)
				.setCommandBuffers(*transferCommandBuffer)
				.setSignalSemaphores(nullptr),
				*fence
			);
		}
		// CPU waits the fence
		{
			vk::Result waitResult = this->_pEngine->context().device().waitForFences(*fence, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
			VK_CHECK(waitResult);
		}
	}
	return *this;
}

template Surface<MaterialType::Simple>;
template Surface<MaterialType::Lambertian>;