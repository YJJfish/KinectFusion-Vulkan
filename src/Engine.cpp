#include "Engine.hpp"
#include <iostream>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <exception>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

Engine::Engine(bool headlessMode_, bool debugMode_) : _headlessMode(headlessMode_), _debugMode(debugMode_) {
	this->_createContext();
	this->_createAllocator();
	this->_createCommandPools();
	this->_createSwapchain();
	this->_createRenderPass();
	this->_createDepthStencil();
	this->_createFramebuffers();
	this->_createDescriptorSetLayouts();
	this->_createDescriptorPool();
	this->_initImGui();
	this->_createPipelineLayouts();
	this->_createPipelines();
	this->_createFrameData();
}

Engine::~Engine(void) {
	for (std::size_t queueType = 0; queueType < jjyou::vk::Context::NumQueueTypes; ++queueType)
		this->_context.queue(queueType)->waitIdle();
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
}

vk::Result Engine::prepareFrame(void) {
	vk::Result waitFenceResult = this->_context.device().waitForFences({ *this->activeFrameData().inFlightFence }, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
	if (waitFenceResult != vk::Result::eSuccess) {
		throw std::runtime_error("[Engine] Error occurred when waiting for the frame fence.");
	}
	vk::Result acquireImageResult{};
	std::tie(acquireImageResult, this->_swapchainImageIndex) = this->_swapchain.swapchain().acquireNextImage(UINT64_MAX, *this->activeFrameData().imageAvailableSemaphore, nullptr);
	if (acquireImageResult == vk::Result::eErrorOutOfDateKHR) {
		this->_resizeRenderResources();
		return vk::Result::eErrorOutOfDateKHR;
	}
	else if (acquireImageResult != vk::Result::eSuccess && acquireImageResult != vk::Result::eSuboptimalKHR) {
		throw std::runtime_error("[Engine] Failed to acquire the image from swapchain.");
	}
	this->_context.device().resetFences({ *this->activeFrameData().inFlightFence });
	for (std::size_t queueType = 0; queueType < jjyou::vk::Context::NumQueueTypes; ++queueType)
		this->activeFrameData().commandBuffer[queueType].reset();

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	return acquireImageResult;
}

vk::Result Engine::presentFrame(void) {
	vk::PresentInfoKHR presentInfo(
		1, &*this->activeFrameData().renderFinishedSemaphore,
		1, &*this->_swapchain.swapchain(), &this->_swapchainImageIndex,
		nullptr
	);
	;
	vk::Result presentResult{};
	try {
		presentResult = this->_context.queue(jjyou::vk::Context::QueueType::Main)->presentKHR(presentInfo);
	}
	catch (const vk::OutOfDateKHRError& e) {
		presentResult = vk::Result::eErrorOutOfDateKHR;
	}
	if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || this->_window.getAndResetFramebufferResized()) {
		this->_resizeRenderResources();
	}
	else if (presentResult != vk::Result::eSuccess) {
		throw std::runtime_error("[Engine] Failed to present the image in swapchain.");
	}
	this->_frameIndex = (this->_frameIndex + 1) % Engine::NUM_FRAMES_IN_FLIGHT;
	return presentResult;
}

