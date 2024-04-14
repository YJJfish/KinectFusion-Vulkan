/***********************************************************************
 * @file	Memory.hpp
 * @author	jjyou
 * @date	2024-2-6
 * @brief	This file implements Texture2D class.
***********************************************************************/

#include "Memory.hpp"
#include "utils.hpp"

namespace jjyou {

	namespace vk {

		class Texture2D {

		public:

			/** @brief	Default constructor.
			  */
			Texture2D(void) {}

			/** @brief	Destructor.
			  */
			~Texture2D(void) {
				this->destroy();
			}

			Texture2D(const Texture2D&) = delete;

			Texture2D(Texture2D&& other) noexcept :
				_pContext(other._pContext),
				_extent(other._extent),
				_numLayers(other._numLayers),
				_mipLevels(other._mipLevels),
				_format(other._format),
				_pAllocator(other._pAllocator),
				_image(other._image),
				_imageMemory(std::move(other._imageMemory)),
				_imageView(other._imageView),
				_sampler(other._sampler)
			{
				other._pContext = nullptr;
			}

			/** @brief	Check whether the texture is in a valid state
			  */
			bool has_value(void) const { return (this->_pContext != nullptr); }

			/** @brief	Create a texture.
			  */
			void create(
				const Context& context,
				MemoryAllocator& allocator,
				VkCommandPool graphicsCommandPool,
				VkCommandPool transferCommandPool,
				const void* data,
				VkFormat format,
				VkExtent2D extent,
				int mipLevels = 1,
				const std::vector<void*>& mipData = {},
				bool cubeMap = false,
				VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT
			){
				this->_pContext = &context;
				this->_pAllocator = &allocator;
				this->_extent = extent;
				this->_numLayers = (cubeMap ? 6 : 1);
				this->_mipLevels = mipLevels;
				this->_format = format;
				if (mipData.size() != this->_mipLevels - 1U)
					JJYOU_VK_UTILS_THROW(VK_ERROR_FORMAT_NOT_SUPPORTED);
				VkDeviceSize elementSize = 0;
				switch (this->_format) {
				case VK_FORMAT_R8_UNORM:
				case VK_FORMAT_R8_SNORM:
				case VK_FORMAT_R8_USCALED:
				case VK_FORMAT_R8_SSCALED:
				case VK_FORMAT_R8_UINT:
				case VK_FORMAT_R8_SINT:
				case VK_FORMAT_R8_SRGB:
					elementSize = 1;
					break;
				case VK_FORMAT_R8G8B8_UNORM:
				case VK_FORMAT_R8G8B8_SNORM:
				case VK_FORMAT_R8G8B8_USCALED:
				case VK_FORMAT_R8G8B8_SSCALED:
				case VK_FORMAT_R8G8B8_UINT:
				case VK_FORMAT_R8G8B8_SINT:
				case VK_FORMAT_R8G8B8_SRGB:
				case VK_FORMAT_B8G8R8_UNORM:
				case VK_FORMAT_B8G8R8_SNORM:
				case VK_FORMAT_B8G8R8_USCALED:
				case VK_FORMAT_B8G8R8_SSCALED:
				case VK_FORMAT_B8G8R8_UINT:
				case VK_FORMAT_B8G8R8_SINT:
				case VK_FORMAT_B8G8R8_SRGB:
					elementSize = 3;
					break;
				case VK_FORMAT_R8G8B8A8_UNORM:
				case VK_FORMAT_R8G8B8A8_SNORM:
				case VK_FORMAT_R8G8B8A8_USCALED:
				case VK_FORMAT_R8G8B8A8_SSCALED:
				case VK_FORMAT_R8G8B8A8_UINT:
				case VK_FORMAT_R8G8B8A8_SINT:
				case VK_FORMAT_R8G8B8A8_SRGB:
				case VK_FORMAT_B8G8R8A8_UNORM:
				case VK_FORMAT_B8G8R8A8_SNORM:
				case VK_FORMAT_B8G8R8A8_USCALED:
				case VK_FORMAT_B8G8R8A8_SSCALED:
				case VK_FORMAT_B8G8R8A8_UINT:
				case VK_FORMAT_B8G8R8A8_SINT:
				case VK_FORMAT_B8G8R8A8_SRGB:
					elementSize = 4;
					break;
				case VK_FORMAT_R32_UINT:
				case VK_FORMAT_R32_SINT:
				case VK_FORMAT_R32_SFLOAT:
					elementSize = 4;
					break;
				case VK_FORMAT_R32G32_UINT:
				case VK_FORMAT_R32G32_SINT:
				case VK_FORMAT_R32G32_SFLOAT:
					elementSize = 8;
					break;
				case VK_FORMAT_R32G32B32_UINT:
				case VK_FORMAT_R32G32B32_SINT:
				case VK_FORMAT_R32G32B32_SFLOAT:
					elementSize = 12;
					break;
				case VK_FORMAT_R32G32B32A32_UINT:
				case VK_FORMAT_R32G32B32A32_SINT:
				case VK_FORMAT_R32G32B32A32_SFLOAT:
					elementSize = 16;
					break;
				default:
					JJYOU_VK_UTILS_THROW(VK_ERROR_FORMAT_NOT_SUPPORTED);
				}
				std::uint32_t transferQueueFamily = *this->_pContext->queueFamilyIndex(jjyou::vk::Context::QueueType::Transfer);
				std::uint32_t graphicsQueueFamily = *this->_pContext->queueFamilyIndex(jjyou::vk::Context::QueueType::Main);
				// Create the image
				VkImageCreateInfo imageInfo{
					.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
					.pNext = nullptr,
					.flags = (cubeMap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : VkImageCreateFlags(0)),
					.imageType = VK_IMAGE_TYPE_2D,
					.format = this->_format,
					.extent{
						.width = this->_extent.width,
						.height = this->_extent.height,
						.depth = 1
					},
					.mipLevels = this->_mipLevels,
					.arrayLayers = this->_numLayers,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.tiling = VK_IMAGE_TILING_OPTIMAL,
					.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
					.queueFamilyIndexCount = 1U,
					.pQueueFamilyIndices = &transferQueueFamily,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
				};
				JJYOU_VK_UTILS_CHECK(vkCreateImage(*this->_pContext->device(), &imageInfo, nullptr, &this->_image));
				VkMemoryRequirements imageMemoryRequirements;
				vkGetImageMemoryRequirements(*this->_pContext->device(), this->_image, &imageMemoryRequirements);
				VkMemoryAllocateInfo imageMemoryAllocInfo{
					.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
					.pNext = nullptr,
					.allocationSize = imageMemoryRequirements.size,
					.memoryTypeIndex = this->_pContext->findMemoryType(imageMemoryRequirements.memoryTypeBits, ::vk::MemoryPropertyFlagBits::eDeviceLocal).value()
				};
				JJYOU_VK_UTILS_CHECK(this->_pAllocator->allocate(&imageMemoryAllocInfo, this->_imageMemory));
				vkBindImageMemory(*this->_pContext->device(), this->_image, this->_imageMemory.memory(), this->_imageMemory.offset());
				// Create and begin transfer command buffer
				VkCommandBuffer transferCommandBuffer, graphicsCommandBuffer;
				// Transfer image layout
				transferCommandBuffer = Texture2D::_beginCommandBuffer(*this->_pContext->device(), transferCommandPool);
				VkImageMemoryBarrier imageMemoryBarrier1{
					.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
					.pNext = nullptr,
					.srcAccessMask = 0,
					.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
					.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					.srcQueueFamilyIndex = transferQueueFamily,
					.dstQueueFamilyIndex = transferQueueFamily,
					.image = this->_image,
					.subresourceRange{
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.baseMipLevel = 0,
						.levelCount = this->_mipLevels,
						.baseArrayLayer = 0,
						.layerCount = this->_numLayers
					}
				};
				vkCmdPipelineBarrier(
					transferCommandBuffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &imageMemoryBarrier1
				);
				Texture2D::_endCommandBuffer(*this->_pContext->device(), transferCommandPool, transferCommandBuffer, **this->_pContext->queue(Context::QueueType::Transfer), nullptr, nullptr);
				// Create a stagine buffer.
				const VkDeviceSize maxBufferSize = elementSize * extent.width * extent.height * this->_numLayers;
				VkBuffer stagingBuffer = nullptr;
				Memory stagingBufferMemory{};
				VkBufferCreateInfo bufferInfo{
					.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.size = maxBufferSize,
					.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
					.queueFamilyIndexCount = 1U,
					.pQueueFamilyIndices = &transferQueueFamily
				};
				JJYOU_VK_UTILS_CHECK(vkCreateBuffer(*this->_pContext->device(), &bufferInfo, nullptr, &stagingBuffer));
				VkMemoryRequirements stagingBufferMemoryRequirements;
				vkGetBufferMemoryRequirements(*this->_pContext->device(), stagingBuffer, &stagingBufferMemoryRequirements);
				VkMemoryAllocateInfo stagingBufferMemoryAllocInfo{
					.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
					.pNext = nullptr,
					.allocationSize = stagingBufferMemoryRequirements.size,
					.memoryTypeIndex = this->_pContext->findMemoryType(stagingBufferMemoryRequirements.memoryTypeBits, ::vk::MemoryPropertyFlagBits::eHostVisible | ::vk::MemoryPropertyFlagBits::eHostCoherent).value()
				};
				JJYOU_VK_UTILS_CHECK(this->_pAllocator->allocate(&stagingBufferMemoryAllocInfo, stagingBufferMemory));
				vkBindBufferMemory(*this->_pContext->device(), stagingBuffer, stagingBufferMemory.memory(), stagingBufferMemory.offset());
				this->_pAllocator->map(stagingBufferMemory);
				// Copy data to image
				VkExtent2D levelExtent = this->_extent;
				for (std::uint32_t m = 0; m < this->_mipLevels; ++m) {
					// Copy data to staging buffer
					const VkDeviceSize bufferSize = elementSize * levelExtent.width * levelExtent.height * this->_numLayers;
					if (m == 0U)
						std::memcpy(stagingBufferMemory.mappedAddress(), data, bufferSize);
					else
						std::memcpy(stagingBufferMemory.mappedAddress(), mipData[m - 1U], bufferSize);
					// Copy buffer to image
					transferCommandBuffer = Texture2D::_beginCommandBuffer(*this->_pContext->device(), transferCommandPool);
					VkBufferImageCopy copyRegion{
						.bufferOffset = 0,
						.bufferRowLength = 0,
						.bufferImageHeight = 0,
						.imageSubresource{
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.mipLevel = m,
							.baseArrayLayer = 0,
							.layerCount = this->_numLayers,
						},
						.imageOffset = {
							.x = 0,
							.y = 0,
							.z = 0
						},
						.imageExtent = {
							.width = levelExtent.width,
							.height = levelExtent.height,
							.depth = 1
						}
					};
					vkCmdCopyBufferToImage(transferCommandBuffer, stagingBuffer, this->_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
					Texture2D::_endCommandBuffer(*this->_pContext->device(), transferCommandPool, transferCommandBuffer, **this->_pContext->queue(Context::QueueType::Transfer), nullptr, nullptr);
					levelExtent.width = std::max(levelExtent.width / 2U, 1U);
					levelExtent.height = std::max(levelExtent.height / 2U, 1U);
				}
				// Destroy staging buffer
				this->_pAllocator->unmap(stagingBufferMemory);
				this->_pAllocator->free(stagingBufferMemory);
				vkDestroyBuffer(*this->_pContext->device(), stagingBuffer, nullptr);
				// Transfer image layout
				if (transferQueueFamily != graphicsQueueFamily) {
					transferCommandBuffer = Texture2D::_beginCommandBuffer(*this->_pContext->device(), transferCommandPool);
					VkImageMemoryBarrier imageMemoryBarrier2{
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
						.dstAccessMask = VK_ACCESS_NONE,
						.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						.srcQueueFamilyIndex = transferQueueFamily,
						.dstQueueFamilyIndex = graphicsQueueFamily,
						.image = this->_image,
						.subresourceRange{
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.baseMipLevel = 0,
							.levelCount = this->_mipLevels,
							.baseArrayLayer = 0,
							.layerCount = this->_numLayers
						}
					};
					vkCmdPipelineBarrier(
						transferCommandBuffer,
						VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
						0,
						0, nullptr,
						0, nullptr,
						1, &imageMemoryBarrier2
					);
					Texture2D::_endCommandBuffer(*this->_pContext->device(), transferCommandPool, transferCommandBuffer, **this->_pContext->queue(Context::QueueType::Transfer), nullptr, nullptr);
					graphicsCommandBuffer = Texture2D::_beginCommandBuffer(*this->_pContext->device(), graphicsCommandPool);
					VkImageMemoryBarrier imageMemoryBarrier3{
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = VK_ACCESS_NONE,
						.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						.srcQueueFamilyIndex = graphicsQueueFamily,
						.dstQueueFamilyIndex = graphicsQueueFamily,
						.image = this->_image,
						.subresourceRange{
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.baseMipLevel = 0,
							.levelCount = this->_mipLevels,
							.baseArrayLayer = 0,
							.layerCount = this->_numLayers
						}
					};
					vkCmdPipelineBarrier(
						graphicsCommandBuffer,
						VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
						0,
						0, nullptr,
						0, nullptr,
						1, &imageMemoryBarrier3
					);
					Texture2D::_endCommandBuffer(*this->_pContext->device(), graphicsCommandPool, graphicsCommandBuffer, **this->_pContext->queue(Context::QueueType::Main), nullptr, nullptr);
				}
				else {
					transferCommandBuffer = Texture2D::_beginCommandBuffer(*this->_pContext->device(), transferCommandPool);
					VkImageMemoryBarrier imageMemoryBarrier2{
						.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
						.pNext = nullptr,
						.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
						.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
						.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						.srcQueueFamilyIndex = transferQueueFamily,
						.dstQueueFamilyIndex = transferQueueFamily,
						.image = this->_image,
						.subresourceRange{
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.baseMipLevel = 0,
							.levelCount = this->_mipLevels,
							.baseArrayLayer = 0,
							.layerCount = this->_numLayers
						}
					};
					vkCmdPipelineBarrier(
						transferCommandBuffer,
						VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
						0,
						0, nullptr,
						0, nullptr,
						1, &imageMemoryBarrier2
					);
					Texture2D::_endCommandBuffer(*this->_pContext->device(), transferCommandPool, transferCommandBuffer, **this->_pContext->queue(Context::QueueType::Transfer), nullptr, nullptr);
				}
				// Create the image view
				VkImageViewCreateInfo viewInfo{
					.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					.pNext = nullptr,
					.image = this->_image,
					.viewType = (cubeMap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D),
					.format = this->_format,
					.components = {
						.r = VK_COMPONENT_SWIZZLE_IDENTITY,
						.g = VK_COMPONENT_SWIZZLE_IDENTITY,
						.b = VK_COMPONENT_SWIZZLE_IDENTITY,
						.a = VK_COMPONENT_SWIZZLE_IDENTITY
					},
					.subresourceRange = {
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.baseMipLevel = 0,
						.levelCount = this->_mipLevels,
						.baseArrayLayer = 0,
						.layerCount = this->_numLayers
					}
				};
				JJYOU_VK_UTILS_CHECK(vkCreateImageView(*this->_pContext->device(), &viewInfo, nullptr, &this->_imageView));
				// Create the sampler
				VkSamplerCreateInfo samplerInfo{
					.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.magFilter = VK_FILTER_LINEAR,
					.minFilter = VK_FILTER_LINEAR,
					.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
					.addressModeU = addressMode,
					.addressModeV = addressMode,
					.addressModeW = addressMode,
					.mipLodBias = 0.0f,
					.anisotropyEnable = VK_TRUE,
					.maxAnisotropy = this->_pContext->physicalDevice().getProperties().limits.maxSamplerAnisotropy,
					.compareEnable = VK_FALSE,
					.compareOp = VK_COMPARE_OP_ALWAYS,
					.minLod = 0.0f,
					.maxLod = VK_LOD_CLAMP_NONE,
					.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
					.unnormalizedCoordinates = VK_FALSE,
				};
				JJYOU_VK_UTILS_CHECK(vkCreateSampler(*this->_pContext->device(), &samplerInfo, nullptr, &this->_sampler));
			}

			/** @brief	Call the corresponding vkDestroyXXX function to destroy the wrapped instance.
			  */
			void destroy(void) {
				if (this->_pContext != nullptr) {
					this->_pAllocator->free(this->_imageMemory);
					this->_pAllocator = nullptr;
					vkDestroySampler(*this->_pContext->device(), this->_sampler, nullptr);
					this->_sampler = nullptr;
					vkDestroyImageView(*this->_pContext->device(), this->_imageView, nullptr);
					this->_imageView = nullptr;
					vkDestroyImage(*this->_pContext->device(), this->_image, nullptr);
					this->_image = nullptr;
					this->_extent = { .width = 0, .height = 0 };
					this->_mipLevels = 0;
					this->_format = VK_FORMAT_UNDEFINED;
					this->_pContext = nullptr;
				}
			}

			/** @brief	Get the sampler for this texture.
			  * @return `VkSampler` instance.
			  */
			VkSampler sampler(void) const { return this->_sampler; }

			/** @brief	Get the image for this texture.
			  * @return `VkImage` instance.
			  */
			VkImage image(void) const { return this->_image; }

			/** @brief	Get the image view for this texture.
			  * @return `VkImageView` instance.
			  */
			VkImageView imageView(void) const { return this->_imageView; }

			/** @brief	Get the texture image extent.
			  * @return Texture image extent.
			  */
			VkExtent2D extent(void) const { return this->_extent; }

			/** @brief	Get the number of texture layers.
			  * @return Number of texture image layers.
			  */
			std::uint32_t numLayers(void) const { return this->_numLayers; }

			/** @brief	Get the texture image mipmap.
			  * @return Texture image mipmap levels.
			  */
			std::uint32_t mipLevels(void) const { return this->_mipLevels; }

			/** @brief	Get the texture format.
			  * @return Texture format.
			  */
			VkFormat format(void) const { return this->_format; }

		private:

			const Context* _pContext = nullptr;
			VkExtent2D _extent{};
			std::uint32_t _numLayers = 0;
			std::uint32_t _mipLevels = 0;
			VkFormat _format = VK_FORMAT_UNDEFINED;
			MemoryAllocator* _pAllocator = nullptr;
			VkImage _image = nullptr;
			Memory _imageMemory{};
			VkImageView _imageView = nullptr;
			VkSampler _sampler = nullptr;
			static VkCommandBuffer _beginCommandBuffer(VkDevice device, VkCommandPool commandPool) {
				VkCommandBuffer commandBuffer;
				VkCommandBufferAllocateInfo commandBufferAllocInfo{
					.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
					.commandPool = commandPool,
					.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
					.commandBufferCount = 1,
				};
				JJYOU_VK_UTILS_CHECK(vkAllocateCommandBuffers(device, &commandBufferAllocInfo, &commandBuffer));
				VkCommandBufferBeginInfo beginInfo{
					.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
					.pNext = nullptr,
					.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
					.pInheritanceInfo = nullptr
				};
				JJYOU_VK_UTILS_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
				return commandBuffer;
			}
			static void _endCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore) {
				JJYOU_VK_UTILS_CHECK(vkEndCommandBuffer(commandBuffer));
				VkSubmitInfo submitInfo{
					.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
					.pNext = nullptr,
					.waitSemaphoreCount = (waitSemaphore == nullptr) ? 0U : 1U,
					.pWaitSemaphores = &waitSemaphore,
					.pWaitDstStageMask = 0,
					.commandBufferCount = 1,
					.pCommandBuffers = &commandBuffer,
					.signalSemaphoreCount = (signalSemaphore == nullptr) ? 0U : 1U,
					.pSignalSemaphores = &signalSemaphore
				};
				JJYOU_VK_UTILS_CHECK(vkQueueSubmit(queue, 1, &submitInfo, nullptr));
				JJYOU_VK_UTILS_CHECK(vkQueueWaitIdle(queue));
				vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
			};
			
		};

	}

}