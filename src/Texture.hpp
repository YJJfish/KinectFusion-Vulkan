#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <jjyou/vk/Vulkan.hpp>
#include <jjyou/glsl/glsl.hpp>
#include "Primitives.hpp"

class Engine;

/***********************************************************************
 * @class	Texture2D
 * @brief	Texture2D class that manages relevant Vulkan resources for a
 *			2-dimension texture.
 * 
 *			In this application, a texture includes an image, the binding
 *			image memory, and the image view. The image memory is always
 *			in local device (vk::MemoryPropertyFlagBits::eDeviceLocal).
 *			The image sharing mode is exclusive (vk::SharingMode::eExclusive).
 *			The aspect mask of the image view will either be color or depth, depending
 *			on the image format.
 *			Upon creation, the image layout is undefined and not owned by any queue.
 ***********************************************************************/
class Texture2D {

public:

	/** @brief	Construct an empty texture.
	  */
	Texture2D(std::nullptr_t) {}

	/** @brief	Construct a texture given format, extent, usage, and queue family indices.	
	  */
	Texture2D(
		const Engine& engine_,
		vk::Format format_,
		vk::Extent2D extent_,
		vk::ImageUsageFlags usage_,
		const std::set<std::uint32_t>& queueFamilyIndices_
	);

	/** @brief	Copy constructor is disabled.
	  */
	Texture2D(const Texture2D&) = delete;

	/** @brief	Move constructor.
	  */
	Texture2D(Texture2D&& other_) = default;

	/** @brief	Copy assignment is disabled.
	  */
	Texture2D& operator=(const Texture2D&) = delete;

	/** @brief	Move assignment.
	  */
	Texture2D& operator=(Texture2D&& other_) noexcept {
		if (this != &other_) {
			this->_pEngine = other_._pEngine;
			this->_image = std::move(other_._image);
			this->_imageMemory = std::move(other_._imageMemory);
			this->_imageView = std::move(other_._imageView);
			this->_format = std::move(other_._format);
			this->_extent = std::move(other_._extent);
		}
		return *this;
	}

	/** @brief	Destructor.
	  */
	~Texture2D(void) = default;

	/** @brief	Get the image for this texture.
	  */
	const vk::raii::Image& image(void) const { return this->_image; }

	/** @brief	Get the image view for this texture.
	  */
	const vk::raii::ImageView& imageView(void) const { return this->_imageView; }

	/** @brief	Get the texture format.
	  */
	constexpr vk::Format format(void) const { return this->_format; }

	/** @brief	Get the texture image extent.
	  */
	constexpr vk::Extent2D extent(void) const { return this->_extent; }

	/** @brief	Get the number of texture image mipmap levels.
	  */
	constexpr std::uint32_t mipLevels(void) const { return 1U; }

	/** @brief	Get the number of texture layers.
	  */
	constexpr std::uint32_t numLayers(void) const { return 1U; }

private:

	const Engine* _pEngine = nullptr;
	vk::raii::Image _image{ nullptr };
	vk::Format _format = vk::Format::eUndefined;
	vk::Extent2D _extent{};
	jjyou::vk::VmaAllocation _imageMemory{ nullptr };
	vk::raii::ImageView _imageView{ nullptr };

};

/***********************************************************************
 * @class	Surface
 * @brief	Surface class that can be used for engine's surface rendering.
 * 
 *			Simple surfaces have color and depth maps. Lambertian surfaces
 *			have color, depth, and normal maps.
 *			In current implementation, Surface textures are always in general
 *			layout, and concurrent shared by the graphics and compute queue families.
 *			Performance improvements may be made in the future.
 ***********************************************************************/
template <MaterialType _materialType>
class Surface {

public:

	static inline constexpr MaterialType materialType = _materialType;
	static inline constexpr std::uint32_t numTextures = ((_materialType == MaterialType::Simple) ? (2) : (3));

	/** @brief	Construct an empty surface in invalid state.
	  */
	Surface(std::nullptr_t) {}

	/** @brief	Copy constructor is disabled.
	  */
	Surface(const Surface&) = delete;

	/** @brief	Move constructor.
	  */
	Surface(Surface&& other_) = default;

	/** @brief	Destructor.
	  */
	~Surface(void) = default;
	
	/** @brief	Copy assignment is disabled.
	  */
	Surface& operator=(const Surface&) = delete;

