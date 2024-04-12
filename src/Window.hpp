#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#include <jjyou/vk/Vulkan.hpp>
#include <jjyou/vis/CameraView.hpp>
#include <chrono>
#include <optional>

/***********************************************************************
 * @class	Window
 * @brief	Window class that wraps GLFWwindow.
 *
 *	This class follows RAII design pattern. Similar to Vulkan RAII
 *	wrappers, it has a constructor that takes std::nullptr to construct
 *	an empty window. It registers event callbacks for interactive visualization.
 ***********************************************************************/
class Window {

public:

	/***********************************************************************
	 * @enum	CameraMode
	 * @brief	Enum used to determine the camera mode.
	 ***********************************************************************/
	enum class CameraMode {
		Scene,	/**< Interactive scene camera controlled by mouse and keyboard. */
		Fixed	/**< Fix camera provided by the user. The scene camera will not be modified by callbacks in this mode. */
	};

	/** @brief	Construct an empty window.
	  */
	Window(std::nullptr_t) {}

	/** @brief	Construct a window using window size and title.
	  */
	Window(int width_, int height_, const char* title_);

	/** @brief	Copy constructor is disabled.
	  */
	Window(const Window&) = delete;

	/** @brief	Move constructor.
	  */
	Window(Window&& other_) noexcept :
		_glfwWindow(other_._glfwWindow), _surface(std::move(other_._surface)), _sceneViewer(other_._sceneViewer),
		_framebufferResized(other_._framebufferResized), _mouseButtonLeftPressTime(other_._mouseButtonLeftPressTime),
		_mouseButtonLeftPressed(other_._mouseButtonLeftPressed),
		_mouseButtonRightPressed(other_._mouseButtonRightPressed),
		_mouseButtonMiddlePressed(other_._mouseButtonMiddlePressed),
		_cursorPosX(other_._cursorPosX), _cursorPosY(other_._cursorPosY)
	{
		if (this->_glfwWindow != nullptr)
			glfwSetWindowUserPointer(this->_glfwWindow, this);
		other_._glfwWindow = nullptr;
	}

	/** @brief	Destructor.
	  */
	~Window(void);

	/** @brief	Copy assignment is disabled.
	  */
	Window& operator=(const Window&) = delete;

	/** @brief	Move assignment.
	  */
	Window& operator=(Window&& other_) noexcept {
		if (this != &other_) {
			this->~Window();
			this->_glfwWindow = other_._glfwWindow;
			this->_surface = std::move(other_._surface);
			this->_sceneViewer = other_._sceneViewer;
			this->_framebufferResized = other_._framebufferResized;
			this->_mouseButtonLeftPressTime = other_._mouseButtonLeftPressTime;
			this->_mouseButtonLeftPressed = other_._mouseButtonLeftPressed;
			this->_mouseButtonRightPressed = other_._mouseButtonRightPressed;
			this->_mouseButtonMiddlePressed = other_._mouseButtonMiddlePressed;
			this->_cursorPosX = other_._cursorPosX;
			this->_cursorPosY = other_._cursorPosY;
			if (this->_glfwWindow != nullptr)
				glfwSetWindowUserPointer(this->_glfwWindow, this);
			other_._glfwWindow = nullptr;
		}
		return *this;
	}

	/** @brief	Get required Vulkan instance extensions.
	  */
	static std::vector<const char*> getRequiredInstanceExtensions(void);

	/** @brief	Create Vulkan surface.
	  */
	void createSurface(const vk::raii::Instance& instance_);

	/** @brief	Get and reset the framebufferResized state to false.
	  */
	bool getAndResetFramebufferResized(void) {
		bool ret = _framebufferResized;
		this->_framebufferResized = false;
		return ret;
	}

	/** @brief	Get the underlying GLFW window.
	  */
	GLFWwindow* glfwWindow(void) const { return this->_glfwWindow; }

	/** @brief	Get the underlying Vulkan surface.
	  */
	const vk::raii::SurfaceKHR& surface(void) const { return this->_surface; }

	/** @brief	Get the window framebuffer's size.
	  */
	std::pair<int, int> framebufferSize(void) const { int width{}, height{}; glfwGetFramebufferSize(this->_glfwWindow, &width, &height); return std::make_pair(width, height); }
	
	static void waitEvents(void) { glfwWaitEvents(); }
	static void pollEvents(void) { glfwPollEvents(); }
	int windowShouldClose(void) const { return glfwWindowShouldClose(this->_glfwWindow); }
	
	/** @brief	Set camera mode.
	  * @param	cameraMode_		The new camera mode.
	  * @param	viewMatrix_		The camera view matrix. IGNORED for scene camera. REQUIRED for fixed camera.
	  */
	void setCameraMode(CameraMode cameraMode_, std::optional<jjyou::glsl::mat4> viewMatrix_) {
		this->_cameraMode = cameraMode_;
		if (cameraMode_ == CameraMode::Fixed)
			this->_fixedCameraView = *viewMatrix_;
	}

	/** @brief	Get the camera view matrix.
	  */
	jjyou::glsl::mat4 getViewMatrix(void) const {
		switch (this->_cameraMode) {
		case CameraMode::Scene: {
			return this->_sceneViewer.getViewMatrix();
			break;
		}
		case CameraMode::Fixed: {
			return this->_fixedCameraView;
			break;
		}
		default: {
			return jjyou::glsl::mat4(1.0f);
			break;
		}
		}
	}

private:

	// GLFW window
	GLFWwindow* _glfwWindow = nullptr;

	// Vulkan surface
	vk::raii::SurfaceKHR _surface{ nullptr };

	// Camera
	jjyou::vis::SceneView _sceneViewer{};
	void _resetSceneViewer(void) {
		this->_sceneViewer.reset();
		this->_sceneViewer.setZoomRate(10.0f);
	}
	CameraMode _cameraMode = CameraMode::Scene;
	jjyou::glsl::mat4 _fixedCameraView{ 1.0f };

	// Window counter
	static std::size_t _numGLFWWindows;

	bool _framebufferResized = false;
	static void _framebufferResizeCallbackDelegate(GLFWwindow* glfwWindow_, int width_, int height_);
	void _framebufferResizeCallback(int width_, int height_);

	std::chrono::steady_clock::time_point _mouseButtonLeftPressTime{}; 
	bool _mouseButtonLeftPressed = false;
	bool _mouseButtonRightPressed = false;
	bool _mouseButtonMiddlePressed = false;
	static void _mouseButtonCallbackDelegate(GLFWwindow* glfwWindow_, int button_, int action_, int mods_);
	void _mouseButtonCallback(int button_, int action_, int mods_);

	double _cursorPosX{}, _cursorPosY{};
	static void _cursorPosCallbackDelegate(GLFWwindow* glfwWindow_, double xPos_, double yPos_);
	void _cursorPosCallback(double xPos_, double yPos_);

	static void _scrollCallbackDelegate(GLFWwindow* glfwWindow_, double xOffset_, double yOffset_);
	void _scrollCallback(double xOffset_, double yOffset_);

	static void _keyCallbackDelegate(GLFWwindow* glfwWindow_, int key_, int scancode_, int action_, int mods_);
	void _keyCallback(int key_, int scancode_, int action_, int mods_);

};