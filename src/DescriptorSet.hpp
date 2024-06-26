#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <jjyou/vk/Vulkan.hpp>
#include <jjyou/glsl/glsl.hpp>
#include <stdexcept>

class Engine;
class KinectFusion;

/***********************************************************************
 * @class	ViewLevelDescriptorSet
 * @brief	Descriptor set 0 in the scene rendering shaders.
 ***********************************************************************/
class ViewLevelDescriptorSet {

public:

	/***********************************************************************
	 * @class	CameraParameters
	 * @brief	Binding 0 uniform buffer in the shaders.
	 ***********************************************************************/
	struct CameraParameters {
		jjyou::glsl::mat4 projection{};
		jjyou::glsl::mat4 view{};
		jjyou::glsl::vec4 viewPos{};
	};

	/** @brief	Construct an empty descriptor set in invalid state.
	  */
	ViewLevelDescriptorSet(std::nullptr_t) {}

	/** @brief	Construct a descriptor set given the engine.
	  */
	ViewLevelDescriptorSet(const Engine& engine_);

	/** @brief	Copy constructor is disabled.
	  */
	ViewLevelDescriptorSet(const ViewLevelDescriptorSet&) = delete;

	/** @brief	Move constructor.
	  */
	ViewLevelDescriptorSet(ViewLevelDescriptorSet&& other_) = default;

	/** @brief	Copy assignment is disabled.
	  */
	ViewLevelDescriptorSet& operator=(const ViewLevelDescriptorSet&) = delete;

	/** @brief	Move assignment.
	  */
	ViewLevelDescriptorSet& operator=(ViewLevelDescriptorSet&& other_) noexcept {
		if (this != &other_) {
			this->_pEngine = other_._pEngine;
			this->_descriptorSetLayout = other_._descriptorSetLayout;
			this->_descriptorSet = std::move(other_._descriptorSet);
			this->_cameraParametersBuffer = std::move(other_._cameraParametersBuffer);
			this->_cameraParametersBufferMemory = std::move(other_._cameraParametersBufferMemory);
			this->_cameraParametersBufferMemoryMappedAddress = other_._cameraParametersBufferMemoryMappedAddress;
		}
		return *this;
	}

	/** @brief	Destructor.
	  */
	~ViewLevelDescriptorSet(void) = default;

	/** @brief	Get the descriptor set.
	  */
	const vk::raii::DescriptorSet& descriptorSet(void) const { return this->_descriptorSet; }

	/** @brief	Get the mapped address for CameraParameters (binding 0).
	  */
	CameraParameters& cameraParameters(void) const { return *reinterpret_cast<ViewLevelDescriptorSet::CameraParameters*>(this->_cameraParametersBufferMemoryMappedAddress); }
	
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

	/** @brief	Get the descriptor set layout.
	  */
	vk::DescriptorSetLayout descriptorSetLayout(void) const {
		return this->_descriptorSetLayout;
	}
	
	/** @brief	Create the descriptor set layout.
	  */
	static vk::raii::DescriptorSetLayout createDescriptorSetLayout(const vk::raii::Device& device_) {
		std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
			vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
			.setPImmutableSamplers(nullptr)
		};
		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setFlags(vk::DescriptorSetLayoutCreateFlags(0))
			.setBindings(descriptorSetLayoutBindings);
		return vk::raii::DescriptorSetLayout(device_, descriptorSetLayoutCreateInfo);
	}

private:

	const Engine* _pEngine = nullptr;
	vk::DescriptorSetLayout _descriptorSetLayout{ nullptr }; // Descriptor set layout should be owned by the engine.
	vk::raii::DescriptorSet _descriptorSet{ nullptr };
	vk::raii::Buffer _cameraParametersBuffer{ nullptr };
	jjyou::vk::VmaAllocation _cameraParametersBufferMemory{ nullptr };
	void* _cameraParametersBufferMemoryMappedAddress = nullptr;

};

/***********************************************************************
 * @class	InstanceLevelDescriptorSet
 * @brief	Descriptor set 1 in the primitives shaders.
 ***********************************************************************/
class InstanceLevelDescriptorSet {

public:

	/***********************************************************************
	 * @class	ModelTransforms
	 * @brief	Binding 0 dynamic uniform buffer in the shaders.
	 ***********************************************************************/
	struct ModelTransforms {
		jjyou::glsl::mat4 model{};
		jjyou::glsl::mat4 normal{};
	};

