#pragma once
#include "Engine.hpp"
#include "KinectFusion.hpp"

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

private:

	bool _headlessMode = false;
	bool _debugMode = true;
	Engine _engine;
	KinectFusion _kinectFusion;
};