#include "Application.hpp"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

Application::Application(void) :
	_headlessMode(false),
	_debugMode(true)
{
	this->_pEngine.reset(new Engine(this->_headlessMode, this->_debugMode));
	this->_pDataLoader.reset(new VirtualDataLoader(
		vk::Extent2D(128, 128),
		jjyou::glsl::vec3(0.0f, 0.0f, 0.0f),
		0.5f
	));
	this->_pKinectFusion.reset(new KinectFusion(
		*this->_pEngine,
		this->_pDataLoader->colorFrameExtent(),
		this->_pDataLoader->depthFrameExtent(),
		100.0f,
		this->_pDataLoader->minDepth(),
		this->_pDataLoader->maxDepth(),
		this->_pDataLoader->invalidDepth(),
		jjyou::glsl::uvec3(512U, 512U, 512U),
		0.005f
	));
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
	auto axes = this->_pEngine->createPrimitives<MaterialType::Simple, PrimitiveType::Line>();
	axes.setVertexData(axesData);
	auto triangle = this->_pEngine->createPrimitives<MaterialType::Lambertian, PrimitiveType::Triangle>();
	triangle.setVertexData(triangleData);
	Surface<MaterialType::Lambertian> rayCastingMap = this->_pEngine->createSurface<MaterialType::Lambertian>();
	Surface<MaterialType::Simple> inputMap = this->_pEngine->createSurface<MaterialType::Simple>();
	
	/*for (int i = 0; i < 10; ++i)
		FrameData frameData = this->_pDataLoader->getFrame();
	FrameData frameData = this->_pDataLoader->getFrame();
	inputMap.createTextures(
		{ {this->_pDataLoader->colorFrameExtent(), this->_pDataLoader->depthFrameExtent()} },
		{ {frameData.colorMap, frameData.depthMap} }
	);*/
	
	while (!this->_pEngine->window().windowShouldClose()) {
		// Upload the new frame
		FrameData frameData = this->_pDataLoader->getFrame();
		inputMap.createTextures(
			{ {this->_pDataLoader->colorFrameExtent(), this->_pDataLoader->depthFrameExtent()} },
			{ {frameData.colorMap, frameData.depthMap} }
		);
		// Fuse the new frame
		this->_pKinectFusion->fuse(
			inputMap,
			frameData.projection,
			*frameData.view
		);
		// Resize the ray casting map if its size does not match the window framebuffer
		std::pair<int, int> framebufferSize = this->_pEngine->window().framebufferSize();
		vk::Extent2D rayCastingExtent = vk::Extent2D(static_cast<std::uint32_t>(framebufferSize.first), static_cast<std::uint32_t>(framebufferSize.second));
		if (rayCastingMap.texture(0).extent() != rayCastingExtent)
			rayCastingMap.createTextures({ {rayCastingExtent, rayCastingExtent, rayCastingExtent} });
		// Ray casting for visualization
		jjyou::glsl::mat3 projection = jjyou::glsl::pinhole(std::numbers::pi_v<float> / 3.0f, rayCastingExtent.width, rayCastingExtent.height);
		this->_pKinectFusion->rayCasting(
			rayCastingMap,
			projection,
			this->_pEngine->window().getViewMatrix(),
			0.01f, 100.0f,
			100000.0f,
			std::nullopt
		);
		this->_pEngine->prepareFrame();
		ImGui::ShowDemoWindow(nullptr);
		this->_pEngine->drawPrimitives(axes, jjyou::glsl::mat4(1.0f));
		this->_pEngine->drawPrimitives(triangle, jjyou::glsl::mat4(1.0f));
		this->_pEngine->drawPrimitives(
			axes,
			jjyou::glsl::inverse(*frameData.view)
		);
		this->_pEngine->drawSurface(rayCastingMap);
		//this->_pEngine->drawSurface(inputMap);
		this->_pEngine->recordCommandbuffer();
		this->_pEngine->presentFrame();
		this->_pEngine->window().pollEvents();
	}
	this->_pEngine->waitIdle();
}