	/** @brief	Construct an empty descriptor set in invalid state.
	  */
	InstanceLevelDescriptorSet(std::nullptr_t) {}

	/** @brief	Construct a descriptor set given the engine and the number of ModelTransforms.
	  */
	InstanceLevelDescriptorSet(const Engine& engine_, std::uint32_t numModelTransforms_);

	/** @brief	Copy constructor is disabled.
	  */
	InstanceLevelDescriptorSet(const InstanceLevelDescriptorSet&) = delete;

	/** @brief	Move constructor.
	  */
	InstanceLevelDescriptorSet(InstanceLevelDescriptorSet&& other_) = default;

	/** @brief	Copy assignment is disabled.
	  */
	InstanceLevelDescriptorSet& operator=(const InstanceLevelDescriptorSet&) = delete;

	/** @brief	Move assignment.
	  */
	InstanceLevelDescriptorSet& operator=(InstanceLevelDescriptorSet&& other_) noexcept {
		if (this != &other_) {
			this->_pEngine = other_._pEngine;
			this->_descriptorSetLayout = other_._descriptorSetLayout;
			this->_descriptorSet = std::move(other_._descriptorSet);
			this->_modelTransformsBufferOffset = other_._modelTransformsBufferOffset;
			this->_modelTransformsBuffer = std::move(other_._modelTransformsBuffer);
			this->_modelTransformsBufferMemory = std::move(other_._modelTransformsBufferMemory);
			this->_modelTransformsBufferMemoryMappedAddress = other_._modelTransformsBufferMemoryMappedAddress;
			this->_numModelTransforms = other_._numModelTransforms;
		}
		return *this;
	}

	/** @brief	Destructor.
	  */
	~InstanceLevelDescriptorSet(void) = default;

	/** @brief	Get the descriptor set.
	  */
	const vk::raii::DescriptorSet& descriptorSet(void) const { return this->_descriptorSet; }

	/** @brief	Get the mapped address for ModelTransforms (binding 0).
	  */
	ModelTransforms& modelTransforms(std::uint32_t idx_) const {
		char* baseAddr = reinterpret_cast<char*>(this->_modelTransformsBufferMemoryMappedAddress);
		char* offsetAddr = baseAddr + this->modelTransformsDynamicOffset(idx_);
		return *reinterpret_cast<ModelTransforms*>(offsetAddr);
	}

	/** @brief	Get the number of model transforms in the dynamic uniform buffer at binding 0.
	  */
	std::uint32_t numModelTransforms(void) const { return this->_numModelTransforms; }

	/** @brief	Get the dynamic uniform offset.
	  */
	std::uint32_t modelTransformsDynamicOffset(std::uint32_t idx_) const { return idx_ * static_cast<std::uint32_t>(this->_modelTransformsBufferOffset); }

	/** @brief	Bind the descriptor set.
	  */
	void bind(
		const vk::raii::CommandBuffer& commandBuffer_,
		vk::PipelineBindPoint pipelineBindPoint_,
		const vk::raii::PipelineLayout& pipelineLayout_,
		std::uint32_t setIndex_,
		std::uint32_t instanceIndex_
	) const {
		commandBuffer_.bindDescriptorSets(pipelineBindPoint_, *pipelineLayout_, setIndex_, *this->_descriptorSet, this->modelTransformsDynamicOffset(instanceIndex_));
	}

	/** @brief	Get the descriptor set layout.
	  */
	vk::DescriptorSetLayout descriptorSetLayout(void) const {
		return this->_descriptorSetLayout;
	}
	
	/** @brief	Create the descriptor set layout.
	  */
	static vk::raii::DescriptorSetLayout createDescriptorSetLayout(const vk::raii::Device& device_) {
		std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
			vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex)
			.setPImmutableSamplers(nullptr)
		};
		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
			.setFlags(vk::DescriptorSetLayoutCreateFlags(0))
			.setBindings(descriptorSetLayoutBindings);
		return vk::raii::DescriptorSetLayout(device_, descriptorSetLayoutCreateInfo);
	}

