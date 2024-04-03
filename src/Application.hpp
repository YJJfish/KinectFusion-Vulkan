#pragma once
#include "Window.hpp"
#include "Engine.hpp"
#include "KinectFusion.hpp"

class Application {

public:

	Application(void) {

	}

private:

	Engine _engine;
	Window _window;
	KinectFusion _kinectFusion;
};