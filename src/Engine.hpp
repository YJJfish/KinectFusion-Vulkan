#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <jjyou/vk/Vulkan.hpp>
#include "Window.hpp"

class Engine {

public:

	static inline constexpr std::uint32_t NUM_FRAMES_IN_FLIGHT = 2;

	Engine(bool headlessMode_, bool debugMode_);
	Engine(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(const Engine&) = delete;
	Engine& operator=(Engine&&) = delete;
	~Engine(void);

	bool headlessMode(void) const { return this->_headlessMode; }
	bool debugMode(void) const { return this->_debugMode; }
	const jjyou::vk::Context& context(void) const { return this->_context; }
	const jjyou::vk::VmaAllocator& allocator(void) const { return this->_allocator; }
	const Window& window(void) const { return this->_window; }
	const vk::raii::CommandPool& commandPool(jjyou::vk::Context::QueueType queueType_) const { return this->_commandPools[queueType_]; }
	const vk::raii::CommandPool& commandPool(std::size_t queueType_) const { return this->_commandPools[queueType_]; }
	const vk::raii::DescriptorSetLayout& viewLevelDescriptorSetLayout(void) const { return this->_viewLevelDescriptorSetLayout; }
	const vk::raii::DescriptorSetLayout& instanceLevelDescriptorSetLayout(void) const { return this->_instanceLevelDescriptorSetLayout; }
	const vk::raii::DescriptorSetLayout& imageDescriptorSetLayout(void) const { return this->_imageDescriptorSetLayout; }

	struct FrameData {
		vk::raii::Fence inFlightFence{ nullptr };
		vk::raii::Semaphore imageAvailableSemaphore{ nullptr };
		vk::raii::Semaphore renderFinishedSemaphore{ nullptr };
		std::array<vk::raii::CommandBuffer, jjyou::vk::Context::NumQueueTypes> commandBuffer{ { vk::raii::CommandBuffer{nullptr},vk::raii::CommandBuffer{nullptr},vk::raii::CommandBuffer{nullptr} } };
	};
	vk::Result prepareFrame(void);
	const FrameData& activeFrameData(void) const { return this->_framesInFlight[static_cast<std::size_t>(this->_frameIndex)]; }
	const vk::raii::Framebuffer& activeFramebuffer(void) const { return this->_framebuffers[static_cast<std::size_t>(this->_swapchainImageIndex)]; }
	vk::Result presentFrame(void);

	void recordCommandBuffersDemo(void);


private:

	bool _headlessMode;
	bool _debugMode;
	
	jjyou::vk::Context _context{ nullptr };
	jjyou::vk::VmaAllocator _allocator{ nullptr };
	
	Window _window{};
	
	std::array<vk::raii::CommandPool, jjyou::vk::Context::NumQueueTypes> _commandPools{ { vk::raii::CommandPool{nullptr},vk::raii::CommandPool{nullptr},vk::raii::CommandPool{nullptr} } };
	
	jjyou::vk::Swapchain _swapchain{ nullptr };

	vk::raii::RenderPass _renderPass{ nullptr };

	vk::raii::Image _depthImage{ nullptr };
	jjyou::vk::VmaAllocation _depthImageMemory{ nullptr };
	vk::raii::ImageView _depthImageView{ nullptr };

	std::vector<vk::raii::Framebuffer> _framebuffers;

	vk::raii::DescriptorSetLayout _viewLevelDescriptorSetLayout{ nullptr };
	vk::raii::DescriptorSetLayout _instanceLevelDescriptorSetLayout{ nullptr };
	vk::raii::DescriptorSetLayout _imageDescriptorSetLayout{ nullptr };

	vk::raii::DescriptorPool _descriptorPool{ nullptr };

	// Pipeline layout for drawing simple primitives
	vk::raii::PipelineLayout _simplePipelineLayout{ nullptr };

	// Pipeline layout for drawing lambertian primitives
	vk::raii::PipelineLayout _lambertianPipelineLayout{ nullptr };

	// Pipeline layout for drawing a quad to display images
	vk::raii::PipelineLayout _imagePipelineLayout{ nullptr };

	// Pipeline for drawing simple lines
	vk::raii::Pipeline _simpleLinePipeline{ nullptr };

	// Pipeline for drawing lambertian triangles
	vk::raii::Pipeline _lambertianTrianglePipeline{ nullptr };

	// Pipeline for drawing a quad to display images
	vk::raii::Pipeline _imagePipeline{ nullptr };

	std::array<FrameData, static_cast<std::size_t>(Engine::NUM_FRAMES_IN_FLIGHT)> _framesInFlight;
	std::uint32_t _swapchainImageIndex = 0;
	std::uint32_t _frameIndex = 0;

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