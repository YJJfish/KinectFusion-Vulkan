#include "Application.hpp"
#include "Camera.hpp"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <numbers>
#include <fstream>
#include <chrono>

Application::Application(void) :
	_headlessMode(false),
	_debugMode(true)
{
	// Load dataset
	/*this->_pDataLoader.reset(new VirtualDataLoader(
		vk::Extent2D(128, 128),
		jjyou::glsl::vec3(0.0f, 0.0f, 0.0f),
		0.5f
	));*/

	std::filesystem::path baseDir = "E:/Courses/16-833/living_room_traj2_frei_png/";
	std::ifstream extrinsicsFile(baseDir / "livingRoom2.gt.freiburg", std::ios::in);
	std::vector<jjyou::glsl::mat4> views;
	jjyou::glsl::mat4 invertY(1.0f); invertY[1][1] = -1.0f;
	for (std::uint32_t i = 0; i < 880; ++i) {
		std::uint32_t id{};
		jjyou::glsl::vec3 translation{};
		jjyou::glsl::quat rotation{};
		extrinsicsFile >> id >> translation.x >> translation.y >> translation.z >> rotation.x >> rotation.y >> rotation.z >> rotation.w;
		jjyou::glsl::mat4 view = jjyou::glsl::mat3(rotation);
		view[3] = jjyou::glsl::vec4(translation, 1.0f);
		view = jjyou::glsl::inverse(invertY * view * invertY);
		views.push_back(view);
	}
	float depthScale = 65535.0f / 5000.0f;
	this->_pDataLoader.reset(new ImageFolder(
		baseDir / "rgb_alphabetical",
		baseDir / "depth_alphabetical",
		depthScale,
		Camera::fromVision(418.2f, 480.0f, 319.5f, 239.5f, 0.01f, 100.0f, 640, 480),
		views,
		0.0f,
		100.0f,
		0.0f
	));

	// Create Vulkan engine
	this->_pEngine.reset(new Engine(this->_headlessMode, this->_debugMode));

	// Create KinectFusion
	this->_pKinectFusion.reset(new KinectFusion(
		*this->_pEngine,
		this->_pDataLoader->colorFrameExtent(),
		this->_pDataLoader->depthFrameExtent(),
		100,
		this->_pDataLoader->minDepth(),
		this->_pDataLoader->maxDepth(),
		this->_pDataLoader->invalidDepth(),
		jjyou::glsl::uvec3(512U, 512U, 512U),
		0.02f
	));

	// Init assets
	this->_initAssets();
}

