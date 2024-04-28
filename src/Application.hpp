#pragma once
#include "Engine.hpp"
#include "KinectFusion.hpp"
#include "DataLoader.hpp"
#include <memory>

/***********************************************************************
 * @class	Application
 * @brief	Application class that connects Engine, KinectFusion and DataLoader.
 ***********************************************************************/
class Application {

public:

	/** @brief	Construct with CLI arguments.
	  */
	Application(int argc_, char** argv_);

	/** @brief	Enter mainloop.
	  */
	void mainLoop(void);

	/** @brief	Disable copy/move constructor/assignment.
	  */
	Application(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(const Application&) = delete;
	Application& operator=(Application&&) = delete;

	/** @brief	Destructor.
	  */
	~Application(void) {
		this->_pEngine->waitIdle();
	}

private:

	bool _headlessMode = false;
	bool _debugMode = false;
	struct Arguments {
		float sigmaColor{};
		float sigmaSpace{};
		int filterKernelSize{};
		float distanceThreshold{};
		float angleThreshold{};
	} _arguments{};
	std::unique_ptr<Engine> _pEngine{};
	std::unique_ptr<DataLoader> _pDataLoader{};
	std::unique_ptr<KinectFusion> _pKinectFusion{};
	std::string _physicalDeviceName{};
	Primitives<MaterialType::Simple, PrimitiveType::Line> _axis{ nullptr };
	Primitives<MaterialType::Lambertian, PrimitiveType::Triangle> _arSphere{ nullptr };
	std::vector<Primitives<MaterialType::Simple, PrimitiveType::Line>> _cameraFrames{};
	std::vector<Primitives<MaterialType::Simple, PrimitiveType::Line>> _grayCameraFrames{}; // For groundtruth visualization
	std::vector<Surface<MaterialType::Simple>> _inputMaps{};
	std::vector<Surface<MaterialType::Lambertian>> _rayCastingMaps{};
	std::vector<Surface<MaterialType::Simple>> _arSurfaces{};

	void _initAssets(void);
	static void _updateCameraFrame(
		Primitives<MaterialType::Simple, PrimitiveType::Line>& cameraFrame_,
		Primitives<MaterialType::Simple, PrimitiveType::Line>& grayCameraFrame_,
		const Camera& camera_
	);
};