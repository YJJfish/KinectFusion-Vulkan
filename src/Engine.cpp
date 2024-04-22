#include "Engine.hpp"
#include <iostream>
#include <GLFW/glfw3.h>
#include <numbers>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

Engine::Engine(bool headlessMode_, bool debugMode_) : _headlessMode(headlessMode_), _debugMode(debugMode_), _window(800, 600, "KinectFusion-Vulkan") {
	if (headlessMode_)
		throw std::logic_error("[Engine] Headless mode has not been implemented.");
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
	this->waitIdle();
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
}

vk::Result Engine::prepareFrame(void) {
	this->_getPrimitivesToDraw<MaterialType::Simple, PrimitiveType::Point>().clear();
	this->_getPrimitivesToDraw<MaterialType::Simple, PrimitiveType::Line>().clear();
	this->_getPrimitivesToDraw<MaterialType::Simple, PrimitiveType::Triangle>().clear();
	this->_getPrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Point>().clear();
	this->_getPrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Line>().clear();
	this->_getPrimitivesToDraw<MaterialType::Lambertian, PrimitiveType::Triangle>().clear();
	this->_getSurfacesToDraw<MaterialType::Simple>().clear();
	this->_getSurfacesToDraw<MaterialType::Lambertian>().clear();
	vk::Result waitFenceResult = this->_context.device().waitForFences({ *this->_activeFrameData().inFlightFence }, VK_TRUE, std::numeric_limits<std::uint64_t>::max());
	if (waitFenceResult != vk::Result::eSuccess) {
		throw std::runtime_error("[Engine] Error occurred when waiting for the frame fence.");
	}
	vk::Result acquireImageResult{};
	std::tie(acquireImageResult, this->_swapchainImageIndex) = this->_swapchain.swapchain().acquireNextImage(UINT64_MAX, *this->_activeFrameData().imageAvailableSemaphore, nullptr);
	if (acquireImageResult == vk::Result::eErrorOutOfDateKHR) {
		this->_resizeRenderResources();
		return vk::Result::eErrorOutOfDateKHR;
	}
	else if (acquireImageResult != vk::Result::eSuccess && acquireImageResult != vk::Result::eSuboptimalKHR) {
		throw std::runtime_error("[Engine] Failed to acquire the image from swapchain.");
	}
	this->_context.device().resetFences({ *this->_activeFrameData().inFlightFence });
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	this->_activeFrameData().graphicsCommandBuffer.reset();
	this->_activeFrameData().graphicsCommandBuffer.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlags(0)).setPInheritanceInfo(nullptr));
	return acquireImageResult;
}

