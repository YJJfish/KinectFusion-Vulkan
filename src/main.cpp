#include "Window.hpp"
#include "Engine.hpp"
#include "KinectFusion.hpp"
int main() {
	Engine engine(true);
	Window window;
	window.createWindow(800, 600);
	engine.createContext();
	window.createSurface(engine.context().instance());
}