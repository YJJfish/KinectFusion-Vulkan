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
file(GLOB ImGui_HEADERS
	./dep/imgui/imconfig.h
	./dep/imgui/imgui.h
	./dep/imgui/imgui_internal.h
	./dep/imgui/imstb_rectpack.h
	./dep/imgui/imstb_textedit.h
	./dep/imgui/imstb_truetype.h
	./dep/imgui/backends/imgui_impl_glfw.h
	./dep/imgui/backends/imgui_impl_vulkan.h
)
file(GLOB ImGui_SOURCES
	./dep/imgui/imgui.cpp
	./dep/imgui/imgui_demo.cpp
	./dep/imgui/imgui_draw.cpp
	./dep/imgui/imgui_tables.cpp
	./dep/imgui/imgui_widgets.cpp
	./dep/imgui/backends/imgui_impl_glfw.cpp
	./dep/imgui/backends/imgui_impl_vulkan.cpp
)
add_library(ImGui STATIC ${ImGui_HEADERS} ${ImGui_SOURCES})
target_include_directories(ImGui PUBLIC
	${Vulkan_INCLUDE_DIRS}
	./dep/glfw/include/
	./dep/imgui/
)

# KinectFusion-Vulkan
file(GLOB KinectFusion_HEADERS ./src/*.hpp)
file(GLOB KinectFusion_SOURCES ./src/*.cpp)
add_executable(KinectFusion-Vulkan ${KinectFusion_HEADERS} ${KinectFusion_SOURCES})

# Link
target_include_directories(KinectFusion-Vulkan PUBLIC
	${Vulkan_INCLUDE_DIRS}
	./dep/glfw/include/
	./dep/eigen/
	./dep/imgui/
	./dep/VulkanMemoryAllocator/include/
	./dep/jjyouLib/include/
)
target_link_libraries(KinectFusion-Vulkan
	${Vulkan_LIBRARIES}
	glfw
	ImGui
)