#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <jjyou/vk/Vulkan.hpp>

class Engine {

public:

	Engine(bool debugMode_ = false) : _debugMode(debugMode_) {}
	void createContext(void);
	~Engine(void) = default;

	const jjyou::vk::Context& context(void) const { return this->_context; }

private:

	bool _debugMode;
	jjyou::vk::Context _context{ nullptr };
	jjyou::vk::Swapchain _swapChain{ nullptr };
};