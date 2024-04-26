#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <jjyou/vk/Vulkan.hpp>
#include <jjyou/glsl/glsl.hpp>
#include "Texture.hpp"

class KinectFusion;

/***********************************************************************
 * @class	PyramidData
 * @brief	A class made up of a depth map, a vertex map, and a normal map
 *			in one level of the pyramid.
 *
 *			All the three maps are stored in images. The formats are
 *			R32Sfloat, R32G32B32A32Sfloat, and R32G32B32A32Sfloat, respectively.
 *			Different from graphics rendering, we use float format for normal map
 *			for higher precision. The last channel of vertex/normal map serves
 *			as the validity mask (zero for invalid, nonzero for valid).
 *			The images will be exclusively owned by the compute queue. The images'
 *			layout will be `vk::ImageLayout::eGeneral`.
 ***********************************************************************/
class PyramidData {

public:

	static inline constexpr std::uint32_t numTextures = 3U;

	/** @brief	Construct an empty PyramidData in invalid state.
	  */
	PyramidData(std::nullptr_t) {}

	/** @brief	Copy constructor is disabled.
	  */
	PyramidData(const PyramidData&) = delete;

	/** @brief	Move constructor.
	  */
	PyramidData(PyramidData&& other_) = default;

	/** @brief	Explicitly clear the surface.
	  */
	void clear(void) {
		this->~PyramidData();
	}

	/** @brief	Destructor.
	  */
	~PyramidData(void) = default;

	/** @brief	Copy assignment is disabled.
	  */
	PyramidData& operator=(const PyramidData&) = delete;

	/** @brief	Move assignment.
	  */
	PyramidData& operator=(PyramidData&& other_) noexcept {
		if (this != &other_) {
			this->_pEngine = other_._pEngine;
			this->_pKinectFusion = other_._pKinectFusion;
			this->_descriptorSetLayout = other_._descriptorSetLayout;
			this->_textures = std::move(other_._textures);
			this->_descriptorSet = std::move(other_._descriptorSet);
		}
		return *this;
	}

	/** @brief	Construct an empty PyramidData.
	  */
	PyramidData(const Engine& engine_, const KinectFusion& kinectFusion_, vk::Extent2D extent_);

	/** @brief	Bind the descriptor set of 3 storage image descriptors.
	  */
	void bind(
		const vk::raii::CommandBuffer& commandBuffer_,
		vk::PipelineBindPoint pipelineBindPoint_,
		const vk::raii::PipelineLayout& pipelineLayout_,
		std::uint32_t setIndex_
	) const {
		commandBuffer_.bindDescriptorSets(pipelineBindPoint_, *pipelineLayout_, setIndex_, *this->_descriptorSet, nullptr);
	}

	/** @brief	Get the underlying texture.
	  */
	const Texture2D& texture(std::uint32_t index_) const {
		return this->_textures[index_];
	}

	/** @brief	Get the descriptor set layout of 3 storage image descriptors.
	  */
	vk::DescriptorSetLayout descriptorSetLayout(void) const {
		return this->_descriptorSetLayout;
	}

	/** @brief	Create the descriptor set layout of 3 storage image descriptors.
	  */
	static vk::raii::DescriptorSetLayout createDescriptorSetLayout(const vk::raii::Device& device_) {
		std::array<vk::DescriptorSetLayoutBinding, PyramidData::numTextures> descriptorSetLayoutBindings;
		for (std::uint32_t i = 0; i < PyramidData::numTextures; ++i) {
			descriptorSetLayoutBindings[i]
				.setBinding(i)
				.setDescriptorType(vk::DescriptorType::eStorageImage)
				.setDescriptorCount(1)
				.setStageFlags(vk::ShaderStageFlagBits::eCompute)
				.setPImmutableSamplers(nullptr);
		};
		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setFlags(vk::DescriptorSetLayoutCreateFlags(0))
			.setBindings(descriptorSetLayoutBindings);
		return vk::raii::DescriptorSetLayout(device_, descriptorSetLayoutCreateInfo);
	}

private:

	const Engine* _pEngine = nullptr;
	const KinectFusion* _pKinectFusion = nullptr;
	vk::DescriptorSetLayout _descriptorSetLayout{ nullptr }; // Descriptor set layout should be owned by KinectFusion.
	std::array<Texture2D, PyramidData::numTextures> _textures{ { Texture2D{nullptr},Texture2D{nullptr},Texture2D{nullptr} } };
	vk::raii::DescriptorSet _descriptorSet{ nullptr };

};