void Application::mainLoop(void) {
	std::uint32_t resourceCycleCounter = 0;
	FrameData frameData{};
	std::chrono::steady_clock::time_point timer{};
	std::uint32_t numFramesSinceLastTimer = 0U;
	std::uint32_t fps = 0U;
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
			bool displayInputFrames = false;
		} visualization;
	} ui;

	// Main loop
	timer = std::chrono::steady_clock::now();
	while (!this->_pEngine->window().windowShouldClose()) {

		// Compute FPS
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(now - timer).count()) {
			timer = now;
			fps = numFramesSinceLastTimer;
			numFramesSinceLastTimer = 0U;
		}
		++numFramesSinceLastTimer;

		// Prepare the new frame
		if (frameData.state != FrameState::Eof) {
			frameData = this->_pDataLoader->getFrame();
		}
		vk::Result prepareFrameResult = this->_pEngine->prepareFrame();
		if (prepareFrameResult != vk::Result::eSuccess)
			continue;

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
				ImGui::Checkbox("Display input frames", &ui.visualization.displayInputFrames);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Info")) {
				ImGui::Text("Frame index: %d", frameData.frameIndex);
				ImGui::Text("Frame state: %s", to_string(frameData.state).c_str());
				ImGui::Text("FPS: %d", fps);
				ImGui::TreePop();
			}
		}
		ImGui::End();

		// Process the new frame
		if (frameData.state != FrameState::Eof && frameData.state != FrameState::Invalid) {
			// Upload the new frame
			this->_inputMaps[resourceCycleCounter].createTextures(
				{ {this->_pDataLoader->colorFrameExtent(), this->_pDataLoader->depthFrameExtent()} },
				{ {frameData.colorMap, frameData.depthMap} },
				false
			);
			// Fuse the new frame
			this->_pKinectFusion->fuse(
				this->_inputMaps[resourceCycleCounter],
				frameData.camera,
				*frameData.view
			);
		}

		// Reset the volume if requested
		if (ui.fusion.resetVolume) {
			ui.fusion.resetVolume = false;
			this->_pKinectFusion->initTSDFVolume();
		}

		// Track camera
		if (ui.visualization.trackCamera || ui.visualization.displayInputFrames) {
			this->_pEngine->setCameraMode(
				Window::CameraMode::Fixed,
				*frameData.view,
				frameData.camera
			);
		}
		else {
			this->_pEngine->setCameraMode(
				Window::CameraMode::Scene,
				std::nullopt,
				std::nullopt
			);
		}

		// Ray casting for visualization
		if (ui.visualization.rayCasting && !ui.visualization.displayInputFrames) {
			// Resize the ray casting map if its size does not match the window framebuffer
			Camera rayCastingCamera = this->_pEngine->getCamera();
			vk::Extent2D rayCastingExtent = vk::Extent2D(rayCastingCamera.width, rayCastingCamera.height);
			if (this->_rayCastingMaps[resourceCycleCounter].texture(0).extent() != rayCastingExtent)
				this->_rayCastingMaps[resourceCycleCounter].createTextures(
					{ {rayCastingExtent, rayCastingExtent, rayCastingExtent} },
					std::nullopt,
					false
				);
			// Ray casting
			this->_pKinectFusion->rayCasting(
				this->_rayCastingMaps[resourceCycleCounter],
				rayCastingCamera,
				this->_pEngine->window().getViewMatrix(),
				rayCastingCamera.zNear, rayCastingCamera.zFar,
				std::numeric_limits<float>::infinity(),
				std::nullopt
			);
			this->_pEngine->drawSurface(this->_rayCastingMaps[resourceCycleCounter]);
		}

		// Display input frames
		if (ui.visualization.displayInputFrames) {
			this->_pEngine->drawSurface(this->_inputMaps[resourceCycleCounter]);
		}

		// Draw AR sphere
		if (ui.ar.reset) {
			ui.ar.reset = false;
			ui.ar.position = jjyou::glsl::vec3(0.0f);
			ui.ar.scale = 0.2f;
		}
		if (ui.ar.drawARSphere) {
			jjyou::glsl::mat4 model(1.0);
			model[0][0] = model[1][1] = model[2][2] = ui.ar.scale;
			model[3] = jjyou::glsl::vec4(ui.ar.position, 1.0f);
			this->_pEngine->drawPrimitives(this->_arSphere, model);
		}

		// Draw world space axis
		this->_pEngine->drawPrimitives(this->_axis, jjyou::glsl::mat4(1.0f));

		// Draw camera space axis and camera space
		if (!ui.visualization.trackCamera && !ui.visualization.displayInputFrames) {
			this->_pEngine->drawPrimitives(this->_axis, jjyou::glsl::inverse(*frameData.view) * jjyou::glsl::mat4(jjyou::glsl::mat3(0.2f)));
			this->_updateCameraFrame(this->_cameraFrames[resourceCycleCounter], frameData.camera);
			this->_pEngine->drawPrimitives(this->_cameraFrames[resourceCycleCounter], jjyou::glsl::inverse(*frameData.view) * jjyou::glsl::mat4(jjyou::glsl::mat3(0.2f)));
		}

		// Record command buffer and present frame.
		this->_pEngine->recordCommandbuffer();
		this->_pEngine->presentFrame();
		this->_pEngine->window().pollEvents();
		resourceCycleCounter = (resourceCycleCounter + 1) % Engine::NUM_FRAMES_IN_FLIGHT;
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
		this->_axis = this->_pEngine->createPrimitives<MaterialType::Simple, PrimitiveType::Line>(MemoryPattern::Static);
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
		this->_arSphere = this->_pEngine->createPrimitives<MaterialType::Lambertian, PrimitiveType::Triangle>(MemoryPattern::Static);
		this->_arSphere.setVertexData(sphereData, false);
	}
	// Camera frames
	{
		this->_cameraFrames.reserve(static_cast<std::size_t>(Engine::NUM_FRAMES_IN_FLIGHT));
		for (std::uint32_t i = 0; i < Engine::NUM_FRAMES_IN_FLIGHT; ++i) {
			this->_cameraFrames.push_back(this->_pEngine->createPrimitives<MaterialType::Simple, PrimitiveType::Line>(MemoryPattern::Dynamic));
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

void Application::_updateCameraFrame(
	Primitives<MaterialType::Simple, PrimitiveType::Line>& cameraFrame_,
	const Camera& camera_
) {
	std::array<Vertex<MaterialType::Simple>, 5> vertices{ {
		Vertex<MaterialType::Simple>{.position = {0.0f, 0.0f, 0.0f}, .color = {255, 255, 255, 255}},
		Vertex<MaterialType::Simple>{.position = {-0.5f, -0.5f, 1.0f}, .color = {255, 255, 255, 255}},
		Vertex<MaterialType::Simple>{.position = {-0.5f, static_cast<float>(camera_.height) - 0.5f, 1.0f}, .color = {255, 255, 255, 255}},
		Vertex<MaterialType::Simple>{.position = {static_cast<float>(camera_.width) - 0.5f, static_cast<float>(camera_.height) - 0.5f, 1.0f}, .color = {255, 255, 255, 255}},
		Vertex<MaterialType::Simple>{.position = {static_cast<float>(camera_.width) - 0.5f, -0.5f, 1.0f}, .color = {255, 255, 255, 255}},
	} };
	jjyou::glsl::mat3 invProjection = jjyou::glsl::inverse(camera_.getVisionProjection());
	for (auto& v : vertices)
		v.position = invProjection * v.position;
	std::array<std::size_t, 16> indices = { {
		0, 1,
		0, 2,
		0, 3,
		0, 4,
		1, 2,
		2, 3,
		3, 4,
		4, 1
	} };
	std::array<Vertex<MaterialType::Simple>, 16> vertexBuffer{};
	for (std::size_t i = 0; i < indices.size(); ++i)
		vertexBuffer[i] = vertices[indices[i]];
	cameraFrame_.setVertexData(vertexBuffer, false);
}