private:

	const Engine* _pEngine = nullptr;
	vk::DescriptorSetLayout _descriptorSetLayout{ nullptr }; // Descriptor set layout should be owned by the engine.
	vk::raii::DescriptorSet _descriptorSet{ nullptr };
	vk::DeviceSize _modelTransformsBufferOffset = 0;
	vk::raii::Buffer _modelTransformsBuffer{ nullptr };
	jjyou::vk::VmaAllocation _modelTransformsBufferMemory{ nullptr };
	void* _modelTransformsBufferMemoryMappedAddress = nullptr;
	std::uint32_t _numModelTransforms = 0;

};

/***********************************************************************
 * @class	RayCastingDescriptorSet
 * @brief	Descriptor set 1 in the ray casting shader.
 ***********************************************************************/
class RayCastingDescriptorSet {

public:

	/***********************************************************************
	 * @class	RayCastingParameters
	 * @brief	Binding 0 uniform buffer in the ray casting shaders.
	 ***********************************************************************/
	struct RayCastingParameters {
		float fx, fy, cx, cy;
		jjyou::glsl::mat4 invView;
		float minDepth;
		float maxDepth;
		float invalidDepth;
		float marchingStep;
	};

	/** @brief	Construct an empty descriptor set in invalid state.
	  */
	RayCastingDescriptorSet(std::nullptr_t) {}

	/** @brief	Construct a descriptor set given the engine and the fusion.
	  */
	RayCastingDescriptorSet(
		const Engine& engine_,
		const KinectFusion& kinectFusion_
	);

	/** @brief	Copy constructor is disabled.
	  */
	RayCastingDescriptorSet(const RayCastingDescriptorSet&) = delete;

	/** @brief	Move constructor.
	  */
	RayCastingDescriptorSet(RayCastingDescriptorSet&& other_) = default;

	/** @brief	Copy assignment is disabled.
	  */
	RayCastingDescriptorSet& operator=(const RayCastingDescriptorSet&) = delete;

	/** @brief	Move assignment.
	  */
	RayCastingDescriptorSet& operator=(RayCastingDescriptorSet&& other_) noexcept {
		if (this != &other_) {
			this->_pEngine = other_._pEngine;
			this->_descriptorSetLayout = other_._descriptorSetLayout;
			this->_descriptorSet = std::move(other_._descriptorSet);
			this->_rayCastingParametersBuffer = std::move(other_._rayCastingParametersBuffer);
			this->_rayCastingParametersBufferMemory = std::move(other_._rayCastingParametersBufferMemory);
			this->_rayCastingParametersBufferMemoryMappedAddress = other_._rayCastingParametersBufferMemoryMappedAddress;
		}
		return *this;
	}

	/** @brief	Destructor.
	  */
	~RayCastingDescriptorSet(void) = default;

	/** @brief	Get the descriptor set.
	  */
	const vk::raii::DescriptorSet& descriptorSet(void) const { return this->_descriptorSet; }

	/** @brief	Get the mapped address for RayCastingParameters (binding 0).
	  */
	RayCastingParameters& rayCastingParameters(void) const { return *reinterpret_cast<RayCastingDescriptorSet::RayCastingParameters*>(this->_rayCastingParametersBufferMemoryMappedAddress); }

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

	/** @brief	Get the descriptor set layout.
	  */
	vk::DescriptorSetLayout descriptorSetLayout(void) const {
		return this->_descriptorSetLayout;
	}

	/** @brief	Create the descriptor set layout.
	  */
	static vk::raii::DescriptorSetLayout createDescriptorSetLayout(const vk::raii::Device& device_) {
		std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
			vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
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
	vk::raii::DescriptorSet _descriptorSet{ nullptr };
	vk::raii::Buffer _rayCastingParametersBuffer{ nullptr };
	jjyou::vk::VmaAllocation _rayCastingParametersBufferMemory{ nullptr };
	void* _rayCastingParametersBufferMemoryMappedAddress = nullptr;

};

/***********************************************************************
 * @class	FusionDescriptorSet
 * @brief	Descriptor set 1 in the fusion shader.
 ***********************************************************************/
class FusionDescriptorSet {

public:

	/***********************************************************************
	 * @class	FusionParameters
	 * @brief	Binding 0 uniform buffer in the fusion shaders.
	 ***********************************************************************/
	struct FusionParameters {
		float fx, fy, cx, cy;
		jjyou::glsl::mat4 view;
		jjyou::glsl::vec3 viewPos;
		int truncationWeight;
		float minDepth;
		float maxDepth;
		float invalidDepth;
	};

