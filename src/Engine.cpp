#include "Engine.hpp"
#include <iostream>

Engine::Engine(bool headless, bool validation) : context(nullptr) {
	glfwInit();
	jjyou::vk::ContextBuilder contextBuilder;
	contextBuilder
		.enableValidation(validation)
		.headless(headless)
		.applicationName("KinectFusion-Vulkan")
		.applicationVersion(0U, 1U, 0U, 0U)
		.engineName("KinectFusion-Vulkan")
		.engineVersion(0U, 1U, 0U, 0U)
		.apiVersion(0U, 1U, 0U, 0U);
	if (validation)
		contextBuilder.useDefaultDebugUtilsMessenger();
	if (!headless) {
		std::uint32_t count{};
		const char** glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&count);
		contextBuilder.enableInstanceExtensions(glfwRequiredExtensions, glfwRequiredExtensions + count);
	}
	contextBuilder.buildInstance(this->context);
	contextBuilder.selectPhysicalDevice(this->context);
	contextBuilder.buildDevice(this->context);
}