vk::Result Engine::presentFrame(void) {
	this->_activeFrameData().graphicsCommandBuffer.end();
	vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo = vk::SubmitInfo()
		.setWaitSemaphores(*this->_activeFrameData().imageAvailableSemaphore).setWaitDstStageMask(waitStage)
		.setCommandBuffers(*this->_activeFrameData().graphicsCommandBuffer)
		.setSignalSemaphores(*this->_activeFrameData().renderFinishedSemaphore);
	this->_context.queue(jjyou::vk::Context::QueueType::Main)->submit(submitInfo, *this->_activeFrameData().inFlightFence);
	vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR()
		.setWaitSemaphores(*this->_activeFrameData().renderFinishedSemaphore)
		.setSwapchains(*this->_swapchain.swapchain())
		.setImageIndices(this->_swapchainImageIndex)
		.setPResults(nullptr);
	vk::Result presentResult{};
	try {
		presentResult = this->_context.queue(jjyou::vk::Context::QueueType::Main)->presentKHR(presentInfo);
	}
	catch (const vk::OutOfDateKHRError&) {
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

void Engine::recordCommandbuffer(void) const {
	// Set the viewport and the scissor
	vk::Extent2D screenExtent = this->_swapchain.extent();
	vk::Extent2D cameraExtent = vk::Extent2D(this->getCamera().width, this->getCamera().height);
	vk::Viewport sceneViewport = vk::Viewport()
		.setX(static_cast<float>(screenExtent.width - cameraExtent.width) / 2.0f)
		.setY(static_cast<float>(screenExtent.height - cameraExtent.height) / 2.0f)
		.setWidth(static_cast<float>(cameraExtent.width))
		.setHeight(static_cast<float>(cameraExtent.height))
		.setMinDepth(0.0f)
		.setMaxDepth(1.0f);
	vk::Rect2D sceneScissor = vk::Rect2D()
		.setOffset(vk::Offset2D(0, 0))
		.setExtent(screenExtent);
	this->_activeFrameData().graphicsCommandBuffer.setViewport(0, sceneViewport);
	this->_activeFrameData().graphicsCommandBuffer.setScissor(0, sceneScissor);
	// Begin render pass
	std::vector<vk::ClearValue> clearValues = {
		vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{{0.0f, 0.0f, 0.0f, 1.0f}})),
		vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0))
	};
	vk::RenderPassBeginInfo renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setRenderPass(*this->_renderPass)
		.setFramebuffer(*this->_activeFramebuffer())
		.setRenderArea(vk::Rect2D(vk::Offset2D(0, 0), this->_swapchain.extent()))
		.setClearValues(clearValues);
	this->_activeFrameData().graphicsCommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
	// Set view level uniform data
	jjyou::glsl::mat4 viewMatrix = this->_window.getViewMatrix();
	jjyou::glsl::mat4 projectionMatrix = this->getCamera().getGraphicsProjection();
	this->_activeFrameData().viewLevelDescriptorSet.cameraParameters().projection = projectionMatrix;
	this->_activeFrameData().viewLevelDescriptorSet.cameraParameters().view = viewMatrix;
	this->_activeFrameData().viewLevelDescriptorSet.cameraParameters().viewPos = jjyou::glsl::vec4(-jjyou::glsl::transpose(jjyou::glsl::mat3(viewMatrix)) * jjyou::glsl::vec3(viewMatrix[3]), 1.0f);
	std::uint32_t instanceCount = 0;
	// Render primitives
	auto renderPrimitives = [&]<MaterialType _materialType, PrimitiveType _primitiveType>() {
		this->_activeFrameData().graphicsCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *this->_primitivePipelines[_materialType][_primitiveType]);
		this->_activeFrameData().viewLevelDescriptorSet.bind(this->_activeFrameData().graphicsCommandBuffer, vk::PipelineBindPoint::eGraphics, this->_primitivePipelineLayouts[_materialType], 0);
		for (const auto& instance : this->_getPrimitivesToDraw<_materialType, _primitiveType>()) {
			this->_activeFrameData().instanceLevelDescriptorSet.modelTransforms(instanceCount).model = instance.modelMatrix;
			this->_activeFrameData().instanceLevelDescriptorSet.modelTransforms(instanceCount).normal = instance.normalMatrix;
			this->_activeFrameData().instanceLevelDescriptorSet.bind(this->_activeFrameData().graphicsCommandBuffer, vk::PipelineBindPoint::eGraphics, this->_primitivePipelineLayouts[_materialType], 1, instanceCount);
			instance.pPrimitives->draw(this->_activeFrameData().graphicsCommandBuffer);
			++instanceCount;
		}
	};
	renderPrimitives.template operator() < MaterialType::Simple, PrimitiveType::Point > ();
	renderPrimitives.template operator() < MaterialType::Simple, PrimitiveType::Line > ();
	renderPrimitives.template operator() < MaterialType::Simple, PrimitiveType::Triangle > ();
	renderPrimitives.template operator() < MaterialType::Lambertian, PrimitiveType::Point > ();
	renderPrimitives.template operator() < MaterialType::Lambertian, PrimitiveType::Line > ();
	renderPrimitives.template operator() < MaterialType::Lambertian, PrimitiveType::Triangle > ();
	// Render surfaces
	auto renderSurfaces = [&]<MaterialType _materialType>() {
		this->_activeFrameData().graphicsCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *this->_surfacePipelines[_materialType]);
		this->_activeFrameData().viewLevelDescriptorSet.bind(this->_activeFrameData().graphicsCommandBuffer, vk::PipelineBindPoint::eGraphics, this->_surfacePipelineLayouts[_materialType], 0);
		for (const auto& pSurface : this->_getSurfacesToDraw<_materialType>()) {
			pSurface->bindSampler(this->_activeFrameData().graphicsCommandBuffer, vk::PipelineBindPoint::eGraphics, this->_surfacePipelineLayouts[_materialType], 1);
			pSurface->draw(this->_activeFrameData().graphicsCommandBuffer);
		}
	};
	renderSurfaces.template operator() < MaterialType::Simple > ();
	renderSurfaces.template operator() < MaterialType::Lambertian > ();
	// Render UI
	ImGui::Render();
	ImDrawData* imDrawData = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData(imDrawData, *this->_activeFrameData().graphicsCommandBuffer);
	this->_activeFrameData().graphicsCommandBuffer.endRenderPass();
}

