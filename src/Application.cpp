#include "Application.hpp"
#include "Camera.hpp"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <numbers>
#include <exception>
#include <stdexcept>
#include <chrono>
#include <argparse/argparse.hpp>

Application::Application(int argc_, char** argv_)
{
	// Parse arguments.
	argparse::ArgumentParser argumentParser("KinectFusion-Vulkan", "1.0");
	// Input dataset.
	argumentParser
		.add_argument("--dataset")
		.help("Input dataset. Supported: \"VirtualDataLoader\", \"TUM\".")
		.default_value("VirtualDataLoader");
	// Parameters of VirtualDataLoader.
	argumentParser
		.add_argument("--VirtualDataLoader.extent")
		.help("The frame extent of VirtualDataLoader.")
		.nargs(2)
		.scan<'i', int>()
		.default_value(std::vector<int>{128, 128});
	argumentParser
		.add_argument("--VirtualDataLoader.center")
		.help("The center position of the cube in VirtualDataLoader.")
		.nargs(3)
		.scan<'g', float>()
		.default_value(std::vector<float>{0.0f, 0.0f, 0.0f});
	argumentParser
		.add_argument("--VirtualDataLoader.length")
		.help("The edge length of the cube in VirtualDataLoader.")
		.nargs(1)
		.scan<'g', float>()
		.default_value(0.5f);
	// Parameters of TUM.
	argumentParser
		.add_argument("--TUM.path")
		.help("Path to the folder of TUM RGB-D dataset.");
	// Application settings.
	argumentParser.add_argument("--debug")
		.help("Enable headless mode.")
		.flag();
	argumentParser.add_argument("--headless")
		.help("Enable debug mode.")
		.flag();
	// KinectFusion parameters.
	argumentParser
		.add_argument("--truncation-weight")
		.help("The truncation weight in fusion stage.")
		.nargs(1)
		.scan<'i', int>()
		.default_value(100);
	argumentParser
		.add_argument("--volume-resolution")
		.help("The resolution of TSDF volume.")
		.nargs(3)
		.scan<'i', int>()
		.default_value(std::vector<int>{512, 512, 512});
	argumentParser
		.add_argument("--volume-size")
		.help("The size of voxels in TSDF volume.")
		.nargs(1)
		.scan<'g', float>()
		.default_value(0.02f);
	argumentParser
		.add_argument("--volume-corner")
		.help("The coordinate of the corner voxel's center point. By default, the volume will be placed such that its center point is at the origin.")
		.nargs(3)
		.scan<'g', float>();
	argumentParser
		.add_argument("--truncation-distance")
		.help("The truncation distance of TSDF. By default, it is 3x the voxel size.")
		.nargs(1)
		.scan<'g', float>();
	argumentParser
		.add_argument("--sigma-color")
		.help("The sigma color term in bilateral filtering.")
		.nargs(1)
		.scan<'g', float>()
		.default_value(0.75f);
	argumentParser
		.add_argument("--sigma-space")
		.help("The sigma space term in bilateral filtering.")
		.nargs(1)
		.scan<'g', float>()
		.default_value(0.75f);
	argumentParser
		.add_argument("--filter-kernel-size")
		.help("The kernel size of bilateral filtering. Must be an odd number.")
		.nargs(1)
		.scan<'i', int>()
		.default_value(5);
	argumentParser
		.add_argument("--distance-threshold")
		.help("The distance threshold used in projective correspondence search in ICP.")
		.nargs(1)
		.scan<'g', float>()
		.default_value(0.08f);
	argumentParser
		.add_argument("--angle-threshold")
		.help("The angle threshold used in projective correspondence search in ICP.")
		.nargs(1)
		.scan<'g', float>()
		.default_value(std::numbers::pi_v<float> / 15.0f);
	argumentParser.parse_args(argc_, argv_);

	// Set application mode.
	if (argumentParser.get<bool>("--debug"))
		this->_debugMode = true;
	if (argumentParser.get<bool>("--headless"))
		this->_headlessMode = true;

	// Load dataset
	if (argumentParser.get<std::string>("--dataset") == "VirtualDataLoader") {
		std::vector<int> extent = argumentParser.get<std::vector<int>>("--VirtualDataLoader.extent");
		std::vector<float> center = argumentParser.get<std::vector<float>>("--VirtualDataLoader.center");
		float length = argumentParser.get<float>("--VirtualDataLoader.length");
		this->_pDataLoader.reset(new VirtualDataLoader(
			vk::Extent2D(static_cast<std::uint32_t>(extent[0]), static_cast<std::uint32_t>(extent[1])),
			jjyou::glsl::vec3(center[0], center[1], center[2]),
			length
		));
	}
	else if (argumentParser.get<std::string>("--dataset") == "TUM") {
		std::optional<std::string> path = argumentParser.present<std::string>("--TUM.path");
		if (!path.has_value()) {
			throw std::logic_error("[Application] Please specify the path to the TUM dataset by \"--TUM.path\".");
		}
		this->_pDataLoader.reset(new TUMDataset(
			*path
		));
	}
	else {
		throw std::logic_error("[Application] Unsupported dataset " + argumentParser.get<std::string>("--dataset") + ".");
	}

	// Create Vulkan engine
	this->_pEngine.reset(new Engine(this->_headlessMode, this->_debugMode));
	this->_physicalDeviceName = std::string(this->_pEngine->context().physicalDevice().getProperties().deviceName.data());

	// Create KinectFusion
	int truncationWeight = argumentParser.get<int>("--truncation-weight");
	std::vector<int> _volumeResolution = argumentParser.get<std::vector<int>>("--volume-resolution");
	jjyou::glsl::uvec3 volumeResolution(
		static_cast<std::uint32_t>(_volumeResolution[0]),
		static_cast<std::uint32_t>(_volumeResolution[1]),
		static_cast<std::uint32_t>(_volumeResolution[2])
	);
	float volumeSize = argumentParser.get<float>("--volume-size");
	std::optional<std::vector<float>> _volumeCorner = argumentParser.present<std::vector<float>>("--volume-corner");
	std::optional<jjyou::glsl::vec3> volumeCorner;
	if (_volumeCorner.has_value())
		volumeCorner = jjyou::glsl::vec3((*_volumeCorner)[0], (*_volumeCorner)[1], (*_volumeCorner)[2]);
	std::optional<float> truncationDistance = argumentParser.present<float>("--truncation-distance");
	this->_pKinectFusion.reset(new KinectFusion(
		*this->_pEngine,
		this->_pDataLoader->colorFrameExtent(),
		this->_pDataLoader->depthFrameExtent(),
		static_cast<std::int16_t>(truncationWeight),
		this->_pDataLoader->minDepth(),
		this->_pDataLoader->maxDepth(),
		this->_pDataLoader->invalidDepth(),
		volumeResolution,
		volumeSize,
		volumeCorner,
		truncationDistance
	));

	// Init assets
	this->_initAssets();

	// Store other arguments
	this->_arguments.sigmaColor = argumentParser.get<float>("--sigma-color");
	this->_arguments.sigmaSpace = argumentParser.get<float>("--sigma-space");
	this->_arguments.filterKernelSize = argumentParser.get<int>("--filter-kernel-size");
	this->_arguments.distanceThreshold = argumentParser.get<float>("--distance-threshold");
	this->_arguments.angleThreshold = argumentParser.get<float>("--angle-threshold");
}

