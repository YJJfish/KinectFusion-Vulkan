#include "Window.hpp"
#include "Engine.hpp"
#include "KinectFusion.hpp"
int main() {
	Engine engine(false, true);
	while (!engine.window().windowShouldClose()) {
		engine.prepareFrame();
		engine.recordCommandBuffersDemo();
		engine.presentFrame();
		engine.window().pollEvents();
	}
}