void Engine::waitIdle(void) const {
	for (std::size_t queueType = 0; queueType < jjyou::vk::Context::NumQueueTypes; ++queueType)
		this->_context.queue(queueType)->waitIdle();
}

void Engine::setCameraMode(
	Window::CameraMode cameraMode_,
	std::optional<jjyou::glsl::mat4> viewMatrix_,
	std::optional<Camera> camera_
) {
	this->_cameraMode = cameraMode_;
	if (!this->_headlessMode)
		this->_window.setCameraMode(cameraMode_, viewMatrix_);
	if (cameraMode_ == Window::CameraMode::Fixed) {
		this->_fixedCamera = *camera_;
		if (!this->_headlessMode) {
			vk::Extent2D swapchainExtent = this->_swapchain.extent();
			this->_fixedCamera.scaleToFit(swapchainExtent.width, swapchainExtent.height);
		}
	}
}

void Engine::_createContext(void) {
	// Create glfw window
	//if (!this->_headlessMode)
	//	this->_window = Window(800, 600, "KinectFusion-Vulkan");
	this->_sceneCamera = Camera::fromGraphics(std::nullopt, std::numbers::pi_v<float> / 3.0f, 0.1f, 100.0f, 800, 600);
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
		contextBuilder.addSurface(this->_window.surface());
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
			vk::CommandPoolCreateInfo()
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(*this->_context.queueFamilyIndex(queueType))
		);
}

void Engine::_createSwapchain(void) {
	jjyou::vk::SwapchainBuilder builder(this->_context, this->_window.surface());
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
		vk::AttachmentDescription()
		.setFlags(vk::AttachmentDescriptionFlags(0U))
		.setFormat(this->_swapchain.surfaceFormat().format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout((this->_headlessMode) ? vk::ImageLayout::eTransferSrcOptimal : vk::ImageLayout::ePresentSrcKHR),
		// Depth attachment
		vk::AttachmentDescription()
		.setFlags(vk::AttachmentDescriptionFlags(0U))
		.setFormat(vk::Format::eD32Sfloat)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
	};

	// Subpass
	vk::AttachmentReference subpassColorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentReference subpassDepthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	vk::SubpassDescription subpassDescription = vk::SubpassDescription()
		.setFlags(vk::SubpassDescriptionFlags(0U))
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setInputAttachments(nullptr)
		.setColorAttachments(subpassColorAttachmentRef)
		//.setResolveAttachments(nullptr)
		.setPDepthStencilAttachment(&subpassDepthAttachmentRef)
		.setPreserveAttachments(nullptr);

	// Subpass dependencies
	std::vector<vk::SubpassDependency> subpassDependencies = {
		vk::SubpassDependency()
		.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
		.setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests)
		.setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
		.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead)
		.setDependencyFlags(vk::DependencyFlags(0U)),
		vk::SubpassDependency()
		.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags(0U))
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
		.setDependencyFlags(vk::DependencyFlags(0U))
	};

	// Create renderpass
	vk::RenderPassCreateInfo renderPassCreateInfo = vk::RenderPassCreateInfo()
		.setFlags(vk::RenderPassCreateFlags(0U))
		.setAttachments(attachmentDescriptions)
		.setSubpasses(subpassDescription)
		.setDependencies(subpassDependencies);
	this->_renderPass = vk::raii::RenderPass(this->_context.device(), renderPassCreateInfo);
}

