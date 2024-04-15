#include "Application.hpp"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <numbers>

Application::Application(void) :
	_headlessMode(false),
	_debugMode(true)
{
	// Load dataset
	/*std::filesystem::path baseDir = "E:/Courses/16-833/living_room_traj2_frei_png/";
	
	ImageFolder tum = ImageFolder(
		baseDir / "rgb",
		baseDir / "depth",
		50000.0f,
		jjyou::glsl::mat3(1.0f),
		std::nullopt,
		0.0f,
		100.0f,
		0.0f
	);*/
	this->_pDataLoader.reset(new VirtualDataLoader(
		vk::Extent2D(128, 128),
		jjyou::glsl::vec3(0.0f, 0.0f, 0.0f),
		0.5f
	));

	// Create Vulkan engine
	this->_pEngine.reset(new Engine(this->_headlessMode, this->_debugMode));

	// Create KinectFusion
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

	// Init assets
	this->_initAssets();
}

void Application::mainLoop(void) {
	std::uint32_t frameIndex = 0;
	FrameData frameData{};
	// UI
	struct {
		struct {
			bool drawARSphere = false;
			jjyou::glsl::vec3 position{};
			float scale = 0.2f;
			bool reset = false;
		} ar;
		struct {
			bool resetVolume = false;
		} fusion;
		struct {
			bool rayCasting = true;
			bool trackCamera = false;
		} visualization;
	} ui;
	// Main loop
	while (!this->_pEngine->window().windowShouldClose()) {
		// Prepare the new frame
		frameData = this->_pDataLoader->getFrame();
		this->_pEngine->prepareFrame();
		// Draw UI
		if (ImGui::Begin("KinectFusion")) {
			if (ImGui::TreeNode("AR")) {
				ImGui::Checkbox("Draw AR sphere", &ui.ar.drawARSphere);
				ImGui::SliderFloat3("Position", ui.ar.position.data.data(), -5.0f, 5.0f);
				ImGui::SliderFloat("Scale", &ui.ar.scale, 0.1f, 1.0f);
				if (ImGui::Button("Reset")) {
					ui.ar.reset = true;
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Fusion")) {
				if (ImGui::Button("Reset volume")) {
					ui.fusion.resetVolume = true;
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Visualization")) {
				ImGui::Checkbox("Ray casting", &ui.visualization.rayCasting);
				ImGui::Checkbox("Track camera", &ui.visualization.trackCamera);
				ImGui::TreePop();
			}
		}
		ImGui::End();
		// Upload the new frame
		this->_inputMaps[frameIndex].createTextures(
			{ {this->_pDataLoader->colorFrameExtent(), this->_pDataLoader->depthFrameExtent()} },
			{ {frameData.colorMap, frameData.depthMap} },
			false
		);
		// Fuse the new frame
		this->_pKinectFusion->fuse(
			this->_inputMaps[frameIndex],
			frameData.projection,
			*frameData.view
		);
		// Adjust camera frame vertices
		
		// Draw AR sphere
		if (ui.ar.reset) {
			ui.ar.reset = false;
			ui.ar.position = jjyou::glsl::vec3(0.0f);
			ui.ar.scale = 0.0f;
		}
		if (ui.ar.drawARSphere) {
			jjyou::glsl::mat4 model(1.0);
			model[0][0] = model[1][1] = model[2][2] = ui.ar.scale;
			model[3] += jjyou::glsl::vec4(ui.ar.position, 0.0f);
			this->_pEngine->drawPrimitives(this->_arSphere, model);
		}
		if (ui.fusion.resetVolume) {
			ui.fusion.resetVolume = false;
			this->_pKinectFusion->initTSDFVolume();
		}
		// Ray casting for visualization
		if (ui.visualization.rayCasting) {
			// Resize the ray casting map if its size does not match the window framebuffer
			std::pair<int, int> framebufferSize = this->_pEngine->window().framebufferSize();
			vk::Extent2D rayCastingExtent = vk::Extent2D(static_cast<std::uint32_t>(framebufferSize.first), static_cast<std::uint32_t>(framebufferSize.second));
			if (this->_rayCastingMaps[frameIndex].texture(0).extent() != rayCastingExtent)
				this->_rayCastingMaps[frameIndex].createTextures(
					{ {rayCastingExtent, rayCastingExtent, rayCastingExtent} },
					std::nullopt,
					false
				);
			// Ray casting for visualization
			jjyou::glsl::mat3 projection = jjyou::glsl::pinhole(std::numbers::pi_v<float> / 3.0f, rayCastingExtent.width, rayCastingExtent.height);
			this->_pKinectFusion->rayCasting(
				this->_rayCastingMaps[frameIndex],
				projection,
				this->_pEngine->window().getViewMatrix(),
				0.01f, 100.0f,
				100000.0f,
				std::nullopt
			);
			this->_pEngine->drawSurface(this->_rayCastingMaps[frameIndex]);
		}
		// Draw world space axis
		this->_pEngine->drawPrimitives(this->_axis, jjyou::glsl::mat4(1.0f));
		// Draw camera space axis
		this->_pEngine->drawPrimitives(this->_axis, jjyou::glsl::inverse(*frameData.view));
		// Record command buffer and present frame.
		this->_pEngine->recordCommandbuffer();
		this->_pEngine->presentFrame();
		this->_pEngine->window().pollEvents();
		frameIndex = (frameIndex + 1) % Engine::NUM_FRAMES_IN_FLIGHT;
	}
}

void Application::_initAssets(void) {
	// Axis
	{
		std::array<Vertex<MaterialType::Simple>, 6> axisData = { {
			Vertex<MaterialType::Simple>{.position{0.0f, 0.0f, 0.0f}, .color{255, 0, 0, 255} },
			Vertex<MaterialType::Simple>{.position{1.0f, 0.0f, 0.0f}, .color{255, 0, 0, 255} },
			Vertex<MaterialType::Simple>{.position{0.0f, 0.0f, 0.0f}, .color{0, 255, 0, 255} },
			Vertex<MaterialType::Simple>{.position{0.0f, 1.0f, 0.0f}, .color{0, 255, 0, 255} },
			Vertex<MaterialType::Simple>{.position{0.0f, 0.0f, 0.0f}, .color{0, 0, 255, 255} },
			Vertex<MaterialType::Simple>{.position{0.0f, 0.0f, 1.0f}, .color{0, 0, 255, 255} },
		} };
		this->_axis = this->_pEngine->createPrimitives<MaterialType::Simple, PrimitiveType::Line>();
		this->_axis.setVertexData(axisData, false);
	}
	// AR sphere
	{
		constexpr std::uint32_t numLattitudeSplits = 90U;
		constexpr std::uint32_t numLongtitudeSplits = 45U;
		std::vector<Vertex<MaterialType::Lambertian>> sphereData{};
		sphereData.reserve(static_cast<std::size_t>(numLattitudeSplits) * static_cast<std::size_t>(numLongtitudeSplits) * 6ULL);
		for (std::uint32_t x = 0U; x < numLattitudeSplits; ++x) {
			float thetaCurr = static_cast<float>(x) / static_cast<float>(numLattitudeSplits) * 2.0f * std::numbers::pi_v<float>;
			float thetaNext = static_cast<float>(x + 1U) / static_cast<float>(numLattitudeSplits) * 2.0f * std::numbers::pi_v<float>;
			for (std::uint32_t y = 0U; y < numLongtitudeSplits; ++y) {
				float phiCurr = static_cast<float>(y) / static_cast<float>(numLongtitudeSplits) * std::numbers::pi_v<float>;
				float phiNext = static_cast<float>(y + 1U) / static_cast<float>(numLongtitudeSplits) * std::numbers::pi_v<float>;
				std::array<Vertex<MaterialType::Lambertian>, 4> vertices{ {
					Vertex<MaterialType::Lambertian>{
						.position = {std::sin(phiCurr) * std::cos(thetaCurr), std::cos(phiCurr), std::sin(phiCurr) * std::sin(thetaCurr)},
						.normal = {},
						.color = {255, 0, 0, 255}
					},
					Vertex<MaterialType::Lambertian>{
						.position = {std::sin(phiCurr) * std::cos(thetaNext), std::cos(phiCurr), std::sin(phiCurr) * std::sin(thetaNext)},
						.normal = {},
						.color = {255, 0, 0, 255}
					},
					Vertex<MaterialType::Lambertian>{
						.position = {std::sin(phiNext) * std::cos(thetaNext), std::cos(phiNext), std::sin(phiNext) * std::sin(thetaNext)},
						.normal = {},
						.color = {255, 0, 0, 255}
					},
					Vertex<MaterialType::Lambertian>{
						.position = {std::sin(phiNext) * std::cos(thetaCurr), std::cos(phiNext), std::sin(phiNext) * std::sin(thetaCurr)},
						.normal = {},
						.color = {255, 0, 0, 255}
					}
				} };
				for (auto& vertex : vertices)
					vertex.normal = vertex.position;
				for (const std::size_t& index : { 0ULL, 1ULL, 2ULL, 0ULL, 2ULL, 3ULL })
					sphereData.push_back(vertices[index]);
			}
		}
		this->_arSphere = this->_pEngine->createPrimitives<MaterialType::Lambertian, PrimitiveType::Triangle>();
		this->_arSphere.setVertexData(sphereData, false);
	}
	// Camera frames
	{
		this->_cameraFrames.reserve(static_cast<std::size_t>(Engine::NUM_FRAMES_IN_FLIGHT));
		for (std::uint32_t i = 0; i < Engine::NUM_FRAMES_IN_FLIGHT; ++i) {
			this->_cameraFrames.push_back(this->_pEngine->createPrimitives<MaterialType::Simple, PrimitiveType::Line>());
		}
	}

	// Input maps
	{
		this->_inputMaps.reserve(static_cast<std::size_t>(Engine::NUM_FRAMES_IN_FLIGHT));
		for (std::uint32_t i = 0; i < Engine::NUM_FRAMES_IN_FLIGHT; ++i) {
			this->_inputMaps.push_back(this->_pEngine->createSurface<MaterialType::Simple>());
			this->_inputMaps.back().createTextures(
				{ {this->_pDataLoader->colorFrameExtent(), this->_pDataLoader->depthFrameExtent()} },
				std::nullopt,
				false
			);
		}
	}

	// Ray casting maps
	{
		std::pair<int, int> framebufferSize = this->_pEngine->window().framebufferSize();
		vk::Extent2D rayCastingExtent = vk::Extent2D(static_cast<std::uint32_t>(framebufferSize.first), static_cast<std::uint32_t>(framebufferSize.second));
		this->_rayCastingMaps.reserve(static_cast<std::size_t>(Engine::NUM_FRAMES_IN_FLIGHT));
		for (std::uint32_t i = 0; i < Engine::NUM_FRAMES_IN_FLIGHT; ++i) {
			this->_rayCastingMaps.push_back(this->_pEngine->createSurface<MaterialType::Lambertian>());
			this->_rayCastingMaps.back().createTextures(
				{ {rayCastingExtent, rayCastingExtent, rayCastingExtent} },
				std::nullopt,
				false
			);
		}
	}
}