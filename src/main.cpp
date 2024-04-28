#include "Application.hpp"

int main(int argc, char** argv) {
	Application application(argc, argv);
	application.mainLoop();
	return 0;
}