void Engine::_createDepthStencil(void) {
	vk::Extent2D extent = this->_swapchain.extent();
	this->_depthImage = Texture2D(*this, vk::Format::eD32Sfloat, extent, vk::ImageUsageFlagBits::eDepthStencilAttachment, {});
}

void Engine::_createFramebuffers(void) {
	this->_framebuffers.clear();
	this->_framebuffers.reserve(static_cast<std::size_t>(this->_swapchain.numImages()));
	vk::FramebufferCreateInfo framebufferCreateInfo = vk::FramebufferCreateInfo()
		.setFlags(vk::FramebufferCreateFlags(0))
		.setRenderPass(*this->_renderPass)
		.setAttachments(nullptr)
		.setWidth(this->_swapchain.extent().width)
		.setHeight(this->_swapchain.extent().height)
		.setLayers(1);
	for (std::uint32_t i = 0; i < this->_swapchain.numImages(); ++i) {
		std::array<vk::ImageView, 2> attachments = {
			*this->_swapchain.imageView(i),
			*this->_depthImage.imageView()
		};
		framebufferCreateInfo.setAttachments(attachments);
		this->_framebuffers.emplace_back(this->_context.device(), framebufferCreateInfo);
	}
}

void Engine::_createDescriptorSetLayouts(void) {
	// _viewLevelDescriptorSetLayout
	this->_viewLevelDescriptorSetLayout = ViewLevelDescriptorSet::createDescriptorSetLayout(this->_context.device());

	// _instanceLevelDescriptorSetLayout
	this->_instanceLevelDescriptorSetLayout = InstanceLevelDescriptorSet::createDescriptorSetLayout(this->_context.device());

	// _surfaceDescriptorSetLayouts - simple
	this->_surfaceSamplerDescriptorSetLayouts[MaterialType::Simple] = Surface<MaterialType::Simple>::createSamplerDescriptorSetLayout(this->_context.device());
	this->_surfaceStorageDescriptorSetLayouts[MaterialType::Simple] = Surface<MaterialType::Simple>::createStorageDescriptorSetLayout(this->_context.device());
	
	// _surfaceDescriptorSetLayouts - lambertian
	this->_surfaceSamplerDescriptorSetLayouts[MaterialType::Lambertian] = Surface<MaterialType::Lambertian>::createSamplerDescriptorSetLayout(this->_context.device());
	this->_surfaceStorageDescriptorSetLayouts[MaterialType::Lambertian] = Surface<MaterialType::Lambertian>::createStorageDescriptorSetLayout(this->_context.device());
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
	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo()
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
		.setMaxSets(1000)
		.setPoolSizes(descriptorPoolSizes);
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
	// simple primitives
	{
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_viewLevelDescriptorSetLayout,
			*this->_instanceLevelDescriptorSetLayout,
		};
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setFlags(vk::PipelineLayoutCreateFlags(0))
			.setSetLayouts(descriptorSetLayouts)
			.setPushConstantRanges(nullptr);
		this->_primitivePipelineLayouts[MaterialType::Simple] = vk::raii::PipelineLayout(this->_context.device(), pipelineLayoutCreateInfo);
	}

	// lambertian primitives
	{
		// Currently, lambertian pipeline layout is the same as simple pipeline layout.
		// However, extensions may be made to support textures, lightings, etc.
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_viewLevelDescriptorSetLayout,
			*this->_instanceLevelDescriptorSetLayout,
		};
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setFlags(vk::PipelineLayoutCreateFlags(0))
			.setSetLayouts(descriptorSetLayouts)
			.setPushConstantRanges(nullptr);
		this->_primitivePipelineLayouts[MaterialType::Lambertian] = vk::raii::PipelineLayout(this->_context.device(), pipelineLayoutCreateInfo);
	}

	// simple surface
	{
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_viewLevelDescriptorSetLayout,
			*this->_surfaceSamplerDescriptorSetLayouts[MaterialType::Simple]
		};
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setFlags(vk::PipelineLayoutCreateFlags(0))
			.setSetLayouts(descriptorSetLayouts)
			.setPushConstantRanges(nullptr);
		this->_surfacePipelineLayouts[MaterialType::Simple] = vk::raii::PipelineLayout(this->_context.device(), pipelineLayoutCreateInfo);
	}

	// simple surface
	{
		// Currently, lambertian pipeline layout is the same as simple pipeline layout.
		// However, extensions may be made to support textures, lightings, etc.
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			*this->_viewLevelDescriptorSetLayout,
			*this->_surfaceSamplerDescriptorSetLayouts[MaterialType::Lambertian]
		};
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
			.setFlags(vk::PipelineLayoutCreateFlags(0))
			.setSetLayouts(descriptorSetLayouts)
			.setPushConstantRanges(nullptr);
		this->_surfacePipelineLayouts[MaterialType::Lambertian] = vk::raii::PipelineLayout(this->_context.device(), pipelineLayoutCreateInfo);
	}
}

