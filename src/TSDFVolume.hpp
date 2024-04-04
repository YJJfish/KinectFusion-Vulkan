#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <jjyou/vk/Vulkan.hpp>
#include <jjyou/glsl/glsl.hpp>
#include <optional>

/***********************************************************************
 * @class	TSDFVolume
 * @brief	TSDFVolume class that manages the properties and the GPU memory
 *			of a TSDF volume.
 * 
 *	This class follows RAII design pattern. Similar to Vulkan RAII
 *	wrappers, it has a constructor that takes std::nullptr to construct
 *	an empty volume.
 ***********************************************************************/
class TSDFVolume {

public:

	/** @brief	Construct an empty volume.
	  */
	TSDFVolume(std::nullptr_t) {}

	/** @brief	Create a volume.
	  * @param	context_				Vulkan context.
	  * @param	allocator_				Vulkan memory allocator.
	  * @param	descriptorPool_			Vulkan descriptor pool.
	  * @param	resolution_				Voxel resolution.
	  * @param	size_					Voxel size, in meter.
	  * @param	corner_					Volume corner. By default, the volume will be placed
	  *									such that its center point is at the origin.
	  * @param	truncationDistance_		Truncation distance. By default, it is 3x the voxel size.
	  */
	TSDFVolume(
		// Vulkan resources
		const jjyou::vk::Context& context_,
		const jjyou::vk::VmaAllocator& allocator_,
		const vk::raii::DescriptorPool& descriptorPool_,

		// Volume parameters
		const jjyou::glsl::uvec3& resolution_,
		float size_,
		std::optional<jjyou::glsl::vec3> corner_ = std::nullopt,
		std::optional<float> truncationDistance_ = std::nullopt
	);

	/** @brief	Get the voxel resolution (i.e. number of voxels along the x/y/z axis).
	  */
	const jjyou::glsl::uvec3& resolution(void) const { return this->_resolution; }

	/** @brief	Get the voxel size.
	  */
	float size(void) const { return this->_size; }

	/** @brief	Get the coordinate of the volume's corner (minX, minY, minZ).
	  */
	const jjyou::glsl::vec3& corner(void) const { return this->_corner; }

	/** @brief	Get the truncation distance.
	  */
	float truncationDistance(void) const { return this->_truncationDistance; }

	/** @brief	Get the underlying storage buffer size.
	  */
	float bufferSize(void) const { return this->_bufferSize; }

	/** @brief	Get the descriptor set layout for the storage buffer.
	  */
	const vk::raii::DescriptorSetLayout& descriptorSetLayout(void) const { return this->_descriptorSetLayout; }

	/** @brief	Get the descriptor set for the storage buffer.
	  */
	const vk::raii::DescriptorSet& descriptorSet(void) const { return this->_descriptorSet; }

	/** @brief	Copy CPU memory to GPU storage buffer memory.
	  */
	void upload(const jjyou::glsl::vec2* src_) const {}

	/** @brief	Copy GPU storage buffer memory to CPU memory.
	  */
	void download(jjyou::glsl::vec2* dst_) const {}

private:

	const jjyou::vk::Context* _pContext = nullptr;
	const jjyou::vk::VmaAllocator* _pAllocator = nullptr;
	vk::DescriptorPool _descriptorPool{ nullptr };
	jjyou::glsl::uvec3 _resolution{};
	float _size = 0.0f;
	jjyou::glsl::vec3 _corner{};
	float _truncationDistance = 0.0f;
	vk::DeviceSize _bufferSize = 0ULL;
	vk::raii::Buffer _volume{ nullptr };
	jjyou::vk::VmaAllocation _volumeMemory{ nullptr };
	vk::raii::DescriptorSetLayout _descriptorSetLayout{ nullptr };
	vk::raii::DescriptorSet _descriptorSet{ nullptr };

	void _createDescriptorSetLayout(void);
	void _createStorageBuffer(void);
	void _createDescriptorSet(void);
};