void Engine::recordCommandBuffersDemo(void) {
	ImGui::ShowDemoWindow(nullptr);
	ImGui::Render();
	ImDrawData* imDrawData = ImGui::GetDrawData();
	vk::CommandBufferBeginInfo commandBufferBeginInfo(
		vk::CommandBufferUsageFlags(0),
		nullptr
	);
	this->activeFrameData().commandBuffer[jjyou::vk::Context::QueueType::Main].begin(commandBufferBeginInfo);
	std::vector<vk::ClearValue> clearValues = {
		vk::ClearValue(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)),
		vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0))
	};
	vk::RenderPassBeginInfo renderPassBeginInfo(
		*this->_renderPass,
		*this->activeFramebuffer(),
		vk::Rect2D(vk::Offset2D(0, 0), this->_swapchain.extent()),
		clearValues
	);
	this->activeFrameData().commandBuffer[jjyou::vk::Context::QueueType::Main].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
	ImGui_ImplVulkan_RenderDrawData(imDrawData, *this->activeFrameData().commandBuffer[jjyou::vk::Context::QueueType::Main]);
	this->activeFrameData().commandBuffer[jjyou::vk::Context::QueueType::Main].endRenderPass();
	this->activeFrameData().commandBuffer[jjyou::vk::Context::QueueType::Main].end();
	vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo(
		1, &*this->activeFrameData().imageAvailableSemaphore, &waitStage,
		1, &*this->activeFrameData().commandBuffer[jjyou::vk::Context::QueueType::Main],
		1, &*this->activeFrameData().renderFinishedSemaphore
	);
	this->_context.queue(jjyou::vk::Context::QueueType::Main)->submit(submitInfo, *this->activeFrameData().inFlightFence);
}


void Engine::_createContext(void) {
	// Create glfw window
	if (!this->_headlessMode)
		this->_window.createWindow(800, 600, "KinectFusion-Vulkan");
	jjyou::vk::ContextBuilder contextBuilder;
	// Instance
	contextBuilder
		.enableValidation(this->_debugMode)
		.headless(this->_headlessMode)
		.applicationName("KinectFusion-Vulkan")
		.applicationVersion(0U, 1U, 0U, 0U)
		.engineName("KinectFusion-Vulkan")
		.engineVersion(0U, 1U, 0U, 0U)
		.apiVersion(0U, 1U, 0U, 0U);
	if (this->_debugMode)
		contextBuilder.useDefaultDebugUtilsMessenger();
	if (!this->_headlessMode) {
		std::vector<const char*> instanceExtensions = Window::getRequiredInstanceExtensions();
		contextBuilder.enableInstanceExtensions(instanceExtensions.begin(), instanceExtensions.end());
	}
	contextBuilder.buildInstance(this->_context);
	// Physical device
	contextBuilder.requestPhysicalDeviceType(vk::PhysicalDeviceType::eDiscreteGpu);
	if (!this->_headlessMode) {
		this->_window.createSurface(this->_context.instance());
		contextBuilder.addSurface(*this->_window.surface());
	}
	contextBuilder.selectPhysicalDevice(this->_context);
	// Device
	contextBuilder.buildDevice(this->_context);
	// Check queue support. Require all types of queues (main, compute, transfer).
	for (std::size_t queueType = 0; queueType < jjyou::vk::Context::NumQueueTypes; ++queueType)
		if (!this->_context.queueFamilyIndex(queueType).has_value() || !this->_context.queue(queueType).has_value()) {
			throw std::runtime_error("[Engine] GPU does not support required queues.");
		}
}

void Engine::_createAllocator(void) {
	VmaAllocatorCreateInfo vmaAllocatorCreateInfo{
		.flags = VmaAllocatorCreateFlags(0U),
		.physicalDevice = *this->_context.physicalDevice(),
		.device = *this->_context.device(),
		.preferredLargeHeapBlockSize = VkDeviceSize(0),
		.pAllocationCallbacks = nullptr,
		.pDeviceMemoryCallbacks = nullptr,
		.pHeapSizeLimit = nullptr,
		.pVulkanFunctions = nullptr,
		.instance = *this->_context.instance(),
		.vulkanApiVersion = VK_API_VERSION_1_0
	};
	this->_allocator = jjyou::vk::VmaAllocator(vmaAllocatorCreateInfo);
}

void Engine::_createCommandPools(void) {
	for (std::size_t queueType = 0; queueType < jjyou::vk::Context::NumQueueTypes; ++queueType)
		this->_commandPools[queueType] = vk::raii::CommandPool(this->_context.device(),
			vk::CommandPoolCreateInfo(
				vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				*this->_context.queueFamilyIndex(queueType)
			)
		);
}