void Engine::_createPipelines() {
	vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = vk::PipelineViewportStateCreateInfo()
		.setFlags(vk::PipelineViewportStateCreateFlags(0))
		.setViewportCount(1).setPViewports(nullptr)
		.setScissorCount(1).setPScissors(nullptr);

	vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo()
		.setFlags(vk::PipelineRasterizationStateCreateFlags(0))
		.setDepthClampEnable(VK_FALSE)
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setCullMode((this->_debugMode) ? vk::CullModeFlagBits::eNone : vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setDepthBiasEnable(VK_FALSE).setDepthBiasConstantFactor(0.0f).setDepthBiasClamp(0.0f).setDepthBiasSlopeFactor(0.0f)
		.setLineWidth(1.0f);

	vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo()
		.setFlags(vk::PipelineMultisampleStateCreateFlags(0))
		.setRasterizationSamples(vk::SampleCountFlagBits::e1)
		.setSampleShadingEnable(VK_FALSE)
		.setMinSampleShading(0.0f)
		.setPSampleMask(nullptr)
		.setAlphaToCoverageEnable(VK_FALSE)
		.setAlphaToOneEnable(VK_FALSE);

	vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = vk::PipelineDepthStencilStateCreateInfo()
		.setFlags(vk::PipelineDepthStencilStateCreateFlags(0))
		.setDepthTestEnable(VK_TRUE)
		.setDepthWriteEnable(VK_TRUE)
		.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
		.setDepthBoundsTestEnable(VK_FALSE)
		.setStencilTestEnable(VK_FALSE)
		.setFront(vk::StencilOpState())
		.setBack(vk::StencilOpState())
		.setMinDepthBounds(0.0f)
		.setMaxDepthBounds(1.0f);

	vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = vk::PipelineColorBlendAttachmentState()
		.setBlendEnable(VK_FALSE)
		.setSrcColorBlendFactor(vk::BlendFactor::eZero)
		.setDstColorBlendFactor(vk::BlendFactor::eZero)
		.setColorBlendOp(vk::BlendOp::eAdd)
		.setSrcAlphaBlendFactor(vk::BlendFactor::eZero)
		.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
		.setAlphaBlendOp(vk::BlendOp::eAdd)
		.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

	vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo()
		.setFlags(vk::PipelineColorBlendStateCreateFlags(0))
		.setLogicOpEnable(VK_FALSE)
		.setLogicOp(vk::LogicOp::eCopy)
		.setAttachments(pipelineColorBlendAttachmentState)
		.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

	std::vector<vk::DynamicState> dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};

	vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
		.setFlags(vk::PipelineDynamicStateCreateFlags(0))
		.setDynamicStates(dynamicStates);

	vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
		.setFlags(vk::PipelineCreateFlags(0))
		//.setStages()
		//.setPVertexInputState()
		//.setPInputAssemblyState()
		.setPTessellationState(nullptr)
		.setPViewportState(&pipelineViewportStateCreateInfo)
		.setPRasterizationState(&pipelineRasterizationStateCreateInfo)
		.setPMultisampleState(&pipelineMultisampleStateCreateInfo)
		.setPDepthStencilState(&pipelineDepthStencilStateCreateInfo)
		.setPColorBlendState(&pipelineColorBlendStateCreateInfo)
		.setPDynamicState(&pipelineDynamicStateCreateInfo)
		//.setLayout()
		.setRenderPass(*this->_renderPass)
		.setSubpass(0U)
		.setBasePipelineHandle(nullptr)
		.setBasePipelineIndex(-1);

	// simple primitive
	{
#include "./shader/simplePrimitive.vert.spv.h"
		vk::raii::ShaderModule vertShaderModule(this->_context.device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(simplePrimitive_vert_spv))
			.setCodeSize(sizeof(simplePrimitive_vert_spv))
		);
#include "./shader/simplePrimitive.frag.spv.h"
		vk::raii::ShaderModule fragShaderModule(this->_context.device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(simplePrimitive_frag_spv))
			.setCodeSize(sizeof(simplePrimitive_frag_spv))
		);
		std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos{
			vk::PipelineShaderStageCreateInfo()
			.setFlags(vk::PipelineShaderStageCreateFlags(0))
			.setStage(vk::ShaderStageFlagBits::eVertex)
			.setModule(*vertShaderModule)
			.setPName("main")
			.setPSpecializationInfo(nullptr),
			vk::PipelineShaderStageCreateInfo()
			.setFlags(vk::PipelineShaderStageCreateFlags(0))
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setModule(*fragShaderModule)
			.setPName("main")
			.setPSpecializationInfo(nullptr),
		};
		vk::VertexInputBindingDescription vertexInputBindingDescription = Vertex<MaterialType::Simple>::getInputBindingDescription();
		std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions = Vertex<MaterialType::Simple>::getInputAttributeDescriptions();
		vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo()
			.setFlags(vk::PipelineVertexInputStateCreateFlags(0))
			.setVertexBindingDescriptions(vertexInputBindingDescription)
			.setVertexAttributeDescriptions(vertexInputAttributeDescriptions);
		vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo()
			.setFlags(vk::PipelineInputAssemblyStateCreateFlags(0))
			//.setTopology()
			.setPrimitiveRestartEnable(VK_FALSE);
		graphicsPipelineCreateInfo
			.setStages(pipelineShaderStageCreateInfos)
			.setPVertexInputState(&pipelineVertexInputStateCreateInfo)
			.setPInputAssemblyState(&pipelineInputAssemblyStateCreateInfo)
			.setLayout(*this->_primitivePipelineLayouts[MaterialType::Simple]);
		pipelineInputAssemblyStateCreateInfo.setTopology(vk::PrimitiveTopology::ePointList);
		this->_primitivePipelines[MaterialType::Simple][PrimitiveType::Point] = vk::raii::Pipeline(this->_context.device(), nullptr, graphicsPipelineCreateInfo);
		pipelineInputAssemblyStateCreateInfo.setTopology(vk::PrimitiveTopology::eLineList);
		this->_primitivePipelines[MaterialType::Simple][PrimitiveType::Line] = vk::raii::Pipeline(this->_context.device(), nullptr, graphicsPipelineCreateInfo);
		pipelineInputAssemblyStateCreateInfo.setTopology(vk::PrimitiveTopology::eTriangleList);
		this->_primitivePipelines[MaterialType::Simple][PrimitiveType::Triangle] = vk::raii::Pipeline(this->_context.device(), nullptr, graphicsPipelineCreateInfo);
	}

	// lambertian primitive
	{
#include "./shader/lambertianPrimitive.vert.spv.h"
		vk::raii::ShaderModule vertShaderModule(this->_context.device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(lambertianPrimitive_vert_spv))
			.setCodeSize(sizeof(lambertianPrimitive_vert_spv))
		);
