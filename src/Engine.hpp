#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <jjyou/vk/Vulkan.hpp>
#include "Window.hpp"
#include "Primitives.hpp"
#include "Texture.hpp"
#include "DescriptorSet.hpp"

/***********************************************************************
 * @class	Engine
 * @brief	Vulkan Engine class that creates and manages most of the
 *			Vulkan resources and provides API for graphics rendering.
 *
 *			This class does not create or manage the resources related
 *			to KinectFusion. It is only responsible for Vulkan initialization
 *			and graphics rendering.
 ***********************************************************************/
class Engine {

public:

	/** @brief	Number of frames in flight.
	  */
	static inline constexpr std::uint32_t NUM_FRAMES_IN_FLIGHT = 2;
	
	/** @brief	Constructor.
	  */
	Engine(bool headlessMode_, bool debugMode_);

	/** @brief	Disable copy/move constructor/assignment.
	  */
	Engine(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine& operator=(Engine&&) = delete;

	/** @brief	Destructor.
	  */
	~Engine(void);

	/** @brief	Getters.
	  */
	bool headlessMode(void) const { return this->_headlessMode; }
	bool debugMode(void) const { return this->_debugMode; }
	const jjyou::vk::Context& context(void) const { return this->_context; }
	const jjyou::vk::VmaAllocator& allocator(void) const { return this->_allocator; }
	const Window& window(void) const { return this->_window; }
	const vk::raii::CommandPool& commandPool(jjyou::vk::Context::QueueType queueType_) const { return this->_commandPools[queueType_]; }
	const vk::raii::CommandPool& commandPool(std::size_t queueType_) const { return this->_commandPools[queueType_]; }
	const vk::raii::DescriptorPool& descriptorPool(void) const { return this->_descriptorPool; }
	const vk::raii::DescriptorSetLayout& viewLevelDescriptorSetLayout(void) const { return this->_viewLevelDescriptorSetLayout; }
	const vk::raii::DescriptorSetLayout& instanceLevelDescriptorSetLayout(void) const { return this->_instanceLevelDescriptorSetLayout; }
	const vk::raii::DescriptorSetLayout& surfaceSamplerDescriptorSetLayout(MaterialType _materialType) const { return this->_surfaceSamplerDescriptorSetLayouts[_materialType]; }
	const vk::raii::DescriptorSetLayout& surfaceStorageDescriptorSetLayout(MaterialType _materialType) const { return this->_surfaceStorageDescriptorSetLayouts[_materialType]; }

	/** @brief	Create a `Primitives` instance.
	  */
	template <MaterialType _materialType, PrimitiveType _primitiveType>
	Primitives<_materialType, _primitiveType> createPrimitives(void) {
		return Primitives<_materialType, _primitiveType>(*this);
	}

	/** @brief	Create a `Surface` instance.
	  */
	template <MaterialType _materialType>
	Surface<_materialType> createSurface(void) {
		return Surface<_materialType>(*this);
	}

	/** @brief	Prepare a new frame. Call this function before rendering.
	  * @return	The Vulkan result of acquiring a new image from the swapchain.
	  *			If the result is `vk::Result::eErrorOutOfDateKHR`, you should skip
	  *			this frame.
	  */
	vk::Result prepareFrame(void);

	/** @brief	Add a Primitives to draw.
	  */
	template<MaterialType materialType, PrimitiveType primitiveType>
	void drawPrimitives(
		const Primitives<materialType, primitiveType>& primitives_,
		const jjyou::glsl::mat4& modelMatrix_
	) {
		this->_getPrimitivesToDraw<materialType, primitiveType>().emplace_back(
			&primitives_,
			modelMatrix_,
			jjyou::glsl::transpose(jjyou::glsl::inverse(modelMatrix_))
		);
	}

	/** @brief	Add a Surface to draw.
	  */
	template<MaterialType materialType>
	void drawSurface(
		const Surface<materialType>& surface_
	) {
		this->_getSurfacesToDraw<materialType>().push_back(&surface_);
	}

	/** @brief	Record the command buffer. Call this function after sending all instances
	  *			to draw to the engine via `Engine::drawPrimitives` and `Engine::drawSurface`.
	  */
	void recordCommandbuffer(void) const;

	/** @brief	Present the current frame. Call this function after recording the command buffer.
	  */
	vk::Result presentFrame(void);

	/** @brief	Wait all queues to be idle.
	  */
	void waitIdle(void) const;

private:

	bool _headlessMode = false;

	bool _debugMode = true;
	
	jjyou::vk::Context _context{ nullptr };

	jjyou::vk::VmaAllocator _allocator{ nullptr };
	
	Window _window{ nullptr };
	
	std::array<vk::raii::CommandPool, jjyou::vk::Context::NumQueueTypes> _commandPools{ { vk::raii::CommandPool{nullptr},vk::raii::CommandPool{nullptr},vk::raii::CommandPool{nullptr} } };
	
	jjyou::vk::Swapchain _swapchain{ nullptr };

	vk::raii::RenderPass _renderPass{ nullptr };

	Texture2D _depthImage{ nullptr };

	std::vector<vk::raii::Framebuffer> _framebuffers{};
	
	// View level descriptor set layout, including camera parameters
	vk::raii::DescriptorSetLayout _viewLevelDescriptorSetLayout{ nullptr };

	// Instance level descriptor set layout, including model and normal matrices
	vk::raii::DescriptorSetLayout _instanceLevelDescriptorSetLayout{ nullptr };

	// Descriptor set layouts for drawing a quad to display a surface
	std::array<vk::raii::DescriptorSetLayout, MaterialType::NumMaterialTypes> _surfaceSamplerDescriptorSetLayouts{ { vk::raii::DescriptorSetLayout{nullptr}, vk::raii::DescriptorSetLayout{nullptr} } };
	std::array<vk::raii::DescriptorSetLayout, MaterialType::NumMaterialTypes> _surfaceStorageDescriptorSetLayouts{ { vk::raii::DescriptorSetLayout{nullptr}, vk::raii::DescriptorSetLayout{nullptr} } };

	// Descriptor pool. Over-allocated for simplicity.
	vk::raii::DescriptorPool _descriptorPool{ nullptr };

	// Pipelines layouts for drawing primitives
	std::array<vk::raii::PipelineLayout, MaterialType::NumMaterialTypes> _primitivePipelineLayouts{ { vk::raii::PipelineLayout{nullptr}, vk::raii::PipelineLayout{nullptr} } };

	// Pipeline layouts for drawing a quad to display a surface
	std::array<vk::raii::PipelineLayout, MaterialType::NumMaterialTypes> _surfacePipelineLayouts{ { vk::raii::PipelineLayout{nullptr}, vk::raii::PipelineLayout{nullptr} } };

	// Pipelines for drawing scene primitives
	std::array<std::array<vk::raii::Pipeline, PrimitiveType::NumPrimitiveTypes>, MaterialType::NumMaterialTypes> _primitivePipelines{ {
		{ { vk::raii::Pipeline{nullptr}, vk::raii::Pipeline{nullptr}, vk::raii::Pipeline{nullptr} } },
		{ { vk::raii::Pipeline{nullptr}, vk::raii::Pipeline{nullptr}, vk::raii::Pipeline{nullptr} } }
	} };

	// Pipelines for drawing a quad to display a surface
	std::array<vk::raii::Pipeline, MaterialType::NumMaterialTypes> _surfacePipelines{ { vk::raii::Pipeline{nullptr}, vk::raii::Pipeline{nullptr} } };

	// Frame data
	struct _FrameData {
		vk::raii::Fence inFlightFence{ nullptr };
		vk::raii::Semaphore imageAvailableSemaphore{ nullptr };
		vk::raii::Semaphore renderFinishedSemaphore{ nullptr };
		vk::raii::CommandBuffer graphicsCommandBuffer{ nullptr };
		ViewLevelDescriptorSet viewLevelDescriptorSet{ nullptr };
		InstanceLevelDescriptorSet instanceLevelDescriptorSet{ nullptr };
	};
	std::array<_FrameData, static_cast<std::size_t>(Engine::NUM_FRAMES_IN_FLIGHT)> _framesInFlight;
	std::uint32_t _swapchainImageIndex = 0;
	std::uint32_t _frameIndex = 0;
	const _FrameData& _activeFrameData(void) const { return this->_framesInFlight[static_cast<std::size_t>(this->_frameIndex)]; }
	const vk::raii::Framebuffer& _activeFramebuffer(void) const { return this->_framebuffers[static_cast<std::size_t>(this->_swapchainImageIndex)]; }

	// Render resources
	/// Primitives
	template<MaterialType _materialType, PrimitiveType _primitiveType>
	struct _PrimitivesToDraw {
		const Primitives<_materialType, _primitiveType>* pPrimitives = nullptr;
		jjyou::glsl::mat4 modelMatrix{ 1.0f };
		jjyou::glsl::mat4 normalMatrix{ 1.0f };
	};
	template<MaterialType _materialType, PrimitiveType _primitiveType>
	std::vector<_PrimitivesToDraw<_materialType, _primitiveType>>&
		_getPrimitivesToDraw(void);
	template<MaterialType _materialType, PrimitiveType _primitiveType>
	const std::vector <_PrimitivesToDraw<_materialType, _primitiveType>>&
		_getPrimitivesToDraw(void) const;
	std::vector<_PrimitivesToDraw<MaterialType::Simple, PrimitiveType::Point>> _simplePoints{};
	std::vector<_PrimitivesToDraw<MaterialType::Simple, PrimitiveType::Line>> _simpleLines{};
	std::vector<_PrimitivesToDraw<MaterialType::Simple, PrimitiveType::Triangle>> _simpleTriangles{};
	std::vector<_PrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Point>> _lambertianPoints{};
	std::vector<_PrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Line>> _lambertianLines{};
	std::vector<_PrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Triangle>> _lambertianTriangles{};
	/// Surfaces
	template<MaterialType _materialType>
	std::vector<const Surface<_materialType>*>&
		_getSurfacesToDraw(void);
	template<MaterialType _materialType>
	const std::vector<const Surface<_materialType>*>&
		_getSurfacesToDraw(void) const;
	std::vector<const Surface<MaterialType::Simple>*> _simpleSurfaces{};
	std::vector<const Surface<MaterialType::Lambertian>*> _lambertianSurfaces{};

	// Initialization functions
	void _createContext(void);
	void _createAllocator(void);
	void _createCommandPools(void);
	void _createSwapchain(void);
	void _createRenderPass(void);
	void _createDepthStencil(void);
	void _createFramebuffers(void);
	void _createDescriptorSetLayouts(void);
	void _createDescriptorPool(void);
	void _initImGui(void);
	void _createPipelineLayouts(void);
	void _createPipelines(void);
	void _createFrameData(void);
	void _resizeRenderResources(void);
};

//// Simple points
template <>
inline std::vector<Engine::_PrimitivesToDraw<MaterialType::Simple, PrimitiveType::Point>>&
Engine::_getPrimitivesToDraw<MaterialType::Simple, PrimitiveType::Point>(void) { return this->_simplePoints; };
template <>
inline const std::vector<Engine::_PrimitivesToDraw<MaterialType::Simple, PrimitiveType::Point>>&
Engine::_getPrimitivesToDraw<MaterialType::Simple, PrimitiveType::Point>(void) const { return this->_simplePoints; };
//// Simple lines
template <>
inline std::vector<Engine::_PrimitivesToDraw<MaterialType::Simple, PrimitiveType::Line>>&
Engine::_getPrimitivesToDraw<MaterialType::Simple, PrimitiveType::Line>(void) { return this->_simpleLines; };
template <>
inline const std::vector<Engine::_PrimitivesToDraw<MaterialType::Simple, PrimitiveType::Line>>&
Engine::_getPrimitivesToDraw<MaterialType::Simple, PrimitiveType::Line>(void) const { return this->_simpleLines; };
//// Simple triangles
template <>
inline std::vector<Engine::_PrimitivesToDraw<MaterialType::Simple, PrimitiveType::Triangle>>&
Engine::_getPrimitivesToDraw<MaterialType::Simple, PrimitiveType::Triangle>(void) { return this->_simpleTriangles; };
template <>
inline const std::vector<Engine::_PrimitivesToDraw<MaterialType::Simple, PrimitiveType::Triangle>>&
Engine::_getPrimitivesToDraw<MaterialType::Simple, PrimitiveType::Triangle>(void) const { return this->_simpleTriangles; };
//// Lambertian points
template <>
inline std::vector<Engine::_PrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Point>>&
Engine::_getPrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Point>(void) { return this->_lambertianPoints; };
template <>
inline const std::vector<Engine::_PrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Point>>&
Engine::_getPrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Point>(void) const { return this->_lambertianPoints; };
//// Lambertian lines
template <>
inline std::vector<Engine::_PrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Line>>&
Engine::_getPrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Line>(void) { return this->_lambertianLines; };
template <>
inline const std::vector<Engine::_PrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Line>>&
Engine::_getPrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Line>(void) const { return this->_lambertianLines; };
//// Lambertian triangles
template <>
inline std::vector<Engine::_PrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Triangle>>&
Engine::_getPrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Triangle>(void) { return this->_lambertianTriangles; };
template <>
inline const std::vector<Engine::_PrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Triangle>>&
Engine::_getPrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Triangle>(void) const { return this->_lambertianTriangles; };
//// Simple surfaces
template <>
inline std::vector<const Surface<MaterialType::Simple>*>&
Engine::_getSurfacesToDraw(void) { return this->_simpleSurfaces; };
template <>
inline const std::vector<const Surface<MaterialType::Simple>*>&
Engine::_getSurfacesToDraw(void) const { return this->_simpleSurfaces; };
//// Lambertian surfaces
template <>
inline std::vector<const Surface<MaterialType::Lambertian>*>&
Engine::_getSurfacesToDraw(void) { return this->_lambertianSurfaces; };
template <>
inline const std::vector<const Surface<MaterialType::Lambertian>*>&
Engine::_getSurfacesToDraw(void) const { return this->_lambertianSurfaces; };