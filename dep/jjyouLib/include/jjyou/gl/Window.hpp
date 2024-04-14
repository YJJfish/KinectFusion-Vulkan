/***********************************************************************
 * @file	Window.hpp
 * @author	jjyou
 * @date	2023-6-4
 * @brief	This file implements Window class and its derived classes,
 *			which are C++ wrappers for `GLFWwindow*`.
 * @details	The jjyou::gl::Window class is a C++ wrapper class for glfw
 *			window. The main idea is to provide object-oriented API for
 *			users. In glfw library, users need to call glfw functions to
 *			set/get window attributes, window callbacks, etc. With the
 *			help of jjyou::gl::Window class, you can directly call its
 *			member functions or modify its member variables to set up
 *			a window. \n
 *			To use this class, you should provide a unique template
 *			parameter `int ID` for each window you want to create.
 *			You cannot instantiate more than one window with the same ID. \n
 *			This file also implements some predefined classes inherited
 *			from the jjyou::gl::Window classes. They have predefined
 *			attributes and callbacks, so that you can use them to
 *			visualize data or develop other graphics applications
 *			without tedious setup process. \n
 *			You can also implement your custom classes inherited from
 *			jjyou::gl::Window.
***********************************************************************/
#ifndef jjyou_gl_Window_hpp
#define jjyou_gl_Window_hpp

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>
#include <string>
#include <chrono>
#include <numbers>
#include "CameraView.hpp"
#include "ModelView.hpp"

namespace jjyou {
	namespace gl {

		/***********************************************************************
		 * @class Window
		 * @brief Wrapper class for `GLFWwindow*`; Base class for other predefined
		 *		  window classes.
		 *
		 * This class is a C++ wrapper class for glfw window. Users can directly 
		 * call its member functions or modify its member variables to set up a window. \n
		 * It is also the base class for other predefined window classes. \n
		 * To use this class, you should provide a unique template parameter `int ID`
		 * for each window you want to create. You cannot instantiate more than one
		 * window with the same ID. \n
		 * 
		 * @tparam ID	The unique ID for the window.
		 ***********************************************************************/
		template <int ID>
		class Window {
		public:
			
			/** @name	Window attributes
			  */
			//@{
			static inline int id = ID;
			static GLFWwindow* window;
			static std::string windowTitle;
			//@}

			/** @name	Window handling callbacks
			  */
			//@{
			static GLFWwindowposfun windowPosFunc;
			static GLFWwindowsizefun windowSizeFunc;
			static GLFWwindowclosefun windowCloseFunc;
			static GLFWwindowrefreshfun windowRefreshFunc;
			static GLFWwindowfocusfun windowFocusFunc;
			static GLFWwindowiconifyfun windowIconifyFunc;
			static GLFWwindowmaximizefun windowMaximizeFunc;
			static GLFWframebuffersizefun frameBufferSizeFunc;
			static GLFWwindowcontentscalefun windowContentScaleFunc;
			//@}

			/** @name	Input handling callbacks
			  */
			//@{
			static GLFWmousebuttonfun mouseButtonFunc;
			static GLFWcursorposfun cursorPosFunc;
			static GLFWcursorenterfun cursorEnterFunc;
			static GLFWscrollfun scrollFunc;
			static GLFWkeyfun keyFunc;
			static GLFWcharfun charFunc;
			static GLFWcharmodsfun charModsFunc;
			static GLFWdropfun dropFunc;
			//@}

			/** @brief Instantiate a window for the given ID.
			  * 
			  * @note You cannot instantiate more than one
			  * window with the same ID. \n
			  */
			Window(void) {
				if (Window<ID>::instanceCounter) {
					std::cout << "[jjyou::gl::Window<" << ID << ">] More than one instance defined" << std::endl;
					exit(1);
				}
				Window<ID>::instanceCounter++;
			}

			/** @brief Destructor.
			  */
			~Window(void) {
				if (Window<ID>::window)
					glfwDestroyWindow(Window<ID>::window);
				Window<ID>::window = nullptr;
			}

