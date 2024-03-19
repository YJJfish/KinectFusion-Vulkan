#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#include <memory>
#include <jjyou/vk/Context.hpp>
#include <jjyou/vk/ImGui.hpp>
class Engine {

public:

	jjyou::vk::Context context;

public:

	Engine(bool headless, bool validation);
	~Engine(void) = default;
};