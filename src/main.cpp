#include "Window.hpp"
#include "Engine.hpp"
#include "KinectFusion.hpp"
int main() {
	Engine engine(false, true);
	TSDFVolume volume(engine.context(), engine.allocator(), engine.descriptorPool(), { 512U,512U,512U }, 0.005f);
	while (!engine.window().windowShouldClose()) {
		engine.prepareFrame();
		engine.recordCommandBuffersDemo();
		engine.presentFrame();
		engine.window().pollEvents();
	}
}