			/** @brief Initialize glfw library and create a glfw window.
			  * 
			  * @param windowWidth		width of the window.
			  * @param windowHeight		height of the window.
			  * @param windowTitle		title of the window.
			  * @return `true` if the window is created.
			  */
			bool createGLFWWindow(
				int windowWidth = 500,
				int windowHeight = 500,
				const std::string& windowTitle = Window<ID>::windowTitle
			) {
				if (glfwInit() != GLFW_TRUE) {
					std::cout << "[jjyou::gl::Window<" << ID << ">] Failed to init GLFW library" << std::endl;
					return false;
				}
				this->destroyGLFWWindow();
				Window<ID>::windowTitle = windowTitle;
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
				glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
				Window<ID>::window = glfwCreateWindow(windowWidth, windowHeight, Window<ID>::windowTitle.c_str(), nullptr, nullptr);
				if (Window<ID>::window == nullptr) {
					std::cout << "[jjyou::gl::Window<" << ID << ">] Failed to create GLFW window" << std::endl;
					return false;
				}
				glfwMakeContextCurrent(Window<ID>::window);
				if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
					std::cout << "[jjyou::gl::Window<" << ID << ">] Failed to initialize GLAD" << std::endl;
					return false;
				}
				return true;
			}

			/** @brief Close the current glfw window.
			  * 
			  * @return `true` if the window existed and is now destoryed.
			  */
			bool destroyGLFWWindow(void) {
				if (Window<ID>::window) {
					glfwDestroyWindow(Window<ID>::window);
					Window<ID>::window = nullptr;
					return true;
				}
				else
					return false;
			}

			/** @brief Register all callbacks.
			  * 
			  * To register callbacks for the glfw window, you can
			  * assign callback functions to the corresponding
			  * member variables of jjyou::gl::Window, then call
			  * this function to register them.
			  */
			void registerCallbacks(void) {
				if (Window<ID>::window == nullptr) {
					return;
				}
				if (Window<ID>::windowPosFunc != nullptr) {
					glfwSetWindowPosCallback(Window<ID>::window, Window<ID>::windowPosFunc);
				}
				if (Window<ID>::windowSizeFunc != nullptr) {
					glfwSetWindowSizeCallback(Window<ID>::window, Window<ID>::windowSizeFunc);
				}
				if (Window<ID>::windowCloseFunc != nullptr) {
					glfwSetWindowCloseCallback(Window<ID>::window, Window<ID>::windowCloseFunc);
				}
				if (Window<ID>::windowRefreshFunc != nullptr) {
					glfwSetWindowRefreshCallback(Window<ID>::window, Window<ID>::windowRefreshFunc);
				}
				if (Window<ID>::windowFocusFunc != nullptr) {
					glfwSetWindowFocusCallback(Window<ID>::window, Window<ID>::windowFocusFunc);
				}
				if (Window<ID>::windowIconifyFunc != nullptr) {
					glfwSetWindowIconifyCallback(Window<ID>::window, Window<ID>::windowIconifyFunc);
				}
				if (Window<ID>::windowMaximizeFunc != nullptr) {
					glfwSetWindowMaximizeCallback(Window<ID>::window, Window<ID>::windowMaximizeFunc);
				}
				if (Window<ID>::frameBufferSizeFunc != nullptr) {
					glfwSetFramebufferSizeCallback(Window<ID>::window, Window<ID>::frameBufferSizeFunc);
				}
				if (Window<ID>::windowContentScaleFunc != nullptr) {
					glfwSetWindowContentScaleCallback(Window<ID>::window, Window<ID>::windowContentScaleFunc);
				}
				if (Window<ID>::mouseButtonFunc != nullptr) {
					glfwSetMouseButtonCallback(Window<ID>::window, Window<ID>::mouseButtonFunc);
				}
				if (Window<ID>::cursorPosFunc != nullptr) {
					glfwSetCursorPosCallback(Window<ID>::window, Window<ID>::cursorPosFunc);
				}
				if (Window<ID>::cursorEnterFunc != nullptr) {
					glfwSetCursorEnterCallback(Window<ID>::window, Window<ID>::cursorEnterFunc);
				}
				if (Window<ID>::scrollFunc != nullptr) {
					glfwSetScrollCallback(Window<ID>::window, Window<ID>::scrollFunc);
				}
				if (Window<ID>::keyFunc != nullptr) {
					glfwSetKeyCallback(Window<ID>::window, Window<ID>::keyFunc);
				}
				if (Window<ID>::charFunc != nullptr) {
					glfwSetCharCallback(Window<ID>::window, Window<ID>::charFunc);
				}
				if (Window<ID>::charModsFunc != nullptr) {
					glfwSetCharModsCallback(Window<ID>::window, Window<ID>::charModsFunc);
				}
				if (Window<ID>::dropFunc != nullptr) {
					glfwSetDropCallback(Window<ID>::window, Window<ID>::dropFunc);
				}
			}

