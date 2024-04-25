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
	_pKinectFusion(&kinectFusion_)//,
	//_descriptorSetLayout(*kinectFusion_.pyramidDataDescriptorSetLayout())
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
	// Create descriptor set
	{
		vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(*this->_pEngine->descriptorPool())
			.setDescriptorSetCount(1)
			.setSetLayouts(this->_descriptorSetLayout);
		this->_descriptorSet = std::move(this->_pEngine->context().device().allocateDescriptorSets(descriptorSetAllocateInfo)[0]);
	}
}