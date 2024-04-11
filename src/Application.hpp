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
	KinectFusion _kinectFusion;
};