	/** @brief	Move assignment.
	  */
	Surface& operator=(Surface&& other_) noexcept {
		if (this != &other_) {
			
		}
		return *this;
	}

	/** @brief	Construct an empty surface.
	  */
	Surface(
		const Engine& engine_,
		const vk::raii::DescriptorSetLayout& samplerDescriptorSetLayout_, // combined image samplers
		const vk::raii::DescriptorSetLayout& storageDescriptorSetLayout_  // storage images
	);

	/** @brief	Create textures, and optionally upload data from CPU.
	  * 
	  *			The data formats of color map should be R8G8B8A8Unorm.
	  *			The data formats of depth map should be R32Sfloat.
	  *			The data formats of normal map should be R32G32B32A32Sfloat.
	  */
	Surface& createTextures(
		std::array<vk::Extent2D, Surface::numTextures> extents_,
		std::optional<std::array<const void*, Surface::numTextures>> data_ = std::nullopt
	);

	/** @brief	Bind the sampler descriptor set.
	  */
	void bindSampler(
		const vk::raii::CommandBuffer& commandBuffer_,
		vk::PipelineBindPoint pipelineBindPoint_,
		const vk::raii::PipelineLayout& pipelineLayout_,
		std::uint32_t setIndex_
	) const {
		commandBuffer_.bindDescriptorSets(pipelineBindPoint_, *pipelineLayout_, setIndex_, *this->_samplerDescriptorSet, nullptr);
	}

	/** @brief	Bind the storage descriptor set.
	  */
	void bindStorage(
		const vk::raii::CommandBuffer& commandBuffer_,
		vk::PipelineBindPoint pipelineBindPoint_,
		const vk::raii::PipelineLayout& pipelineLayout_,
		std::uint32_t setIndex_
	) const {
		commandBuffer_.bindDescriptorSets(pipelineBindPoint_, *pipelineLayout_, setIndex_, *this->_storageDescriptorSet, nullptr);
	}

	/** @brief	Draw the surface.
	  */
	void draw(const vk::raii::CommandBuffer& commandBuffer_) const {
		commandBuffer_.draw(6, 1, 0, 0);
	}

	/** @brief	Create the descriptor set layout of combind image samplers.
	  */
	static vk::raii::DescriptorSetLayout createSamplerDescriptorSetLayout(const vk::raii::Device& device_) {
		std::array<vk::DescriptorSetLayoutBinding, Surface::numTextures> descriptorSetLayoutBindings;
		for (std::uint32_t i = 0; i < Surface::numTextures; ++i) {
			descriptorSetLayoutBindings[i]
				.setBinding(i)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment)
				.setPImmutableSamplers(nullptr);
		};
		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setFlags(vk::DescriptorSetLayoutCreateFlags(0))
			.setBindings(descriptorSetLayoutBindings);
		return vk::raii::DescriptorSetLayout(device_, descriptorSetLayoutCreateInfo);
	}

	/** @brief	Create the descriptor set layout of storage images.
	  */
	static vk::raii::DescriptorSetLayout createStorageDescriptorSetLayout(const vk::raii::Device& device_) {
		std::array<vk::DescriptorSetLayoutBinding, Surface::numTextures> descriptorSetLayoutBindings;
		for (std::uint32_t i = 0; i < Surface::numTextures; ++i) {
			descriptorSetLayoutBindings[i]
				.setBinding(i)
				.setDescriptorType(vk::DescriptorType::eStorageImage)
				.setDescriptorCount(1)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment)
				.setPImmutableSamplers(nullptr);
		};
		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setFlags(vk::DescriptorSetLayoutCreateFlags(0))
			.setBindings(descriptorSetLayoutBindings);
		return vk::raii::DescriptorSetLayout(device_, descriptorSetLayoutCreateInfo);
	}

private:

	const Engine* _pEngine = nullptr;
	vk::DescriptorSetLayout _samplerDescriptorSetLayout{ nullptr }; // Descriptor set layout should be owned by the engine.
	vk::DescriptorSetLayout _storageDescriptorSetLayout{ nullptr }; // Descriptor set layout should be owned by the engine.
	std::vector<Texture2D> _textures{};
	vk::raii::Sampler _sampler{ nullptr };
	vk::raii::DescriptorSet _samplerDescriptorSet{ nullptr };
	vk::raii::DescriptorSet _storageDescriptorSet{ nullptr };
	
};