#include "./shader/lambertianPrimitive.frag.spv.h"
		vk::raii::ShaderModule fragShaderModule(this->_context.device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(lambertianPrimitive_frag_spv))
			.setCodeSize(sizeof(lambertianPrimitive_frag_spv))
		);
		std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos{
			vk::PipelineShaderStageCreateInfo()
			.setFlags(vk::PipelineShaderStageCreateFlags(0))
			.setStage(vk::ShaderStageFlagBits::eVertex)
			.setModule(*vertShaderModule)
			.setPName("main")
			.setPSpecializationInfo(nullptr),
			vk::PipelineShaderStageCreateInfo()
			.setFlags(vk::PipelineShaderStageCreateFlags(0))
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setModule(*fragShaderModule)
			.setPName("main")
			.setPSpecializationInfo(nullptr),
		};
		vk::VertexInputBindingDescription vertexInputBindingDescription = Vertex<MaterialType::Lambertian>::getInputBindingDescription();
		std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions = Vertex<MaterialType::Lambertian>::getInputAttributeDescriptions();
		vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo()
			.setFlags(vk::PipelineVertexInputStateCreateFlags(0))
			.setVertexBindingDescriptions(vertexInputBindingDescription)
			.setVertexAttributeDescriptions(vertexInputAttributeDescriptions);
		vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo()
			.setFlags(vk::PipelineInputAssemblyStateCreateFlags(0))
			//.setTopology()
			.setPrimitiveRestartEnable(VK_FALSE);
		graphicsPipelineCreateInfo
			.setStages(pipelineShaderStageCreateInfos)
			.setPVertexInputState(&pipelineVertexInputStateCreateInfo)
			.setPInputAssemblyState(&pipelineInputAssemblyStateCreateInfo)
			.setLayout(*this->_primitivePipelineLayouts[MaterialType::Lambertian]);
		pipelineInputAssemblyStateCreateInfo.setTopology(vk::PrimitiveTopology::ePointList);
		this->_primitivePipelines[MaterialType::Lambertian][PrimitiveType::Point] = vk::raii::Pipeline(this->_context.device(), nullptr, graphicsPipelineCreateInfo);
		pipelineInputAssemblyStateCreateInfo.setTopology(vk::PrimitiveTopology::eLineList);
		this->_primitivePipelines[MaterialType::Lambertian][PrimitiveType::Line] = vk::raii::Pipeline(this->_context.device(), nullptr, graphicsPipelineCreateInfo);
		pipelineInputAssemblyStateCreateInfo.setTopology(vk::PrimitiveTopology::eTriangleList);
		this->_primitivePipelines[MaterialType::Lambertian][PrimitiveType::Triangle] = vk::raii::Pipeline(this->_context.device(), nullptr, graphicsPipelineCreateInfo);
	}
	
	// simple surface
	{
#include "./shader/surface.vert.spv.h"
		vk::raii::ShaderModule vertShaderModule(this->_context.device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(surface_vert_spv))
			.setCodeSize(sizeof(surface_vert_spv))
		);