void Engine::_createSwapchain(void) {
	jjyou::vk::SwapchainBuilder builder(this->_context, *this->_window.surface());
	builder
		.requestSurfaceFormat(VkSurfaceFormatKHR{ .format = VK_FORMAT_B8G8R8A8_SRGB , .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		.requestPresentMode(::vk::PresentModeKHR::eMailbox);
	int width{}, height{};
	std::tie(width, height) = this->_window.framebufferSize();
	this->_swapchain = builder.build(vk::Extent2D(static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)), std::move(this->_swapchain));
}

void Engine::_createRenderPass(void) {
	std::vector<vk::AttachmentDescription> attachmentDescriptions = {
		// Color attachment
		vk::AttachmentDescription(
			vk::AttachmentDescriptionFlags(0U),
			this->_swapchain.surfaceFormat().format,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			(this->_headlessMode) ? vk::ImageLayout::eTransferSrcOptimal : vk::ImageLayout::ePresentSrcKHR
		),
		// Depth attachment
		vk::AttachmentDescription(
			vk::AttachmentDescriptionFlags(0U),
			vk::Format::eD32Sfloat,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		)
	};

	// Subpass
	vk::AttachmentReference subpassColorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentReference subpassDepthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	vk::SubpassDescription subpassDescription(
		vk::SubpassDescriptionFlags(0U),
		vk::PipelineBindPoint::eGraphics,
		0, nullptr,
		1, &subpassColorAttachmentRef,
		nullptr,
		&subpassDepthAttachmentRef,
		0, nullptr
	);

	// Subpass dependencies
	std::vector<vk::SubpassDependency> subpassDependencies = {
		vk::SubpassDependency(
			VK_SUBPASS_EXTERNAL,
			0,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
			vk::AccessFlagBits::eDepthStencilAttachmentWrite,
			vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead,
			vk::DependencyFlags(0U)
		),
		vk::SubpassDependency(
			VK_SUBPASS_EXTERNAL,
			0,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlags(0U),
			vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead,
			vk::DependencyFlags(0U)
		)
	};

	// Create renderpass
	vk::RenderPassCreateInfo renderPassCreateInfo(
		vk::RenderPassCreateFlags(0U),
		static_cast<uint32_t>(attachmentDescriptions.size()), attachmentDescriptions.data(),
		1, &subpassDescription,
		static_cast<uint32_t>(subpassDependencies.size()), subpassDependencies.data()
	);
	this->_renderPass = vk::raii::RenderPass(this->_context.device(), renderPassCreateInfo);
}

void Engine::_createDepthStencil(void) {
	vk::Extent2D extent = this->_swapchain.extent();
	vk::ImageCreateInfo imageCreateInfo(
		vk::ImageCreateFlags(0),
		vk::ImageType::e2D,
		vk::Format::eD32Sfloat,
		vk::Extent3D(extent, 1),
		1,
		1,
		vk::SampleCountFlagBits::e1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::SharingMode::eExclusive,
		1, &*this->_context.queueFamilyIndex(jjyou::vk::Context::QueueType::Main),
		vk::ImageLayout::eUndefined
	);
	VmaAllocationCreateInfo vmaAllocationCreateInfo{
		.flags = VmaAllocationCreateFlags(0),
		.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		.requiredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.preferredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		.memoryTypeBits = 0,
		.pool = nullptr,
		.pUserData = nullptr,
		.priority = 0.0f,
	};
	VkImage depthImage = nullptr;
	VmaAllocation depthImageMemory = nullptr;
	vmaCreateImage(*this->_allocator, reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo), &vmaAllocationCreateInfo, &depthImage, &depthImageMemory, nullptr);
	this->_depthImage = vk::raii::Image(this->_context.device(), depthImage);
	this->_depthImageMemory = jjyou::vk::VmaAllocation(this->_allocator, depthImageMemory);
	vk::ImageViewCreateInfo imageViewCreateInfo(
		vk::ImageViewCreateFlags(0),
		*this->_depthImage,
		vk::ImageViewType::e2D,
		vk::Format::eD32Sfloat,
		vk::ComponentMapping(vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity),
		vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1)
	);
	this->_depthImageView = vk::raii::ImageView(this->_context.device(), imageViewCreateInfo);
}