void Application::mainLoop(void) {
	std::uint32_t resourceCycleCounter = 0;
	bool firstFrame = true;
	jjyou::glsl::mat4 lastFrameView{};
	jjyou::glsl::mat4 currFrameView{};
	FrameData frameData{};
	bool eof = false;
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
			bool trackCamera = true;
			bool displayInputFrames = false;
			bool drawGTCamera = false;
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
		vk::Result prepareFrameResult = this->_pEngine->prepareFrame();
		if (prepareFrameResult != vk::Result::eSuccess)
			continue;

		// Fetch data
		if (!eof) {
			frameData = this->_pDataLoader->getFrame();
		}
		if (frameData.state == FrameState::Eof) {
			eof = true;
		}

		// Draw UI
		if (ImGui::Begin("KinectFusion-Vulkan")) {
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
				ImGui::Checkbox("Track camera", &ui.visualization.trackCamera);
				ImGui::Checkbox("Display input frames", &ui.visualization.displayInputFrames);
				ImGui::Checkbox("Draw groundtruth camera", &ui.visualization.drawGTCamera);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Info")) {
				ImGui::Text("Device name: %s", this->_physicalDeviceName.c_str());
				ImGui::Text("Frame index: %d", frameData.frameIndex);
				ImGui::Text("Frame state: %s", to_string(frameData.state).c_str());
				ImGui::Text("FPS: %d", fps);
				ImGui::TreePop();
			}
		}
		ImGui::End();

		// Process the new frame
		if (!eof && frameData.state != FrameState::Invalid) {
			// Upload the new frame
			this->_inputMaps[resourceCycleCounter].createTextures(
				{ {this->_pDataLoader->colorFrameExtent(), this->_pDataLoader->depthFrameExtent()} },
				{ {frameData.colorMap, frameData.depthMap} },
				false
			);
			// Estimate the camera pose
			if (!firstFrame) {
				std::optional<jjyou::glsl::mat4> estimatedView = this->_pKinectFusion->estimatePose(
					this->_inputMaps[resourceCycleCounter],
					frameData.camera,
					lastFrameView,
					this->_arguments.sigmaColor,
					this->_arguments.sigmaSpace,
					this->_arguments.filterKernelSize,
					this->_arguments.distanceThreshold,
					this->_arguments.angleThreshold
				);
				if (estimatedView.has_value())
					currFrameView = *estimatedView;
			}
			else {
				currFrameView = this->_pDataLoader->initialPose();
			}
			// Fuse the new frame
			this->_pKinectFusion->fuse(
				this->_inputMaps[resourceCycleCounter],
				frameData.camera,
				currFrameView
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
				currFrameView,
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
			10000.0f,
			std::nullopt
		);

		// Display ray casting maps or input frames
		if (!ui.visualization.displayInputFrames) {
			this->_pEngine->drawSurface(this->_rayCastingMaps[resourceCycleCounter]);
		}
		else {
			this->_arSurfaces[resourceCycleCounter].connect(
				this->_inputMaps[resourceCycleCounter],
				this->_rayCastingMaps[resourceCycleCounter]
			);
			this->_pEngine->drawSurface(this->_arSurfaces[resourceCycleCounter]);
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

		this->_updateCameraFrame(this->_cameraFrames[resourceCycleCounter], this->_grayCameraFrames[resourceCycleCounter], frameData.camera);
		// Draw camera space axis and camera frame
		if (!ui.visualization.trackCamera && !ui.visualization.displayInputFrames) {
			this->_pEngine->drawPrimitives(this->_axis, jjyou::glsl::inverse(currFrameView) * jjyou::glsl::mat4(jjyou::glsl::mat3(0.2f)));
			this->_pEngine->drawPrimitives(this->_cameraFrames[resourceCycleCounter], jjyou::glsl::inverse(currFrameView) * jjyou::glsl::mat4(jjyou::glsl::mat3(0.2f)));
		}

		// Draw GT camera space axis and camera frame
		if (ui.visualization.drawGTCamera && frameData.view.has_value()) {
			this->_pEngine->drawPrimitives(this->_axis, jjyou::glsl::inverse(*frameData.view) * jjyou::glsl::mat4(jjyou::glsl::mat3(0.2f)));
			this->_pEngine->drawPrimitives(this->_grayCameraFrames[resourceCycleCounter], jjyou::glsl::inverse(*frameData.view) * jjyou::glsl::mat4(jjyou::glsl::mat3(0.2f)));
		}

		// Record command buffer and present frame.
		this->_pEngine->recordCommandbuffer();
		this->_pEngine->presentFrame();
		this->_pEngine->window().pollEvents();
		if (!eof)
			resourceCycleCounter = (resourceCycleCounter + 1) % Engine::NUM_FRAMES_IN_FLIGHT;
		firstFrame = false;
		lastFrameView = currFrameView;
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
		this->_grayCameraFrames.reserve(static_cast<std::size_t>(Engine::NUM_FRAMES_IN_FLIGHT));
		for (std::uint32_t i = 0; i < Engine::NUM_FRAMES_IN_FLIGHT; ++i) {
			this->_cameraFrames.push_back(this->_pEngine->createPrimitives<MaterialType::Simple, PrimitiveType::Line>(MemoryPattern::Dynamic));
			this->_grayCameraFrames.push_back(this->_pEngine->createPrimitives<MaterialType::Simple, PrimitiveType::Line>(MemoryPattern::Dynamic));
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

	// AR surfaces
	{
		this->_arSurfaces.reserve(static_cast<std::size_t>(Engine::NUM_FRAMES_IN_FLIGHT));
		for (std::uint32_t i = 0; i < Engine::NUM_FRAMES_IN_FLIGHT; ++i) {
			this->_arSurfaces.push_back(this->_pEngine->createSurface<MaterialType::Simple>());
		}
	}
}

void Application::_updateCameraFrame(
	Primitives<MaterialType::Simple, PrimitiveType::Line>& cameraFrame_,
	Primitives<MaterialType::Simple, PrimitiveType::Line>& grayCameraFrame_,
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
	for (auto& vertex : vertexBuffer)
		vertex.color = jjyou::glsl::vec<unsigned char, 4>(100, 100, 100, 255);
	grayCameraFrame_.setVertexData(vertexBuffer, false);
}