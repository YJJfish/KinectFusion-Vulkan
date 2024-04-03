#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#include <jjyou/vk/Vulkan.hpp>
#define JJYOU_USE_VULKAN
#include <jjyou/vis/CameraView.hpp>
#include <chrono>

class Window {

public:

	Window(void) {}
	Window(const Window&) = delete;
	Window(Window&&) = delete;
	Window& operator=(const Window&) = delete;
	Window& operator=(Window&&) = delete;
	~Window(void);

	void createWindow(int width_, int height_, const char* title_);

	static std::vector<const char*> getRequiredInstanceExtensions(void);

	void createSurface(const vk::raii::Instance& instance_);

	void resetSceneViewer(void) {
		this->_sceneViewer.reset();
		this->_sceneViewer.setZoomRate(10.0f);
	}

	bool getAndResetFramebufferResized(void) {
		bool ret = _framebufferResized;
		this->_framebufferResized = false;
		return ret;
	}

	GLFWwindow* glfwWindow(void) const { return this->_glfwWindow; }
	const vk::raii::SurfaceKHR& surface(void) const { return this->_surface; }

	std::pair<int, int> framebufferSize(void) const { int width{}, height{}; glfwGetFramebufferSize(this->_glfwWindow, &width, &height); return std::make_pair(width, height); }
	static void waitEvents(void) { glfwWaitEvents(); }
	static void pollEvents(void) { glfwPollEvents(); }
	int windowShouldClose(void) const { return glfwWindowShouldClose(this->_glfwWindow); }

private:

	GLFWwindow* _glfwWindow = nullptr;
	vk::raii::SurfaceKHR _surface{ nullptr };
	jjyou::vis::SceneView _sceneViewer{};

	static std::size_t _numGLFWWindows;

	bool _framebufferResized = false;
	static void _framebufferResizeCallbackDelegate(GLFWwindow* glfwWindow_, int width_, int height_);
	void _framebufferResizeCallback(int width_, int height_);

	std::chrono::steady_clock::time_point _mouseButtonLeftPressTime{}; 
	static void _mouseButtonCallbackDelegate(GLFWwindow* glfwWindow_, int button_, int action_, int mods_);
	void _mouseButtonCallback(int button_, int action_, int mods_);

	double _cursorPosX{}, _cursorPosY{};
	static void _cursorPosCallbackDelegate(GLFWwindow* glfwWindow_, double xPos_, double yPos_);
	void _cursorPosCallback(double xPos_, double yPos_);

	static void _scrollCallbackDelegate(GLFWwindow* glfwWindow_, double xOffset_, double yOffset_);
	void _scrollCallback(double xOffset_, double yOffset_);

	static void _keyCallbackDelegate(GLFWwindow* glfwWindow_, int key_, int scancode_, int action_, int mods_);
	void _keyCallback(int key_, int scancode_, int action_, int mods_);

};