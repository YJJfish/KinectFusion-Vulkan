#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <jjyou/vk/Vulkan.hpp>
#include <jjyou/glsl/glsl.hpp>
#include <optional>
#include "Engine.hpp"

class KinectFusion;

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

	/***********************************************************************
	 * @class	TSDFParams
	 * @brief	TSDF volume storage buffer header.
	 * 
	 * In the compute shader, the TSDF volume storage buffer header is
	 * made up of two parts: The header which includes the parameters;
	 * And an array of ivec2 which includes the data (tsdf + weight + color). 
	 * This C++ structure corresponds to the header.
	 ***********************************************************************/
	struct TSDFParams {
		jjyou::glsl::uvec3 resolution;
		float size;
		jjyou::glsl::vec3 corner;
		float truncationDistance;
	};

	/** @brief	Construct an empty volume in invalid state.
	  */
	TSDFVolume(std::nullptr_t) {}

	/** @brief	Create a volume.
	  * @param	engine_					Vulkan engine.
	  * @param	kinectFusion_			The KinectFusion instance that owns this volume.
	  * @param	resolution_				Voxel resolution.
	  * @param	size_					Voxel size, in meter.
	  * @param	corner_					The coordinate of the corner voxel's center point.
	  *									By default, the volume will be placed such that
	  *									its center point is at the origin.
	  * @param	truncationDistance_		Truncation distance. By default, it is 3x the voxel size.
	  */
	TSDFVolume(
		// Vulkan resources
		const Engine& engine_,

		// The KinectFusion that owns the volume
		const KinectFusion& kinectFusion_,

		// Volume parameters
		const jjyou::glsl::uvec3& resolution_,
		float size_,
		std::optional<jjyou::glsl::vec3> corner_ = std::nullopt,
		std::optional<float> truncationDistance_ = std::nullopt
	);

	/** @brief	Copy constructor is disabled.
	  */
	TSDFVolume(const TSDFVolume&) = delete;

	/** @brief	Move constructor.
	  */
	TSDFVolume(TSDFVolume&& other_) = default;

	/** @brief	Copy assignment is disabled.
	  */
	TSDFVolume& operator=(const TSDFVolume&) = delete;

	/** @brief	Move assignment.
	  */
	TSDFVolume& operator=(TSDFVolume&& other_) noexcept {
		if (this != &other_) {
			this->clear();
			this->_pEngine = other_._pEngine;
			this->_pKinectFusion = other_._pKinectFusion;
			this->_descriptorSetLayout = other_._descriptorSetLayout;
			this->_resolution = other_._resolution;
			this->_size = other_._size;
			this->_corner = other_._corner;
			this->_truncationDistance = other_._truncationDistance;
			this->_bufferSize = other_._bufferSize;
			this->_volume = std::move(other_._volume);
			this->_volumeMemory = std::move(other_._volumeMemory);
			this->_descriptorSet = std::move(other_._descriptorSet);
		}
		return *this;
	}

	/** @brief	Explicitly clear the volume.
	  */
	void clear(void) {
		this->~TSDFVolume();
	}

	/** @brief	Destructor.
	  */
	~TSDFVolume(void) = default;

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
	vk::DeviceSize bufferSize(void) const { return this->_bufferSize; }

	/** @brief	Get the descriptor set layout for the volume storage buffer.
	  */
	vk::DescriptorSetLayout descriptorSetLayout(void) const { return this->_descriptorSetLayout; }
	
	/** @brief	Bind the descriptor set.
	  */
	void bind(
		const vk::raii::CommandBuffer& commandBuffer_,
		vk::PipelineBindPoint pipelineBindPoint_,
		const vk::raii::PipelineLayout& pipelineLayout_,
		std::uint32_t setIndex_
	) const {
		commandBuffer_.bindDescriptorSets(pipelineBindPoint_, *pipelineLayout_, setIndex_, *this->_descriptorSet, nullptr);
	}
	
	/** @brief	TODO: Copy CPU memory to GPU storage buffer memory.
	  */
	void upload(const jjyou::glsl::vec2* src_) const {}

	/** @brief	TODO: Copy GPU storage buffer memory to CPU memory.
	  */
	void download(jjyou::glsl::vec2* dst_) const {}

	/** @brief	Create the descriptor set layout for TSDF volume storage buffer.
	  */
	static vk::raii::DescriptorSetLayout createDescriptorSetLayout(const vk::raii::Device& device_) {
		std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
		vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorType(vk::DescriptorType::eStorageBuffer)
		.setDescriptorCount(1)
		.setStageFlags(vk::ShaderStageFlagBits::eCompute)
		.setPImmutableSamplers(nullptr)
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
	jjyou::glsl::uvec3 _resolution{};
	float _size = 0.0f;
	jjyou::glsl::vec3 _corner{};
	float _truncationDistance = 0.0f;
	vk::DeviceSize _bufferSize = 0ULL;
	vk::raii::Buffer _volume{ nullptr };
	jjyou::vk::VmaAllocation _volumeMemory{ nullptr };
	vk::raii::DescriptorSet _descriptorSet{ nullptr };

	void _createStorageBuffer(void);
	void _createDescriptorSet(void);
};