			/** @name	Deleted functions
			  */
			//@{
			Window(const Window&) = delete;
			Window(Window&&) = delete;
			Window& operator=(const Window&) = delete;
			Window& operator=(Window&&) = delete;
			//@}

		private:
			static int instanceCounter;
		};
		template<int ID> int Window<ID>::instanceCounter = 0;
		template<int ID> GLFWwindow* Window<ID>::window = nullptr;
		template<int ID> std::string Window<ID>::windowTitle = "default window " + std::to_string(ID);
		template<int ID> GLFWwindowposfun Window<ID>::windowPosFunc = nullptr;
		template<int ID> GLFWwindowsizefun Window<ID>::windowSizeFunc = nullptr;
		template<int ID> GLFWwindowclosefun Window<ID>::windowCloseFunc = nullptr;
		template<int ID> GLFWwindowrefreshfun Window<ID>::windowRefreshFunc = nullptr;
		template<int ID> GLFWwindowfocusfun Window<ID>::windowFocusFunc = nullptr;
		template<int ID> GLFWwindowiconifyfun Window<ID>::windowIconifyFunc = nullptr;
		template<int ID> GLFWwindowmaximizefun Window<ID>::windowMaximizeFunc = nullptr;
		template<int ID> GLFWframebuffersizefun Window<ID>::frameBufferSizeFunc = nullptr;
		template<int ID> GLFWwindowcontentscalefun Window<ID>::windowContentScaleFunc = nullptr;
		template<int ID> GLFWmousebuttonfun Window<ID>::mouseButtonFunc = nullptr;
		template<int ID> GLFWcursorposfun Window<ID>::cursorPosFunc = nullptr;
		template<int ID> GLFWcursorenterfun Window<ID>::cursorEnterFunc = nullptr;
		template<int ID> GLFWscrollfun Window<ID>::scrollFunc = nullptr;
		template<int ID> GLFWkeyfun Window<ID>::keyFunc = nullptr;
		template<int ID> GLFWcharfun Window<ID>::charFunc = nullptr;
		template<int ID> GLFWcharmodsfun Window<ID>::charModsFunc = nullptr;
		template<int ID> GLFWdropfun Window<ID>::dropFunc = nullptr;



		/***********************************************************************
		 * @class Object3DViewer
		 * @brief Inherited from jjyou::gl::Window for viewing an object in 3d space.
		 *
		 * This window class is used for viewing an object in 3d space.
		 * It has predefined window size callback, framebuffer size callback,
		 * mouse button callback, cursor position callback, and scroll callback.
		 * It maintains a jjyou::gl::CameraView instance and a jjyou::gl::ModelView
		 * instance to compute the model view and camera view matrices for shaders.
		 * Pressing the left mouse button and moving the cursor can translate and
		 * rotate the model view.
		 * Scrolling the middle mouse button can scale the model view.
		 * Double-click the left mouse button can reset the model view.
		 * The camera is fixed to `(0,0,2)`, looking in the direction of `(0,0,-1)`,
		 * with its "up" vector equal to `(0,1,0)`.
		 * To create an instance of this window, you need to provide a unique
		 * template parameter `int ID`. You cannot instantiate more than one
		 * window with the same ID. Also, you need to provide the y-fov, the near
		 * and far clip planes to compute the perspective projection matrix.
		 * 
		 * @tparam ID	The unique ID for the window.
		 ***********************************************************************/
		template <int ID>
		class Object3DViewer : public Window<ID> {
		public:
			
