#include "Window.hpp"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

std::size_t Window::_numGLFWWindows = 0;

void Window::clear(void) {
	if (this->_glfwWindow != nullptr) {
		this->_surface.clear();
		glfwDestroyWindow(this->_glfwWindow);
		this->_glfwWindow = nullptr;
		--Window::_numGLFWWindows;
		if (Window::_numGLFWWindows == 0)
			glfwTerminate();
	}
}

Window::Window(int width_, int height_, const char* title_) {
	if (Window::_numGLFWWindows == 0)
		glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	this->_glfwWindow = glfwCreateWindow(width_, height_, title_, nullptr, nullptr);
	glfwSetWindowUserPointer(this->_glfwWindow, this);
	glfwSetFramebufferSizeCallback(this->_glfwWindow, Window::_framebufferResizeCallbackDelegate);
	glfwSetMouseButtonCallback(this->_glfwWindow, Window::_mouseButtonCallbackDelegate);
	glfwSetCursorPosCallback(this->_glfwWindow, Window::_cursorPosCallbackDelegate);
	glfwSetScrollCallback(this->_glfwWindow, Window::_scrollCallbackDelegate);
	glfwSetKeyCallback(this->_glfwWindow, Window::_keyCallbackDelegate);
	this->_resetSceneViewer();
	++Window::_numGLFWWindows;
}

std::vector<const char*> Window::getRequiredInstanceExtensions(void) {
	std::uint32_t glfwRequiredInstanceExtensionCount{};
	const char** glfwRequiredInstanceExtensions = glfwGetRequiredInstanceExtensions(&glfwRequiredInstanceExtensionCount);
	return std::vector<const char*>(glfwRequiredInstanceExtensions, glfwRequiredInstanceExtensions + glfwRequiredInstanceExtensionCount);
}

void Window::createSurface(const vk::raii::Instance& instance_) {
	VkSurfaceKHR surface{};
	glfwCreateWindowSurface(*instance_, this->_glfwWindow, nullptr, &surface);
	this->_surface = vk::raii::SurfaceKHR(instance_, surface);
}

void Window::_framebufferResizeCallbackDelegate(GLFWwindow* glfwWindow_, int width_, int height_) {
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow_));
	window->_framebufferResizeCallback(width_, height_);
}
void Window::_framebufferResizeCallback(int width_, int height_) {
	this->_framebufferResized = true;
}

void Window::_mouseButtonCallbackDelegate(GLFWwindow* glfwWindow_, int button_, int action_, int mods_) {
	if (ImGui::GetIO().WantCaptureMouse)
		return;
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow_));
	window->_mouseButtonCallback(button_, action_, mods_);
}
void Window::_mouseButtonCallback(int button_, int action_, int mods_) {
	auto now = std::chrono::steady_clock::now();
	switch (button_) {
	case GLFW_MOUSE_BUTTON_LEFT: {
		switch (action_) {
		case GLFW_RELEASE: {
			this->_mouseButtonLeftPressed = false;
			break;
		}
		case GLFW_PRESS: {
			this->_mouseButtonLeftPressed = true;
			if ((this->_cameraMode == CameraMode::Scene) && (now - this->_mouseButtonLeftPressTime).count() <= 200000000) {
				this->_resetSceneViewer();
			}
			this->_mouseButtonLeftPressTime = now;
			break;
		}
		default: {break; }
		}
		break;
	}
	case GLFW_MOUSE_BUTTON_RIGHT: {
		switch (action_) {
		case GLFW_RELEASE: {
			this->_mouseButtonRightPressed = false;
			break;
		}
		case GLFW_PRESS: {
			this->_mouseButtonRightPressed = true;
			break;
		}
		default: {break; }
		}
		break;
	}
	case GLFW_MOUSE_BUTTON_MIDDLE: {
		switch (action_) {
		case GLFW_RELEASE: {
			this->_mouseButtonMiddlePressed = false;
			break;
		}
		case GLFW_PRESS: {
			this->_mouseButtonMiddlePressed = true;
			break;
		}
		default: {break; }
		}
		break;
	}
	default: {break; }
	}
}

void Window::_cursorPosCallbackDelegate(GLFWwindow* glfwWindow_, double xPos_, double yPos_) {
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow_));
	window->_cursorPosCallback(xPos_, yPos_);
}
void Window::_cursorPosCallback(double xPos_, double yPos_) {
	if (this->_cameraMode == CameraMode::Scene) {
		if (this->_mouseButtonMiddlePressed) {
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
	}
	this->_cursorPosX = xPos_;
	this->_cursorPosY = yPos_;
}

void Window::_scrollCallbackDelegate(GLFWwindow* glfwWindow_, double xOffset_, double yOffset_) {
	if (ImGui::GetIO().WantCaptureMouse)
		return;
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
	if (ImGui::GetIO().WantCaptureKeyboard)
		return;
	Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow_));
	window->_keyCallback(key_, scancode_, action_, mods_);
}
void Window::_keyCallback(int key_, int scancode_, int action_, int mods_) {
}