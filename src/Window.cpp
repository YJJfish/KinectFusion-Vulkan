#include "Window.hpp"

std::size_t Window::_numGLFWWindows = 0;

Window::~Window(void) {
	if (this->_glfwWindow != nullptr) {
		glfwDestroyWindow(this->_glfwWindow);
		this->_glfwWindow = nullptr;
		--Window::_numGLFWWindows;
		if (Window::_numGLFWWindows == 0)
			glfwTerminate();
	}
}

void Window::createWindow(int width_, int height_) {
	if (Window::_numGLFWWindows == 0)
		glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	this->_glfwWindow = glfwCreateWindow(width_, height_, "KinectFusion-Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(this->_glfwWindow, this);
	glfwSetFramebufferSizeCallback(this->_glfwWindow, Window::_framebufferResizeCallbackDelegate);
	glfwSetMouseButtonCallback(this->_glfwWindow, Window::_mouseButtonCallbackDelegate);
	glfwSetCursorPosCallback(this->_glfwWindow, Window::_cursorPosCallbackDelegate);
	glfwSetScrollCallback(this->_glfwWindow, Window::_scrollCallbackDelegate);
	glfwSetKeyCallback(this->_glfwWindow, Window::_keyCallbackDelegate);
	this->resetSceneViewer();
	++Window::_numGLFWWindows;
}

void Window::createSurface(const jjyou::vk::Context& context_) {
	VkSurfaceKHR surface{};
	glfwCreateWindowSurface(*context_.instance(), this->_glfwWindow, nullptr, &surface);
	this->_surface = vk::raii::SurfaceKHR(context_.instance(), surface);
}

void Window::setupUI(const jjyou::vk::Context& context_) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = g_Instance;
	init_info.PhysicalDevice = g_PhysicalDevice;
	init_info.Device = g_Device;
	init_info.QueueFamily = g_QueueFamily;
	init_info.Queue = g_Queue;
	init_info.PipelineCache = g_PipelineCache;
	init_info.DescriptorPool = g_DescriptorPool;
	init_info.Subpass = 0;
	init_info.MinImageCount = g_MinImageCount;
	init_info.ImageCount = wd->ImageCount;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = g_Allocator;
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);
}

void Window::_framebufferResizeCallbackDelegate(GLFWwindow* glfwWindow_, int width_, int height_) {
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow_));
	window->_framebufferResizeCallback(width_, height_);
}
void Window::_framebufferResizeCallback(int width_, int height_) {
	this->_framebufferResized = true;
}

void Window::_mouseButtonCallbackDelegate(GLFWwindow* glfwWindow_, int button_, int action_, int mods_) {
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow_));
	window->_mouseButtonCallback(button_, action_, mods_);
}
void Window::_mouseButtonCallback(int button_, int action_, int mods_) {
	auto now = std::chrono::steady_clock::now();
	if (button_ == GLFW_MOUSE_BUTTON_LEFT && action_ == GLFW_PRESS) {
		if ((now - this->_mouseButtonLeftPressTime).count() <= 200000000) {
			this->resetSceneViewer();
		}
		this->_mouseButtonLeftPressTime = now;
	}
}

void Window::_cursorPosCallbackDelegate(GLFWwindow* glfwWindow_, double xPos_, double yPos_) {
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow_));
	window->_cursorPosCallback(xPos_, yPos_);
}
void Window::_cursorPosCallback(double xPos_, double yPos_) {
	if (glfwGetMouseButton(this->_glfwWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
		if (glfwGetKey(this->_glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(this->_glfwWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
			this->_sceneViewer.moveUp(static_cast<float>(0.001 * (yPos_ - this->_cursorPosY)));
			this->_sceneViewer.moveLeft(static_cast<float>(0.001 * (xPos_ - this->_cursorPosX)));
		}
		else if (glfwGetKey(this->_glfwWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(this->_glfwWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
			this->_sceneViewer.zoomIn(static_cast<float>(1.0 - 0.005 * (yPos_ - this->_cursorPosY)));
		}
		else {
			this->_sceneViewer.turn(static_cast<float>(0.002 * (this->_cursorPosX - xPos_)), static_cast<float>(0.002 * (this->_cursorPosY - yPos_)), 0.0f);
		}
	}
	this->_cursorPosX = xPos_;
	this->_cursorPosY = yPos_;
}

void Window::_scrollCallbackDelegate(GLFWwindow* glfwWindow_, double xOffset_, double yOffset_) {
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow_));
	window->_scrollCallback(xOffset_, yOffset_);
}
void Window::_scrollCallback(double xOffset_, double yOffset_) {
	if (yOffset_ < 0.0)
		this->_sceneViewer.zoomOut(1.15f);
	else
		this->_sceneViewer.zoomIn(1.15f);
}

void Window::_keyCallbackDelegate(GLFWwindow* glfwWindow_, int key_, int scancode_, int action_, int mods_) {
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow_));
	window->_keyCallback(key_, scancode_, action_, mods_);
}
void Window::_keyCallback(int key_, int scancode_, int action_, int mods_) {
}