	/** @brief	Construct an empty descriptor set in invalid state.
	  */
	FusionDescriptorSet(std::nullptr_t) {}

	/** @brief	Construct a descriptor set given the engine and the fusion.
	  */
	FusionDescriptorSet(
		const Engine& engine_,
		const KinectFusion& kinectFusion_
	);

	/** @brief	Copy constructor is disabled.
	  */
	FusionDescriptorSet(const FusionDescriptorSet&) = delete;

	/** @brief	Move constructor.
	  */
	FusionDescriptorSet(FusionDescriptorSet&& other_) = default;

	/** @brief	Copy assignment is disabled.
	  */
	FusionDescriptorSet& operator=(const FusionDescriptorSet&) = delete;

	/** @brief	Move assignment.
	  */
	FusionDescriptorSet& operator=(FusionDescriptorSet&& other_) noexcept {
		if (this != &other_) {
			this->_pEngine = other_._pEngine;
			this->_descriptorSetLayout = other_._descriptorSetLayout;
			this->_descriptorSet = std::move(other_._descriptorSet);
			this->_fusionParametersBuffer = std::move(other_._fusionParametersBuffer);
			this->_fusionParametersBufferMemory = std::move(other_._fusionParametersBufferMemory);
			this->_fusionParametersBufferMemoryMappedAddress = other_._fusionParametersBufferMemoryMappedAddress;
		}
		return *this;
	}

	/** @brief	Destructor.
	  */
	~FusionDescriptorSet(void) = default;

	/** @brief	Get the descriptor set.
	  */
	const vk::raii::DescriptorSet& descriptorSet(void) const { return this->_descriptorSet; }

	/** @brief	Get the mapped address for FusionParameters (binding 0).
	  */
	FusionParameters& fusionParameters(void) const { return *reinterpret_cast<FusionDescriptorSet::FusionParameters*>(this->_fusionParametersBufferMemoryMappedAddress); }

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

	/** @brief	Get the descriptor set layout.
	  */
	vk::DescriptorSetLayout descriptorSetLayout(void) const {
		return this->_descriptorSetLayout;
	}

	/** @brief	Create the descriptor set layout.
	  */
	static vk::raii::DescriptorSetLayout createDescriptorSetLayout(const vk::raii::Device& device_) {
		std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
			vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
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
	vk::raii::DescriptorSet _descriptorSet{ nullptr };
	vk::raii::Buffer _fusionParametersBuffer{ nullptr };
	jjyou::vk::VmaAllocation _fusionParametersBufferMemory{ nullptr };
	void* _fusionParametersBufferMemoryMappedAddress = nullptr;

};

/***********************************************************************
 * @class	ICPDescriptorSet
 * @brief	Descriptor set 2 in `buildLinearFunction.comp` and set 0 in
 *			`buildLinearFunctionReduction.comp`.
 ***********************************************************************/
class ICPDescriptorSet {

public:

	/***********************************************************************
	 * @class	ICPParameters
	 * @brief	Binding 0 uniform buffer in the shaders.
	 ***********************************************************************/
	struct ICPParameters {
		jjyou::glsl::mat4 frameInvView;		//!< The inverse of the current view matrix of the frame data.
		jjyou::glsl::mat4 modelView;		//!< The current view matrix of the model data.
		float fx, fy, cx, cy;				//!< The camera projection parameters of the model data.
		float distanceThreshold;			//!< Distance threshold used in projective correspondence search.
		float angleThreshold;				//!< Angle threshold used in projective correspondence search.
	};

	/***********************************************************************
	 * @class	ReductionResult
	 * @brief	Binding 2 uniform buffer in the shaders.
	 ***********************************************************************/
	struct ReductionResult {
		float data[27];
	};

	/** @brief	Construct an empty descriptor set in invalid state.
	  */
	ICPDescriptorSet(std::nullptr_t) {}

	/** @brief	Construct a descriptor set given the engine and the fusion.
	  */
	ICPDescriptorSet(
		const Engine& engine_,
		const KinectFusion& kinectFusion_,
		std::uint32_t globalSumBufferLength_
	);

	/** @brief	Copy constructor is disabled.
	  */
	ICPDescriptorSet(const ICPDescriptorSet&) = delete;