void Engine::_createFramebuffers(void) {
	this->_framebuffers.clear();
	this->_framebuffers.reserve(static_cast<std::size_t>(this->_swapchain.numImages()));
	for (std::uint32_t i = 0; i < this->_swapchain.numImages(); ++i) {
		std::array<vk::ImageView, 2> attachments = {
			*this->_swapchain.imageView(i),
			*this->_depthImageView
		};
		vk::FramebufferCreateInfo framebufferCreateInfo(
			vk::FramebufferCreateFlags(0),
			*this->_renderPass,
			attachments,
			this->_swapchain.extent().width,
			this->_swapchain.extent().height,
			1
		);
		this->_framebuffers.emplace_back(this->_context.device(), framebufferCreateInfo);
	}
}

void Engine::_createDescriptorSetLayouts(void) {
	// _viewLevelDescriptorSetLayout
	{
		std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
			vk::DescriptorSetLayoutBinding(
				0,
				vk::DescriptorType::eUniformBuffer,
				1,
				vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
				nullptr
			)
		};
		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			vk::DescriptorSetLayoutCreateFlags(0),
			descriptorSetLayoutBindings
		);
		this->_viewLevelDescriptorSetLayout = vk::raii::DescriptorSetLayout(this->_context.device(), descriptorSetLayoutCreateInfo);
	}

	// _instanceLevelDescriptorSetLayout
	{
		std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
			vk::DescriptorSetLayoutBinding(
				0,
				vk::DescriptorType::eUniformBufferDynamic,
				1,
				vk::ShaderStageFlagBits::eVertex,
				nullptr
			)
		};
		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			vk::DescriptorSetLayoutCreateFlags(0),
			descriptorSetLayoutBindings
		);
		this->_instanceLevelDescriptorSetLayout = vk::raii::DescriptorSetLayout(this->_context.device(), descriptorSetLayoutCreateInfo);
	}

	// _imageDescriptorSetLayout
	{
		std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
			vk::DescriptorSetLayoutBinding(
				0,
				vk::DescriptorType::eCombinedImageSampler,
				1,
				vk::ShaderStageFlagBits::eFragment,
				nullptr
			),
			vk::DescriptorSetLayoutBinding(
				1,
				vk::DescriptorType::eCombinedImageSampler,
				1,
				vk::ShaderStageFlagBits::eFragment,
				nullptr
			)
		};
		vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
			vk::DescriptorSetLayoutCreateFlags(0),
			descriptorSetLayoutBindings
		);
		this->_imageDescriptorSetLayout = vk::raii::DescriptorSetLayout(this->_context.device(), descriptorSetLayoutCreateInfo);
	}
}

void Engine::_createDescriptorPool(void) {
	std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {
		vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 1000),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1000),
		vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 1000),
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageImage, 1000),
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformTexelBuffer, 1000),
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageTexelBuffer, 1000),
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1000),
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 1000),
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1000),
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageBufferDynamic, 1000),
		vk::DescriptorPoolSize(vk::DescriptorType::eInputAttachment, 1000),
	};
	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(
		vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		1000,
		descriptorPoolSizes
	);
	this->_descriptorPool = vk::raii::DescriptorPool(this->_context.device(), descriptorPoolCreateInfo);
}

void Engine::_initImGui(void) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Load font
	io.Fonts->AddFontDefault();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(this->_window.glfwWindow(), true);
	ImGui_ImplVulkan_InitInfo initInfo = {
		.Instance = *this->_context.instance(),
		.PhysicalDevice = *this->_context.physicalDevice(),
		.Device = *this->_context.device(),
		.QueueFamily = *this->_context.queueFamilyIndex(jjyou::vk::Context::QueueType::Main),
		.Queue = **this->_context.queue(jjyou::vk::Context::QueueType::Main),
		.DescriptorPool = *this->_descriptorPool,
		.RenderPass = *this->_renderPass,
		.MinImageCount = this->_swapchain.numImages(),
		.ImageCount = this->_swapchain.numImages(),
		.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
		.PipelineCache = nullptr,
		.Subpass = 0,
		.UseDynamicRendering = false,
		// .PipelineRenderingCreateInfo = {},
		.Allocator = nullptr,
		.CheckVkResultFn = [](VkResult result) { if (result != VK_SUCCESS) throw std::runtime_error("[ImGui] Internal error."); },
		.MinAllocationSize = 0
	};
	ImGui_ImplVulkan_Init(&initInfo);
}

