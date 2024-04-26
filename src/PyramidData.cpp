#include "PyramidData.hpp"
#include "KinectFusion.hpp"
#include <stdexcept>

#define VK_THROW(err) \
	throw std::runtime_error("[PyramidData] Vulkan error in file " + std::string(__FILE__) + " line " + std::to_string(__LINE__) + ": " + vk::to_string(err))

#define VK_CHECK(value) \
	if (vk::Result err = (value); err != vk::Result::eSuccess) { VK_THROW(err); }

/** @brief	Construct an empty PyramidData.
	  */
PyramidData::PyramidData(
	const Engine & engine_,
	const KinectFusion & kinectFusion_,
	vk::Extent2D extent_
) :
	_pEngine(&engine_),
	_pKinectFusion(&kinectFusion_),
	_descriptorSetLayout(*kinectFusion_.pyramidDataDescriptorSetLayout())
{
	// Create textures
	{
		constexpr std::array<vk::Format, 3> formats = { {
			vk::Format::eR32Sfloat,
			vk::Format::eR32G32B32A32Sfloat,
			vk::Format::eR32G32B32A32Sfloat
		} };
		for (std::uint32_t i = 0; i < PyramidData::numTextures; ++i) {
			this->_textures[i] = Texture2D(
				*this->_pEngine,
				formats[i],
				extent_,
				vk::ImageUsageFlagBits::eStorage,
				{ *this->_pEngine->context().queueFamilyIndex(jjyou::vk::Context::QueueType::Compute) }
			);
		}
	}
	// Create and update descriptor set
	{
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(*this->_pEngine->descriptorPool())
			.setDescriptorSetCount(1)
			.setSetLayouts(this->_descriptorSetLayout);
		this->_descriptorSet = std::move(this->_pEngine->context().device().allocateDescriptorSets(descriptorSetAllocateInfo)[0]);
		std::array<vk::DescriptorImageInfo, PyramidData::numTextures> descriptorImageInfos{};
		std::array<vk::WriteDescriptorSet, PyramidData::numTextures> writeDescriptorSets{};
		for (std::uint32_t i = 0; i < PyramidData::numTextures; ++i) {
			descriptorImageInfos[i]
				.setSampler(nullptr)
				.setImageView(*this->_textures[i].imageView())
				.setImageLayout(vk::ImageLayout::eGeneral);
			writeDescriptorSets[i]
				.setDstSet(*this->_descriptorSet)
				.setDstBinding(i)
				.setDstArrayElement(0)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eStorageImage)
				.setImageInfo(descriptorImageInfos[i]);
		}
		this->_pEngine->context().device().updateDescriptorSets(writeDescriptorSets, nullptr);
	}
	// Transition texture layouts
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
		vk::ImageMemoryBarrier imageMemoryBarrier = vk::ImageMemoryBarrier()
			.setSrcAccessMask(vk::AccessFlags(0))
			.setDstAccessMask(vk::AccessFlags(0))
			.setOldLayout(vk::ImageLayout::eUndefined)
			.setNewLayout(vk::ImageLayout::eGeneral)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			//.setImage()
			.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
		for (std::uint32_t i = 0; i < PyramidData::numTextures; ++i) {
			imageMemoryBarrier.setImage(*this->_textures[i].image());
			computeCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags(0), nullptr, nullptr, imageMemoryBarrier);
		}
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