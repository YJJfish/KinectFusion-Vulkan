#include "Engine.hpp"
#include <iostream>
#include <GLFW/glfw3.h>

void Engine::createContext(void) {
	jjyou::vk::ContextBuilder contextBuilder;
	contextBuilder
		.enableValidation(this->_debugMode)
		.headless(false)
		.applicationName("KinectFusion-Vulkan")
		.applicationVersion(0U, 1U, 0U, 0U)
		.engineName("KinectFusion-Vulkan")
		.engineVersion(0U, 1U, 0U, 0U)
		.apiVersion(0U, 1U, 0U, 0U);
	if (this->_debugMode)
		contextBuilder.useDefaultDebugUtilsMessenger();
	std::uint32_t glfwRequiredExtensionCount{};
	const char** glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&glfwRequiredExtensionCount);
	contextBuilder.enableInstanceExtensions(glfwRequiredExtensions, glfwRequiredExtensions + glfwRequiredExtensionCount);
	contextBuilder.buildInstance(this->_context);
	contextBuilder.selectPhysicalDevice(this->_context);
	contextBuilder.buildDevice(this->_context);
}