	/** @brief	Move constructor.
	  */
	ICPDescriptorSet(ICPDescriptorSet&& other_) = default;

	/** @brief	Copy assignment is disabled.
	  */
	ICPDescriptorSet& operator=(const ICPDescriptorSet&) = delete;

	/** @brief	Move assignment.
	  */
	ICPDescriptorSet& operator=(ICPDescriptorSet&& other_) noexcept {
		if (this != &other_) {
			this->_pEngine = other_._pEngine;
			this->_pKinectFusion = other_._pKinectFusion;
			this->_descriptorSetLayout = other_._descriptorSetLayout;
			this->_descriptorSet = std::move(other_._descriptorSet);
			this->_icpParametersBuffer = std::move(other_._icpParametersBuffer);
			this->_icpParametersBufferMemory = std::move(other_._icpParametersBufferMemory);
			this->_icpParametersBufferMemoryMappedAddress = other_._icpParametersBufferMemoryMappedAddress;
			this->_globalSumBufferSize = other_._globalSumBufferSize;
			this->_globalSumBufferBuffer = std::move(other_._globalSumBufferBuffer);
			this->_globalSumBufferBufferMemory = std::move(other_._globalSumBufferBufferMemory);
			this->_reductionResultBuffer = std::move(other_._reductionResultBuffer);
			this->_reductionResultBufferMemory = std::move(other_._reductionResultBufferMemory);
			this->_reductionResultBufferMemoryMappedAddress = other_._reductionResultBufferMemoryMappedAddress;
		}
		return *this;
	}

	/** @brief	Destructor.
	  */
	~ICPDescriptorSet(void) = default;

	/** @brief	Get the descriptor set.
	  */
	const vk::raii::DescriptorSet& descriptorSet(void) const { return this->_descriptorSet; }

	/** @brief	Get the mapped address for ICPParameters (binding 0).
	  */
	ICPParameters& icpParameters(void) const { return *reinterpret_cast<ICPDescriptorSet::ICPParameters*>(this->_icpParametersBufferMemoryMappedAddress); }

	/** @brief	Get the mapped address for ReductionResult (binding 2).
	  */
	ReductionResult& reductionResult(void) const { return *reinterpret_cast<ICPDescriptorSet::ReductionResult*>(this->_reductionResultBufferMemoryMappedAddress); }

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

	/** @brief	Get the descriptor set layout.
	  */
	vk::DescriptorSetLayout descriptorSetLayout(void) const {
		return this->_descriptorSetLayout;
	}

	/** @brief	Get the Vulkan buffer of GlobalSumBuffer.
	  * 
	  *			You may wish to insert buffer memory barriers for this buffer.
	  */
	const vk::raii::Buffer& globalSumBufferBuffer(void) const {
		return this->_globalSumBufferBuffer;
	}

	/** @brief	Create the descriptor set layout.
	  */
	static vk::raii::DescriptorSetLayout createDescriptorSetLayout(const vk::raii::Device& device_) {
		std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
			vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eCompute)
			.setPImmutableSamplers(nullptr),
			vk::DescriptorSetLayoutBinding()
			.setBinding(1)
			.setDescriptorType(vk::DescriptorType::eStorageBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eCompute)
			.setPImmutableSamplers(nullptr),
			vk::DescriptorSetLayoutBinding()
			.setBinding(2)
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
	vk::DescriptorSetLayout _descriptorSetLayout{ nullptr }; // Descriptor set layout should be owned by the engine.
	vk::raii::DescriptorSet _descriptorSet{ nullptr };
	vk::raii::Buffer _icpParametersBuffer{ nullptr };
	jjyou::vk::VmaAllocation _icpParametersBufferMemory{ nullptr };
	void* _icpParametersBufferMemoryMappedAddress = nullptr;
	vk::DeviceSize _globalSumBufferSize = 0;
	vk::raii::Buffer _globalSumBufferBuffer{ nullptr };
	jjyou::vk::VmaAllocation _globalSumBufferBufferMemory{ nullptr };
	vk::raii::Buffer _reductionResultBuffer{ nullptr };
	jjyou::vk::VmaAllocation _reductionResultBufferMemory{ nullptr };
	void* _reductionResultBufferMemoryMappedAddress = nullptr;

	void _createUniformBufferBinding0(void);
	void _createStorageBufferBinding1(void);
	void _createStorageBufferBinding2(void);

};