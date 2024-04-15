#pragma once
#include "Engine.hpp"
#include "KinectFusion.hpp"
#include "DataLoader.hpp"
#include <memory>

class Application {

public:

	/** @brief	Default constructor.
	  */
	Application(void);

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
	bool _debugMode = true;
	std::unique_ptr<Engine> _pEngine{};
	std::unique_ptr<DataLoader> _pDataLoader{};
	std::unique_ptr<KinectFusion> _pKinectFusion{};
	Primitives<MaterialType::Simple, PrimitiveType::Line> _axis{ nullptr };
	Primitives<MaterialType::Lambertian, PrimitiveType::Triangle> _arSphere{ nullptr };
	std::vector<Primitives<MaterialType::Simple, PrimitiveType::Line>> _cameraFrames{};
	std::vector<Surface<MaterialType::Simple>> _inputMaps{};
	std::vector<Surface<MaterialType::Lambertian>> _rayCastingMaps{};

	void _initAssets(void);
};