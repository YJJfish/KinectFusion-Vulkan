#pragma once
#include "Engine.hpp"
#include "KinectFusion.hpp"
#include "DataLoader.hpp"
#include <memory>

class Application {

public:

	/** @brief	Constructor.
	  */
	Application(void);

	/** @brief	Disable copy/move constructor/assignment.
	  */
	Application(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(const Application&) = delete;
	Application& operator=(Application&&) = delete;

	/** @brief	Destructor.
	  */
	~Application(void) = default;

private:


	bool _headlessMode = false;
	bool _debugMode = true;
	std::unique_ptr<Engine> _pEngine{};
	std::unique_ptr<DataLoader> _pDataLoader{};
	std::unique_ptr<KinectFusion> _pKinectFusion{};
};