#include "./shader/simpleSurface.frag.spv.h"
		vk::raii::ShaderModule fragShaderModule(this->_context.device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(simpleSurface_frag_spv))
			.setCodeSize(sizeof(simpleSurface_frag_spv))
		);
		std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos{
			vk::PipelineShaderStageCreateInfo()
			.setFlags(vk::PipelineShaderStageCreateFlags(0))
			.setStage(vk::ShaderStageFlagBits::eVertex)
			.setModule(*vertShaderModule)
			.setPName("main")
			.setPSpecializationInfo(nullptr),
			vk::PipelineShaderStageCreateInfo()
			.setFlags(vk::PipelineShaderStageCreateFlags(0))
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setModule(*fragShaderModule)
			.setPName("main")
			.setPSpecializationInfo(nullptr),
		};
		vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo()
			.setFlags(vk::PipelineVertexInputStateCreateFlags(0))
			.setVertexBindingDescriptions(nullptr)
			.setVertexAttributeDescriptions(nullptr);
		vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo()
			.setFlags(vk::PipelineInputAssemblyStateCreateFlags(0))
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(VK_FALSE);
		graphicsPipelineCreateInfo
			.setStages(pipelineShaderStageCreateInfos)
			.setPVertexInputState(&pipelineVertexInputStateCreateInfo)
			.setPInputAssemblyState(&pipelineInputAssemblyStateCreateInfo)
			.setLayout(*this->_surfacePipelineLayouts[MaterialType::Simple]);
		this->_surfacePipelines[MaterialType::Simple] = vk::raii::Pipeline(this->_context.device(), nullptr, graphicsPipelineCreateInfo);
	}

	// lambertian surface
	{
#include "./shader/surface.vert.spv.h"
		vk::raii::ShaderModule vertShaderModule(this->_context.device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(surface_vert_spv))
			.setCodeSize(sizeof(surface_vert_spv))
		);
