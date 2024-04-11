#include "Engine.hpp"
#include "KinectFusion.hpp"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
int main() {
	Engine engine(false, true);
	TSDFVolume volume(engine, { 512U,512U,512U }, 0.005f);
	volume.reset();
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
	auto axes = engine.createPrimitives<MaterialType::Simple, PrimitiveType::Line>();
	axes.setVertexData(axesData);
	auto triangle = engine.createPrimitives<MaterialType::Lambertian, PrimitiveType::Triangle>();
	triangle.setVertexData(triangleData);
	auto surface = engine.createSurface<MaterialType::Simple>();
	using vec4ub = jjyou::glsl::vec<unsigned char, 4>;
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
	);
	while (!engine.window().windowShouldClose()) {
		engine.prepareFrame();
		ImGui::ShowDemoWindow(nullptr);
		engine.drawPrimitives(axes, jjyou::glsl::mat4(1.0f));
		engine.drawPrimitives(triangle, jjyou::glsl::mat4(1.0f));
		engine.drawSurface(surface);
		engine.recordCommandbuffer();
		engine.presentFrame();
		engine.window().pollEvents();
	}
	engine.waitIdle();
}