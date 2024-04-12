#include "Application.hpp"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

Application::Application(void) :
	_headlessMode(false),
	_debugMode(true),
	_engine(this->_headlessMode, this->_debugMode),
	_kinectFusion(this->_engine, vk::Extent2D(1024, 1024), vk::Extent2D(1024, 1024), jjyou::glsl::uvec3(512U, 512U, 512U), 0.005f)
{
	std::array<Vertex<MaterialType::Simple>, 6> axesData = { {
		Vertex<MaterialType::Simple>{.position{0.0f, 0.0f, 0.0f}, .color{255, 0, 0, 255} },
		Vertex<MaterialType::Simple>{.position{1.0f, 0.0f, 0.0f}, .color{255, 0, 0, 255} },
		Vertex<MaterialType::Simple>{.position{0.0f, 0.0f, 0.0f}, .color{0, 255, 0, 255} },
		Vertex<MaterialType::Simple>{.position{0.0f, 1.0f, 0.0f}, .color{0, 255, 0, 255} },
		Vertex<MaterialType::Simple>{.position{0.0f, 0.0f, 0.0f}, .color{0, 0, 255, 255} },
		Vertex<MaterialType::Simple>{.position{0.0f, 0.0f, 1.0f}, .color{0, 0, 255, 255} },
	} };
	std::array<Vertex<MaterialType::Lambertian>, 3> triangleData = { {
		Vertex<MaterialType::Lambertian>{.position{-1.0f, -1.0f, 0.0f}, .normal{0.0f, 0.0f, -1.0f}, .color{0, 255, 0, 255} },
		Vertex<MaterialType::Lambertian>{.position{0.0f, +1.0f, 0.0f}, .normal{0.0f, 0.0f, -1.0f}, .color{0, 255, 0, 255} },
		Vertex<MaterialType::Lambertian>{.position{+1.0f, -1.0f, 0.0f}, .normal{0.0f, 0.0f, -1.0f}, .color{0, 255, 0, 255} }
	} };
	auto axes = this->_engine.createPrimitives<MaterialType::Simple, PrimitiveType::Line>();
	axes.setVertexData(axesData);
	auto triangle = this->_engine.createPrimitives<MaterialType::Lambertian, PrimitiveType::Triangle>();
	triangle.setVertexData(triangleData);
	Surface<MaterialType::Lambertian> rayCastingMap = this->_engine.createSurface<MaterialType::Lambertian>();
	/*using vec4ub = jjyou::glsl::vec<unsigned char, 4>;
	using vec4f = jjyou::glsl::vec<float, 4>;
	std::array<vec4ub, 9> surfaceColorData{ {
		vec4ub(255, 0, 0, 255), vec4ub(0, 255, 0, 255), vec4ub(0, 0, 255, 255),
		vec4ub(0, 0, 255, 255), vec4ub(255, 0, 0, 255), vec4ub(0, 255, 0, 255),
		vec4ub(0, 255, 0, 255), vec4ub(0, 0, 255, 255), vec4ub(255, 0, 0, 255),
	} };
	std::array<float, 9> surfaceDepthData{ {
		8.0f, 6.0f, 4.0f,
		4.0f, 8.0f, 6.0f,
		6.0f, 4.0f, 8.0f,
	} };
	surface.createTextures(
		{ {vk::Extent2D(3, 3), vk::Extent2D(3, 3)} },
		std::array<const void*, 2>{{surfaceColorData.data(), surfaceDepthData.data()}}
	);*/
	while (!this->_engine.window().windowShouldClose()) {
		// Resize the ray casting map if its size does not match the window framebuffer
		std::pair<int, int> framebufferSize = this->_engine.window().framebufferSize();
		vk::Extent2D framebufferExtent = vk::Extent2D(static_cast<std::uint32_t>(framebufferSize.first), static_cast<std::uint32_t>(framebufferSize.second));
		if (rayCastingMap.texture(0).extent() != framebufferExtent)
			rayCastingMap.createTextures({ {framebufferExtent, framebufferExtent, framebufferExtent} });
		// Ray casting for visualization
		jjyou::glsl::mat3 projection{ 1.0f };
		{
			float aspectRatio = static_cast<float>(framebufferExtent.width) / static_cast<float>(framebufferExtent.height);
			float tanHalfYFov = std::tan(std::numbers::pi_v<float> / 3.0f / 2.0f);
			float tanHalfXFov = aspectRatio * tanHalfYFov;
			projection[0][0] = 1.0f / tanHalfXFov * static_cast<float>(framebufferExtent.width) / 2.0f; //fx
			projection[1][1] = 1.0f / tanHalfYFov * static_cast<float>(framebufferExtent.height) / 2.0f; //fy
			projection[2][0] = static_cast<float>(framebufferExtent.width) / 2.0f; //cx
			projection[2][1] = static_cast<float>(framebufferExtent.height) / 2.0f; //cy
		}
		
		this->_kinectFusion.rayCasting(
			rayCastingMap,
			projection,
			this->_engine.window().getViewMatrix(),
			0.01f, 100.0f,
			std::nullopt,
			100000.0f
		);
		this->_engine.prepareFrame();
		ImGui::ShowDemoWindow(nullptr);
		this->_engine.drawPrimitives(axes, jjyou::glsl::mat4(1.0f));
		this->_engine.drawPrimitives(triangle, jjyou::glsl::mat4(1.0f));
		this->_engine.drawSurface(rayCastingMap);
		this->_engine.recordCommandbuffer();
		this->_engine.presentFrame();
		this->_engine.window().pollEvents();
	}
	this->_engine.waitIdle();
}