#include "./shader/lambertianSurface.frag.spv.h"
		vk::raii::ShaderModule fragShaderModule(this->_context.device(), vk::ShaderModuleCreateInfo()
			.setFlags(vk::ShaderModuleCreateFlags(0))
			.setPCode(reinterpret_cast<const uint32_t*>(lambertianSurface_frag_spv))
			.setCodeSize(sizeof(lambertianSurface_frag_spv))
		);
		std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos{
			vk::PipelineShaderStageCreateInfo()
			.setFlags(vk::PipelineShaderStageCreateFlags(0))
			.setStage(vk::ShaderStageFlagBits::eVertex)
			.setModule(*vertShaderModule)
			.setPName("main")
			.setPSpecializationInfo(nullptr),
			vk::PipelineShaderStageCreateInfo()
			.setFlags(vk::PipelineShaderStageCreateFlags(0))
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setModule(*fragShaderModule)
			.setPName("main")
			.setPSpecializationInfo(nullptr),
		};
		vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo()
			.setFlags(vk::PipelineVertexInputStateCreateFlags(0))
			.setVertexBindingDescriptions(nullptr)
			.setVertexAttributeDescriptions(nullptr);
		vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo()
			.setFlags(vk::PipelineInputAssemblyStateCreateFlags(0))
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(VK_FALSE);
		graphicsPipelineCreateInfo
			.setStages(pipelineShaderStageCreateInfos)
			.setPVertexInputState(&pipelineVertexInputStateCreateInfo)
			.setPInputAssemblyState(&pipelineInputAssemblyStateCreateInfo)
			.setLayout(*this->_surfacePipelineLayouts[MaterialType::Lambertian]);
		this->_surfacePipelines[MaterialType::Lambertian] = vk::raii::Pipeline(this->_context.device(), nullptr, graphicsPipelineCreateInfo);
	}
}

void Engine::_createFrameData(void) {
	std::vector<vk::raii::CommandBuffer> graphicsCommandBuffers;
	graphicsCommandBuffers = this->_context.device().allocateCommandBuffers(
		vk::CommandBufferAllocateInfo()
		.setCommandPool(*this->_commandPools[jjyou::vk::Context::QueueType::Main])
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(Engine::NUM_FRAMES_IN_FLIGHT)
	);
	for (std::size_t i = 0; i < static_cast<std::size_t>(Engine::NUM_FRAMES_IN_FLIGHT); ++i) {
		this->_framesInFlight[i].inFlightFence = vk::raii::Fence(
			this->_context.device(),
			vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled)
		);
		this->_framesInFlight[i].imageAvailableSemaphore = vk::raii::Semaphore(
			this->_context.device(),
			vk::SemaphoreCreateInfo().setFlags(vk::SemaphoreCreateFlags(0))
		);
		this->_framesInFlight[i].renderFinishedSemaphore = vk::raii::Semaphore(
			this->_context.device(),
			vk::SemaphoreCreateInfo().setFlags(vk::SemaphoreCreateFlags(0))
		);
		this->_framesInFlight[i].graphicsCommandBuffer = std::move(graphicsCommandBuffers[i]);
		this->_framesInFlight[i].viewLevelDescriptorSet = ViewLevelDescriptorSet(*this);
		this->_framesInFlight[i].instanceLevelDescriptorSet = InstanceLevelDescriptorSet(*this, 256);
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
	this->_sceneCamera = Camera::fromGraphics(std::nullopt, this->_sceneCamera.yFov, this->_sceneCamera.zNear, this->_sceneCamera.zFar, static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
	if (this->_cameraMode == Window::CameraMode::Fixed) {
		this->_fixedCamera.scaleToFit(static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
	}
	this->_createSwapchain();
	this->_createDepthStencil();
	this->_createFramebuffers();
	ImGui_ImplVulkan_SetMinImageCount(this->_swapchain.numImages());
	this->_window.getAndResetFramebufferResized();
}