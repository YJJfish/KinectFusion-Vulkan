#include "Primitives.hpp"
#include "Engine.hpp"

#define VK_THROW(err) \
	throw std::runtime_error("[Primitives] Vulkan error in file " + std::string(__FILE__) + " line " + std::to_string(__LINE__) + ": " + vk::to_string(err))

#define VK_CHECK(value) \
	if (vk::Result err = (value); err != vk::Result::eSuccess) { VK_THROW(err); }

template <MaterialType _materialType, PrimitiveType _primitiveType>
Primitives<_materialType, _primitiveType>& Primitives<_materialType, _primitiveType>::setVertexData(
	const Vertex<_materialType>* data_,
	std::uint32_t numVertices_,
	bool waitIdle_
) {
	// Wait graphics queue to be idle.
	if (waitIdle_) {
		this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Main)->waitIdle();
	}
	vk::DeviceSize bufferSize = sizeof(Vertex<_materialType>) * numVertices_;
	if (this->_memoryPattern == MemoryPattern::Static) {
		// 0. Create one transfer command buffer, two graphics command buffer, two semaphores and three fences.
		// 1. Graphics command buffer 0 releases ownership
		// 2. Graphics command buffer 0 submits (signal semaphore 0, signal fence 0)
		// 3. Transfer command buffer acquires ownership
		// 4. Transfer command buffer copies staging buffer to final vertex buffer
		// 5. Transfer command buffer releases ownership
		// 6. Transfer command buffer submits (wait semaphore 0, signal semaphore 1, signal fence 1)
		// 7. Graphics command buffer 1 acquires ownership
		// 8. Graphics command buffer 1 submits (wait semaphore 1, signal fence 2)
		// 9. CPU waits fences
		vk::raii::CommandBuffer transferCommandBuffer = std::move(this->_pEngine->context().device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(*this->_pEngine->commandPool(jjyou::vk::Context::QueueType::Transfer))
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1)
		)[0]);
		std::vector<vk::raii::CommandBuffer> graphicsCommandBuffers = this->_pEngine->context().device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo()
			.setCommandPool(*this->_pEngine->commandPool(jjyou::vk::Context::QueueType::Main))
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(2)
		);
		std::array<vk::raii::Semaphore, 2> semaphores = { {
			vk::raii::Semaphore(this->_pEngine->context().device(), vk::SemaphoreCreateInfo().setFlags(vk::SemaphoreCreateFlags(0))),
			vk::raii::Semaphore(this->_pEngine->context().device(), vk::SemaphoreCreateInfo().setFlags(vk::SemaphoreCreateFlags(0)))
		} };
		std::array<vk::raii::Fence, 3> fences = { {
			vk::raii::Fence(this->_pEngine->context().device(), vk::FenceCreateInfo(vk::FenceCreateFlags(0))),
			vk::raii::Fence(this->_pEngine->context().device(), vk::FenceCreateInfo(vk::FenceCreateFlags(0))),
			vk::raii::Fence(this->_pEngine->context().device(), vk::FenceCreateInfo(vk::FenceCreateFlags(0)))
		} };
		transferCommandBuffer.begin(vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
			.setPInheritanceInfo(nullptr)
		);
		graphicsCommandBuffers[0].begin(vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
			.setPInheritanceInfo(nullptr)
		);
		graphicsCommandBuffers[1].begin(vk::CommandBufferBeginInfo()
			.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
			.setPInheritanceInfo(nullptr)
		);
		// If the new size does not equal to the old size, create a new vertex buffer.
		if (this->_numVertices != numVertices_) {
			this->_numVertices = numVertices_;
			this->_vertexBufferMemory.clear();
			this->_vertexBuffer.clear();
			vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
				.setFlags(vk::BufferCreateFlags(0))
				.setSize(bufferSize)
				.setUsage(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst)
				.setSharingMode(vk::SharingMode::eExclusive)
				.setQueueFamilyIndices(nullptr);
			VmaAllocationCreateInfo vmaAllocationCreateInfo{
				.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
				.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO,
				.requiredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				.preferredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				.memoryTypeBits = 0,
				.pool = nullptr,
				.pUserData = nullptr,
				.priority = 0.0f,
			};
			VkBuffer vertexBuffer = nullptr;
			VmaAllocation vertexBufferMemory = nullptr;
			vmaCreateBuffer(*this->_pEngine->allocator(), reinterpret_cast<VkBufferCreateInfo*>(&bufferCreateInfo), &vmaAllocationCreateInfo, &vertexBuffer, &vertexBufferMemory, nullptr);
			this->_vertexBuffer = vk::raii::Buffer(this->_pEngine->context().device(), vertexBuffer);
			this->_vertexBufferMemory = jjyou::vk::VmaAllocation(this->_pEngine->allocator(), vertexBufferMemory);
		}
		// 1. Graphics command buffer 0 releases ownership
		{
			vk::BufferMemoryBarrier bufferMemoryBarrier = vk::BufferMemoryBarrier()
				.setSrcAccessMask(vk::AccessFlags(0))
				.setDstAccessMask(vk::AccessFlags(0))
				.setSrcQueueFamilyIndex(*this->_pEngine->context().queueFamilyIndex(jjyou::vk::Context::QueueType::Main))
				.setDstQueueFamilyIndex(*this->_pEngine->context().queueFamilyIndex(jjyou::vk::Context::QueueType::Transfer))
				.setBuffer(*this->_vertexBuffer)
				.setOffset(0)
				.setSize(bufferSize);
			graphicsCommandBuffers[0].pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags(0), nullptr, bufferMemoryBarrier, nullptr);
		}
		// 2. Graphics command buffer 0 submits (signal semaphore 0, signal fence 0)
		{
			graphicsCommandBuffers[0].end();
			this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Main)->submit(
				vk::SubmitInfo()
				.setWaitSemaphores(nullptr)
				.setWaitDstStageMask(nullptr)
				.setCommandBuffers(*graphicsCommandBuffers[0])
				.setSignalSemaphores(*semaphores[0]),
				*fences[0]
			);
		}
		// 3. Transfer command buffer acquires ownership
		{
			vk::BufferMemoryBarrier bufferMemoryBarrier = vk::BufferMemoryBarrier()
				.setSrcAccessMask(vk::AccessFlags(0))
				.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
				.setSrcQueueFamilyIndex(*this->_pEngine->context().queueFamilyIndex(jjyou::vk::Context::QueueType::Main))
				.setDstQueueFamilyIndex(*this->_pEngine->context().queueFamilyIndex(jjyou::vk::Context::QueueType::Transfer))
				.setBuffer(*this->_vertexBuffer)
				.setOffset(0)
				.setSize(bufferSize);
			transferCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(0), nullptr, bufferMemoryBarrier, nullptr);
		}
		// Create staging buffer and copy CPU data to it.
		vk::raii::Buffer stagingBuffer{ nullptr };
		jjyou::vk::VmaAllocation stagingBufferMemory{ nullptr };
		{
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
			stagingBuffer = vk::raii::Buffer(this->_pEngine->context().device(), pStagingBuffer);
			stagingBufferMemory = jjyou::vk::VmaAllocation(this->_pEngine->allocator(), pStagingBufferMemory);
			memcpy(allocationInfo.pMappedData, data_, bufferSize);
		}
		// 4. Transfer command buffer copies staging buffer to final vertex buffer
		{
			transferCommandBuffer.copyBuffer(*stagingBuffer, *this->_vertexBuffer, vk::BufferCopy(0, 0, bufferSize));
		}
		// 5. Transfer command buffer releases ownership
		{
			vk::BufferMemoryBarrier bufferMemoryBarrier = vk::BufferMemoryBarrier()
				.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
				.setDstAccessMask(vk::AccessFlags(0))
				.setSrcQueueFamilyIndex(*this->_pEngine->context().queueFamilyIndex(jjyou::vk::Context::QueueType::Transfer))
				.setDstQueueFamilyIndex(*this->_pEngine->context().queueFamilyIndex(jjyou::vk::Context::QueueType::Main))
				.setBuffer(*this->_vertexBuffer)
				.setOffset(0)
				.setSize(bufferSize);
			transferCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags(0), nullptr, bufferMemoryBarrier, nullptr);
		}
		// 6. Transfer command buffer submits (wait semaphore 0, signal semaphore 1, signal fence 1)
		{
			transferCommandBuffer.end();
			this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Transfer)->submit(
				vk::SubmitInfo()
				.setWaitSemaphores(*semaphores[0])
				.setWaitDstStageMask(nullptr)
				.setCommandBuffers(*transferCommandBuffer)
				.setSignalSemaphores(*semaphores[1]),
				*fences[1]
			);
		}
		// 7. Graphics command buffer 1 acquires ownership
		{
			vk::BufferMemoryBarrier bufferMemoryBarrier = vk::BufferMemoryBarrier()
				.setSrcAccessMask(vk::AccessFlags(0))
				.setDstAccessMask(vk::AccessFlags(0))
				.setSrcQueueFamilyIndex(*this->_pEngine->context().queueFamilyIndex(jjyou::vk::Context::QueueType::Transfer))
				.setDstQueueFamilyIndex(*this->_pEngine->context().queueFamilyIndex(jjyou::vk::Context::QueueType::Main))
				.setBuffer(*this->_vertexBuffer)
				.setOffset(0)
				.setSize(bufferSize);
			graphicsCommandBuffers[1].pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags(0), nullptr, bufferMemoryBarrier, nullptr);
		}
		// 8. Graphics command buffer 1 submits (wait semaphore 1, signal fence 2)
		{
			graphicsCommandBuffers[1].end();
			this->_pEngine->context().queue(jjyou::vk::Context::QueueType::Main)->submit(
				vk::SubmitInfo()
				.setWaitSemaphores(*semaphores[1])
				.setWaitDstStageMask(nullptr)
				.setCommandBuffers(*graphicsCommandBuffers[1])
				.setSignalSemaphores(nullptr),
				*fences[2]
			);
		}
		// 9. CPU waits fences
		{
			vk::Result waitResult = this->_pEngine->context().device().waitForFences({ *fences[0], *fences[1], *fences[2] }, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
			VK_CHECK(waitResult);
		}
	}
	else if (this->_memoryPattern == MemoryPattern::Dynamic) {
		// If the new size does not equal to the old size, create a new vertex buffer.
		if (this->_numVertices != numVertices_) {
			this->_numVertices = numVertices_;
			this->_vertexBufferMemory.clear();
			this->_vertexBufferMemoryMappedAddress = nullptr;
			this->_vertexBuffer.clear();
			vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
				.setFlags(vk::BufferCreateFlags(0))
				.setSize(bufferSize)
				.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
				.setSharingMode(vk::SharingMode::eExclusive)
				.setQueueFamilyIndices(nullptr);
			VmaAllocationCreateInfo vmaAllocationCreateInfo{
				.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT,
				.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO,
				.requiredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				.preferredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				.memoryTypeBits = 0,
				.pool = nullptr,
				.pUserData = nullptr,
				.priority = 0.0f,
			};
			VkBuffer vertexBuffer = nullptr;
			VmaAllocation vertexBufferMemory = nullptr;
			VmaAllocationInfo allocationInfo{};
			vmaCreateBuffer(*this->_pEngine->allocator(), reinterpret_cast<VkBufferCreateInfo*>(&bufferCreateInfo), &vmaAllocationCreateInfo, &vertexBuffer, &vertexBufferMemory, &allocationInfo);
			this->_vertexBuffer = vk::raii::Buffer(this->_pEngine->context().device(), vertexBuffer);
			this->_vertexBufferMemory = jjyou::vk::VmaAllocation(this->_pEngine->allocator(), vertexBufferMemory);
			this->_vertexBufferMemoryMappedAddress = allocationInfo.pMappedData;
		}
		// Copy data
		memcpy(this->_vertexBufferMemoryMappedAddress, data_, bufferSize);
	}
	return *this;
}

template class Primitives<MaterialType::Simple, PrimitiveType::Point>;
template class Primitives<MaterialType::Simple, PrimitiveType::Line>;
template class Primitives<MaterialType::Simple, PrimitiveType::Triangle>;
template class Primitives<MaterialType::Lambertian, PrimitiveType::Point>;
template class Primitives<MaterialType::Lambertian, PrimitiveType::Line>;
template class Primitives<MaterialType::Lambertian, PrimitiveType::Triangle>;
