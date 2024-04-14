cmake_minimum_required (VERSION 3.7)
project(KinectFusion-Vulkan LANGUAGES CXX VERSION 1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Vulkan SDK
find_package(Vulkan REQUIRED)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(./dep/glfw/)

# ImGui
add_library(ImGui STATIC
	./dep/imgui/1.90.5/imgui.cpp
	./dep/imgui/1.90.5/imgui_demo.cpp
	./dep/imgui/1.90.5/imgui_draw.cpp
	./dep/imgui/1.90.5/imgui_tables.cpp
	./dep/imgui/1.90.5/imgui_widgets.cpp
	./dep/imgui/1.90.5/backends/imgui_impl_glfw.cpp
	./dep/imgui/1.90.5/backends/imgui_impl_vulkan.cpp
)
target_include_directories(ImGui PUBLIC
	${Vulkan_INCLUDE_DIRS}
	./dep/glfw/include/
	./dep/imgui/1.90.5/
)

file(GLOB KinectFusion_SOURCES ./src/*.cpp)
file(GLOB KinectFusion_HEADERS ./src/*.cpp)
add_executable(KinectFusion-Vulkan ${KinectFusion_HEADERS} ${KinectFusion_SOURCES})

target_include_directories(KinectFusion-Vulkan PUBLIC
	${Vulkan_INCLUDE_DIRS}
	./dep/glfw/include/
	./dep/Eigen/3.4.0/
	./dep/imgui/1.90.5/
	./dep/VulkanMemoryAllocator/3.0.1/include/
	./dep/jjyouLib/include/
)
target_link_libraries(KinectFusion-Vulkan
	${Vulkan_LIBRARIES}
	glfw
	ImGui
)