void Engine::_createPipelineLayouts() {
	// _simplePipelineLayout
	{
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_viewLevelDescriptorSetLayout,
			*this->_instanceLevelDescriptorSetLayout,
		};
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(0),
			descriptorSetLayouts,
			{}
		);
		this->_simplePipelineLayout = vk::raii::PipelineLayout(this->_context.device(), pipelineLayoutCreateInfo);
	}

	// _lambertianPipelineLayout
	{
		// Currently, lambertian pipeline layout is the same as simple pipeline layout.
		// However, extensions may be made to support textures, lightings, etc.
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_viewLevelDescriptorSetLayout,
			*this->_instanceLevelDescriptorSetLayout,
		};
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(0),
			descriptorSetLayouts,
			{}
		);
		this->_lambertianPipelineLayout = vk::raii::PipelineLayout(this->_context.device(), pipelineLayoutCreateInfo);
	}

	// _imagePipelineLayout
	{
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_imageDescriptorSetLayout
		};
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
			vk::PipelineLayoutCreateFlags(0),
			descriptorSetLayouts,
			{}
		);
		this->_imagePipelineLayout = vk::raii::PipelineLayout(this->_context.device(), pipelineLayoutCreateInfo);
	}
}

void Engine::_createPipelines() {

}

void Engine::_createFrameData(void) {
	std::array<std::vector<vk::raii::CommandBuffer>, jjyou::vk::Context::NumQueueTypes> commandBuffers;
	for (std::size_t queueType = 0; queueType < jjyou::vk::Context::NumQueueTypes; ++queueType)
		commandBuffers[queueType] = this->_context.device().allocateCommandBuffers(
			vk::CommandBufferAllocateInfo(
				*this->_commandPools[queueType],
				vk::CommandBufferLevel::ePrimary,
				Engine::NUM_FRAMES_IN_FLIGHT
			)
		);
	for (std::size_t i = 0; i < static_cast<std::size_t>(Engine::NUM_FRAMES_IN_FLIGHT); ++i) {
		this->_framesInFlight[i].inFlightFence = vk::raii::Fence(
			this->_context.device(),
			vk::FenceCreateInfo(
				vk::FenceCreateFlagBits::eSignaled
			)
		);
		this->_framesInFlight[i].imageAvailableSemaphore = vk::raii::Semaphore(
			this->_context.device(),
			vk::SemaphoreCreateInfo(
				vk::SemaphoreCreateFlags(0)
			)
		);
		this->_framesInFlight[i].renderFinishedSemaphore = vk::raii::Semaphore(
			this->_context.device(),
			vk::SemaphoreCreateInfo(
				vk::SemaphoreCreateFlags(0)
			)
		);
		for (std::size_t queueType = 0; queueType < jjyou::vk::Context::NumQueueTypes; ++queueType)
			this->_framesInFlight[i].commandBuffer[queueType] = std::move(commandBuffers[queueType][i]);
	}
}

void Engine::_resizeRenderResources(void) {
	int width{}, height{};
	std::tie(width, height) = this->_window.framebufferSize();
	while (width == 0 || height == 0) {
		Window::waitEvents();
		std::tie(width, height) = this->_window.framebufferSize();
	}
	this->_context.queue(jjyou::vk::Context::QueueType::Main)->waitIdle();
	this->_createSwapchain();
	this->_createDepthStencil();
	this->_createFramebuffers();
	ImGui_ImplVulkan_SetMinImageCount(this->_swapchain.numImages());
}