			/** @name	Viewer attributes
			  * 
			  * @note	Do not modify `frameWidth`, `frameHeight`,
			  *			`windowWidth`, and `windowHeight`.
			  *			If you want to resize the frame buffer or
			  *			the window, call `glfwSetWindowSize`. These
			  *			variables will be updated by callback funcions.
			  */
			//@{
			static GLfloat yFov;
			static GLfloat aspectRatio;
			static GLfloat zNear;
			static GLfloat zFar;
			static int frameWidth;
			static int frameHeight;
			static int windowWidth;
			static int windowHeight;
			//@}

			/** @name	Camera view and model view
			  */
			//@{
			static ModelView<1> modelView;
			static FirstPersonView cameraView;
			//@}

			/** @name	Mouse state
			  * @note	Do not modify these variables. They will be
			  *			updated by callback funcions.
			  */
			//@{
			static double cursorX;
			static double cursorY;
			static std::chrono::steady_clock::time_point mouseButtonLeftPressTime;
			//@}

			/** @name	Predefined callback functions
			  */
			//@{
			static void defaultWindowSizeCallback(GLFWwindow* window, int width, int height) {
				Object3DViewer<ID>::windowWidth = width;
				Object3DViewer<ID>::windowHeight = height;
			}
			static void defaultFrameBufferSizeCallback(GLFWwindow* window, int width, int height) {
				Object3DViewer<ID>::frameWidth = width;
				Object3DViewer<ID>::frameHeight = height;
				Object3DViewer<ID>::aspectRatio = (GLfloat)width / height;
				glViewport(0, 0, width, height);
			}
			static void defaultMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
				if (ImGui::GetIO().WantCaptureMouse)
					return;
				auto now = std::chrono::steady_clock::now();
				if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
					if ((now - Object3DViewer<ID>::mouseButtonLeftPressTime).count() <= 200000000)
						Object3DViewer<ID>::modelView.reset();
					Object3DViewer<ID>::mouseButtonLeftPressTime = now;
				}
			}
			static void defaultCursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
				if (ImGui::GetIO().WantCaptureMouse) {
					cursorX = xPos;
					cursorY = yPos;
					return;
				}
				if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
					//y movement
					{
						double dist2Center = xPos - Object3DViewer<ID>::windowWidth / 2.0;
						GLfloat zRotateStrength = std::clamp(dist2Center * 4.0 / Object3DViewer<ID>::windowWidth, -1.0, 1.0);
						GLfloat xRotateAttenuation = std::min(
							1.0, std::pow(0.1, std::abs(dist2Center) / Object3DViewer<ID>::windowWidth * 8.0 - 1.0)
						);
						modelView.rotate(0.003 * (yPos - cursorY) * xRotateAttenuation, glm::vec3(1.0f, 0.0f, 0.0f));
						modelView.rotate(0.002 * (cursorY - yPos) * zRotateStrength, glm::vec3(0.0f, 0.0f, 1.0f));
					}
					//x movement
					{
						double dist2Center = yPos - Object3DViewer<ID>::windowHeight / 2.0;
						GLfloat zRotateStrength = std::clamp(dist2Center * 4.0 / Object3DViewer<ID>::windowHeight, -1.0, 1.0);
						GLfloat yRotateAttenuation = std::min(
							1.0, std::pow(0.1, std::abs(dist2Center) / Object3DViewer<ID>::windowHeight * 8.0 - 1.0)
						);
						modelView.rotate(0.003 * (xPos - cursorX) * yRotateAttenuation, glm::vec3(0.0f, 1.0f, 0.0f));
						modelView.rotate(0.002 * (xPos - cursorX) * zRotateStrength, glm::vec3(0.0f, 0.0f, 1.0f));
					}
				}
				else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
				}
				else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
					modelView.translate(glm::vec3(0.0, 0.002 * (cursorY - yPos), 0.0f));
					modelView.translate(glm::vec3(0.002 * (xPos - cursorX), 0.0f, 0.0f));
				}
				cursorX = xPos;
				cursorY = yPos;
			}
			static void defaultScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
				if (ImGui::GetIO().WantCaptureMouse)
					return;
				if (yOffset < 0.0f)
					modelView.scale(glm::vec3(1.2f));
				else
					modelView.scale(glm::vec3(1.0f / 1.2f));
			}
			//@}

		public:

			/** @brief Instantiate a window for the given ID.
			  *
			  * @note You cannot instantiate more than one
			  * window with the same ID. \n
			  */
			Object3DViewer(void) : Window<ID>() {}

			/** @brief Destructor.
			  */
			~Object3DViewer(void) {}

			/** @brief Initialize glfw library and create a glfw window.
			  * 
			  * If the user do not set the window size / frame buffer size / 
			  * mouse button / cursor position / scroll callback functions,
			  * the predefined callback functions will be registered.
			  * 
			  * @param windowWidth		width of the window.
			  * @param windowHeight		height of the window.
			  * @param windowTitle		title of the window.
			  *	@param yFov				vertical field of view.
			  * @param zNear			z coordinate of the near clip plane.
			  * @param zFar				z coordinate of the far clip plane.
			  * @return `true` if the window is created.
			  */
			bool createGLFWWindow(
				int windowWidth = 500,
				int windowHeight = 500,
				const std::string& windowTitle = Window<ID>::windowTitle,
				GLfloat yFov = std::numbers::pi_v<GLfloat> / 3.0f,
				GLfloat zNear = 0.01f,
				GLfloat zFar = 10.0f
			) {
				//create window
				if (!Window<ID>::createGLFWWindow(windowWidth, windowHeight, windowTitle))
					return false;
				//set viewer attributes
				glfwGetFramebufferSize(Window<ID>::window, &Object3DViewer<ID>::frameWidth, &Object3DViewer<ID>::frameHeight);
				glfwGetWindowSize(Window<ID>::window, &Object3DViewer<ID>::windowWidth, &Object3DViewer<ID>::windowHeight);
				Object3DViewer<ID>::yFov = yFov;
				Object3DViewer<ID>::aspectRatio = (GLfloat)Object3DViewer<ID>::frameWidth / Object3DViewer<ID>::frameHeight;
				Object3DViewer<ID>::zNear = zNear;
				Object3DViewer<ID>::zFar = zFar;
				//reset camera&model matrix
				Object3DViewer<ID>::modelView.reset();
				Object3DViewer<ID>::cameraView.reset();
				Object3DViewer<ID>::cameraView.setPos(0.0f, 0.0f, 2.0f);
				//initialize window state
				glfwGetCursorPos(Window<ID>::window, &Object3DViewer<ID>::cursorX, &Object3DViewer<ID>::cursorY);
				//config window
				glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				if (Window<ID>::windowSizeFunc == nullptr)
					Window<ID>::windowSizeFunc = Object3DViewer<ID>::defaultWindowSizeCallback;
				if (Window<ID>::frameBufferSizeFunc == nullptr)
					Window<ID>::frameBufferSizeFunc = Object3DViewer<ID>::defaultFrameBufferSizeCallback;
				if (Window<ID>::mouseButtonFunc == nullptr)
					Window<ID>::mouseButtonFunc = Object3DViewer<ID>::defaultMouseButtonCallback;
				if (Window<ID>::cursorPosFunc == nullptr)
					Window<ID>::cursorPosFunc = Object3DViewer<ID>::defaultCursorPosCallback;
				if (Window<ID>::scrollFunc == nullptr)
					Window<ID>::scrollFunc = Object3DViewer<ID>::defaultScrollCallback;
				Window<ID>::registerCallbacks();
				//init imgui
				IMGUI_CHECKVERSION();
				ImGui::CreateContext();
				ImGui_ImplGlfw_InitForOpenGL(Window<ID>::window, true);
				ImGui_ImplOpenGL3_Init("#version 330");
				ImGui::StyleColorsDark();
				return true;
			}

			/* @name Compute uniform variables for passing to shaders
			 */
			//@{
			/* @brief Compute model matrix.
			 * @return the model matrix for `modelMatrix` uniform variable in shaders
			 */
			glm::mat4 getModelMatrix(void) {
				return Object3DViewer<ID>::modelView.getModelMatrix();
			}

			/* @brief Compute view matrix.
			 * @return the view matrix for `viewMatrix` uniform variable in shaders
			 */
			glm::mat4 getViewMatrix(void) {
				return Object3DViewer<ID>::cameraView.getViewMatrix();
			}

			/* @brief Compute normal matrix.
			 * @return the normal matrix for `normalMatrix` uniform variable in shaders
			 */
			glm::mat4 getNormalMatrix(void) {
				return glm::mat3(glm::transpose(glm::inverse(Object3DViewer<ID>::getModelMatrix())));
			}

			/* @brief Compute projection matrix.
			 * @return the projection matrix for `projectionMatrix` uniform variable in shaders
			 */
			glm::mat4 getProjectionMatrix(void) {
				return glm::perspective(
					Object3DViewer<ID>::yFov,
					Object3DViewer<ID>::aspectRatio,
					Object3DViewer<ID>::zNear, Object3DViewer<ID>::zFar
				);
			}

			/* @brief Compute view position (i.e. camera position).
			 * @return the view position vector for `viewPosition` uniform variable in shaders
			 */
			glm::vec3 getViewPosition(void) {
				return Object3DViewer<ID>::cameraView.getPos();
			}
			//@}
		};
		template<int ID> GLfloat Object3DViewer<ID>::yFov = std::numbers::pi_v<GLfloat> / 3.0f;
		template<int ID> GLfloat Object3DViewer<ID>::aspectRatio = 1.0f;
		template<int ID> GLfloat Object3DViewer<ID>::zNear = 0.01f;
		template<int ID> GLfloat Object3DViewer<ID>::zFar = 10.0f;
		template<int ID> int Object3DViewer<ID>::frameWidth = 0;
		template<int ID> int Object3DViewer<ID>::frameHeight = 0;
		template<int ID> int Object3DViewer<ID>::windowWidth = 0;
		template<int ID> int Object3DViewer<ID>::windowHeight = 0;
		template<int ID> ModelView<1> Object3DViewer<ID>::modelView;
		template<int ID> FirstPersonView Object3DViewer<ID>::cameraView;
		template<int ID> double Object3DViewer<ID>::cursorX = 0.0;
		template<int ID> double Object3DViewer<ID>::cursorY = 0.0;
		template<int ID> std::chrono::steady_clock::time_point Object3DViewer<ID>::mouseButtonLeftPressTime;


		/***********************************************************************
		 * @class Scene3DViewer
		 * @brief Inherited from jjyou::gl::Window for viewing the scene in 3d space.
		 *
		 * This window class is used for viewing the scene in 3d space.
		 * It has predefined window size callback, framebuffer size callback,
		 * mouse button callback, cursor position callback, and scroll callback.
		 * It maintains a jjyou::gl::SceneView instance to compute the camera
		 * view matrices for shaders.
		 * Pressing the right mouse button and moving the cursor can translate
		 * the camera view.
		 * Scrolling the middle mouse button and moving the cursor can rotate
		 * the camera view.
		 * Scrolling the middle mouse button can scale the model view.
		 * Double-click the left mouse button can reset the model view.
		 * The camera in default pose.
		 * To create an instance of this window, you need to provide a unique
		 * template parameter `int ID`. You cannot instantiate more than one
		 * window with the same ID. Also, you need to provide the y-fov, the near
		 * and far clip planes to compute the perspective projection matrix.
		 *
		 * @tparam ID	The unique ID for the window.
		 ***********************************************************************/
		template <int ID>
		class Scene3DViewer : public Window<ID> {
		public:

			/** @name	Viewer attributes
			  *
			  * @note	Do not modify `frameWidth`, `frameHeight`,
			  *			`windowWidth`, and `windowHeight`.
			  *			If you want to resize the frame buffer or
			  *			the window, call `glfwSetWindowSize`. These
			  *			variables will be updated by callback funcions.
			  */
			  //@{
			static GLfloat yFov;
			static GLfloat aspectRatio;
			static GLfloat zNear;
			static GLfloat zFar;
			static int frameWidth;
			static int frameHeight;
			static int windowWidth;
			static int windowHeight;
			//@}

			/** @name	Camera view
			  */
			  //@{
			static SceneView cameraView;
			//@}

			/** @name	Mouse state
			  * @note	Do not modify these variables. They will be
			  *			updated by callback funcions.
			  */
			  //@{
			static double cursorX;
			static double cursorY;
			static std::chrono::steady_clock::time_point mouseButtonLeftPressTime;
			//@}

			/** @name	Predefined callback functions
			  */
			  //@{
			static void defaultWindowSizeCallback(GLFWwindow* window, int width, int height) {
				Scene3DViewer<ID>::windowWidth = width;
				Scene3DViewer<ID>::windowHeight = height;
			}
			static void defaultFrameBufferSizeCallback(GLFWwindow* window, int width, int height) {
				Scene3DViewer<ID>::frameWidth = width;
				Scene3DViewer<ID>::frameHeight = height;
				Scene3DViewer<ID>::aspectRatio = (GLfloat)width / height;
				glViewport(0, 0, width, height);
			}
			static void defaultMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
				if (ImGui::GetIO().WantCaptureMouse)
					return;
				auto now = std::chrono::steady_clock::now();
				if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
					if ((now - Scene3DViewer<ID>::mouseButtonLeftPressTime).count() <= 200000000)
						Scene3DViewer<ID>::cameraView.reset();
					Scene3DViewer<ID>::mouseButtonLeftPressTime = now;
				}
			}
			static void defaultCursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
				if (ImGui::GetIO().WantCaptureMouse) {
					cursorX = xPos;
					cursorY = yPos;
					return;
				}
				if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
				}
				else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
					cameraView.moveUp(0.001f * (yPos - cursorY));
					cameraView.moveLeft(0.001f * (xPos - cursorX));
				}
				else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
					cameraView.turn(0.002f * (xPos - cursorX), 0.002f * (cursorY - yPos), 0.0f);
				}
				cursorX = xPos;
				cursorY = yPos;
			}
			static void defaultScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
				if (ImGui::GetIO().WantCaptureMouse)
					return;
				if (yOffset < 0.0f)
					cameraView.zoomOut(1.2f);
				else
					cameraView.zoomIn(1.2f);
			}
			//@}

		public:

			/** @brief Instantiate a window for the given ID.
			  *
			  * @note You cannot instantiate more than one
			  * window with the same ID. \n
			  */
			Scene3DViewer(void) : Window<ID>() {}

			/** @brief Destructor.
			  */
			~Scene3DViewer(void) {}

			/** @brief Initialize glfw library and create a glfw window.
			  *
			  * If the user do not set the window size / frame buffer size /
			  * mouse button / cursor position / scroll callback functions,
			  * the predefined callback functions will be registered.
			  *
			  * @param windowWidth		width of the window.
			  * @param windowHeight		height of the window.
			  * @param windowTitle		title of the window.
			  *	@param yFov				vertical field of view.
			  * @param zNear			z coordinate of the near clip plane.
			  * @param zFar				z coordinate of the far clip plane.
			  * @return `true` if the window is created.
			  */
			bool createGLFWWindow(
				int windowWidth = 500,
				int windowHeight = 500,
				const std::string& windowTitle = Window<ID>::windowTitle,
				GLfloat yFov = std::numbers::pi_v<GLfloat> / 3.0f,
				GLfloat zNear = 0.01f,
				GLfloat zFar = 500.0f
			) {
				//create window
				if (!Window<ID>::createGLFWWindow(windowWidth, windowHeight, windowTitle))
					return false;
				//set viewer attributes
				glfwGetFramebufferSize(Window<ID>::window, &Scene3DViewer<ID>::frameWidth, &Scene3DViewer<ID>::frameHeight);
				glfwGetWindowSize(Window<ID>::window, &Scene3DViewer<ID>::windowWidth, &Scene3DViewer<ID>::windowHeight);
				Scene3DViewer<ID>::yFov = yFov;
				Scene3DViewer<ID>::aspectRatio = (GLfloat)Scene3DViewer<ID>::frameWidth / Scene3DViewer<ID>::frameHeight;
				Scene3DViewer<ID>::zNear = zNear;
				Scene3DViewer<ID>::zFar = zFar;
				//reset camera&model matrix
				Scene3DViewer<ID>::cameraView.reset();
				//initialize window state
				glfwGetCursorPos(Window<ID>::window, &Scene3DViewer<ID>::cursorX, &Scene3DViewer<ID>::cursorY);
				//config window
				glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				if (Window<ID>::windowSizeFunc == nullptr)
					Window<ID>::windowSizeFunc = Scene3DViewer<ID>::defaultWindowSizeCallback;
				if (Window<ID>::frameBufferSizeFunc == nullptr)
					Window<ID>::frameBufferSizeFunc = Scene3DViewer<ID>::defaultFrameBufferSizeCallback;
				if (Window<ID>::mouseButtonFunc == nullptr)
					Window<ID>::mouseButtonFunc = Scene3DViewer<ID>::defaultMouseButtonCallback;
				if (Window<ID>::cursorPosFunc == nullptr)
					Window<ID>::cursorPosFunc = Scene3DViewer<ID>::defaultCursorPosCallback;
				if (Window<ID>::scrollFunc == nullptr)
					Window<ID>::scrollFunc = Scene3DViewer<ID>::defaultScrollCallback;
				Window<ID>::registerCallbacks();
				//init imgui
				IMGUI_CHECKVERSION();
				ImGui::CreateContext();
				ImGui_ImplGlfw_InitForOpenGL(Window<ID>::window, true);
				ImGui_ImplOpenGL3_Init("#version 330");
				ImGui::StyleColorsDark();
				return true;
			}

			/* @name Compute uniform variables for passing to shaders
			 */
			 //@{
			 /* @brief Compute model matrix.
			  * @return the model matrix for `modelMatrix` uniform variable in shaders
			  */
			glm::mat4 getModelMatrix(void) {
				return glm::identity<glm::mat4>();
			}

			/* @brief Compute view matrix.
			 * @return the view matrix for `viewMatrix` uniform variable in shaders
			 */
			glm::mat4 getViewMatrix(void) {
				return Scene3DViewer<ID>::cameraView.getViewMatrix();
			}

			/* @brief Compute normal matrix.
			 * @return the normal matrix for `normalMatrix` uniform variable in shaders
			 */
			glm::mat4 getNormalMatrix(void) {
				return glm::mat3(glm::transpose(glm::inverse(Scene3DViewer<ID>::getViewMatrix())));
			}

			/* @brief Compute projection matrix.
			 * @return the projection matrix for `projectionMatrix` uniform variable in shaders
			 */
			glm::mat4 getProjectionMatrix(void) {
				return glm::perspective(
					Scene3DViewer<ID>::yFov,
					Scene3DViewer<ID>::aspectRatio,
					Scene3DViewer<ID>::zNear, Scene3DViewer<ID>::zFar
				);
			}

			/* @brief Compute view position (i.e. camera position).
			 * @return the view position vector for `viewPosition` uniform variable in shaders
			 */
			glm::vec3 getViewPosition(void) {
				return Scene3DViewer<ID>::cameraView.getPos();
			}
			//@}
		};
		template<int ID> GLfloat Scene3DViewer<ID>::yFov = std::numbers::pi_v<GLfloat> / 3.0f;
		template<int ID> GLfloat Scene3DViewer<ID>::aspectRatio = 1.0f;
		template<int ID> GLfloat Scene3DViewer<ID>::zNear = 0.01f;
		template<int ID> GLfloat Scene3DViewer<ID>::zFar = 500.0f;
		template<int ID> int Scene3DViewer<ID>::frameWidth = 0;
		template<int ID> int Scene3DViewer<ID>::frameHeight = 0;
		template<int ID> int Scene3DViewer<ID>::windowWidth = 0;
		template<int ID> int Scene3DViewer<ID>::windowHeight = 0;
		template<int ID> SceneView Scene3DViewer<ID>::cameraView;
		template<int ID> double Scene3DViewer<ID>::cursorX = 0.0;
		template<int ID> double Scene3DViewer<ID>::cursorY = 0.0;
		template<int ID> std::chrono::steady_clock::time_point Scene3DViewer<ID>::mouseButtonLeftPressTime;
	}
}
#endif /* jjyou_gl_Window_hpp */