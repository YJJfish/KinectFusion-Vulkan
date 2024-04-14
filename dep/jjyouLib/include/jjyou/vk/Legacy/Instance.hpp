/***********************************************************************
 * @file	Instance.hpp
 * @author	jjyou
 * @date	2024-1-30
 * @brief	This file implements Instance and InstanceBuilder class.
***********************************************************************/

#ifndef jjyou_vk_Instance_hpp
#define jjyou_vk_Instance_hpp

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <iostream>
#include <format>

#include "utils.hpp"

namespace jjyou {

	namespace vk {

		//Forward declaration
		class Instance;
		class InstanceBuilder;

		/***********************************************************************
		 * @class Instance
		 * @brief C++ wrapper for VkInstance.
		 *
		 * This class is a C++ wrapper class of VkInstance. It wraps
		 * some frequently used vulkan functions into class methods.
		 ***********************************************************************/
		class Instance {

		public:

			/** @brief	Default constructor.
			  */
			Instance(void) {}

			/** @brief	Destructor.
			  */
			~Instance(void) {}

			/** @brief	Check whether the wrapper class contains a VkInstance instance.
			  * @return `true` if not empty.
			  */
			bool has_value() const {
				return (this->instance != nullptr);
			}

			/** @brief	Call the corresponding vkDestroyXXX function to destroy the wrapped instance.
			  */
			void destroy(void) {
				if (this->instance != nullptr) {
					if (this->_debugMessenger != nullptr) {
						PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(this->instance, "vkDestroyDebugUtilsMessengerEXT");
						if (vkDestroyDebugUtilsMessengerEXT == nullptr)
							JJYOU_VK_UTILS_THROW(VK_ERROR_EXTENSION_NOT_PRESENT);
						vkDestroyDebugUtilsMessengerEXT(this->instance, this->_debugMessenger, nullptr);
					}
					vkDestroyInstance(this->instance, nullptr);
					this->instance = nullptr;
				}
			}

			/** @brief	Get the wrapped VkInstance instance.
			  * @return The wrapped VkInstance instance.
			  */
			VkInstance get() const { return this->instance; }

			/** @brief	Get the rendering mode.
			  * @return `true` if offscreen rendering mode.
			  */
			bool offscreen() const { return this->_offscreen; }

			/** @brief	Get enabled layers.
			  * @return Vector of enabled layers.
			  */
			const std::vector<const char*>& enabledLayers(void) const { return this->_enabledLayers; }

			/** @brief	Get enabled instance extensions.
			  * @return Vector of enabled instance extensions.
			  */
			const std::vector<const char*>& enabledInstanceExtensions(void) const { return this->_enabledInstanceExtensions; }
			
			/** @brief	Get the debug messenger.
			  * @return A VkDebugUtilsMessengerEXT instance. If not created, it will be nullptr.
			  */
			VkDebugUtilsMessengerEXT debugMessenger(void) const { return this->_debugMessenger; }

			/** @brief	Wrapper function for vkEnumeratePhysicalDevices.
			  * @return	Vector of physical devices.
			  */
			std::vector<VkPhysicalDevice> enumeratePhysicalDevices(void) const {
				return Instance::enumeratePhysicalDevices(this->instance);
			}

			/** @brief	Wrapper function for vkEnumeratePhysicalDevices.
			  * @param	Instance	VkInstance instance.
			  * @return	Vector of physical devices.
			  */
			static std::vector<VkPhysicalDevice> enumeratePhysicalDevices(VkInstance instance) {
				uint32_t physicalDeviceCount = 0;
				vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
				std::vector<VkPhysicalDevice> availablePhysicalDevices(physicalDeviceCount);
				vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, availablePhysicalDevices.data());
				return availablePhysicalDevices;
			}

			/** @brief	Wrapper function for vkEnumerateInstanceLayerProperties.
			  * @return	Vector of instance layer properties.
			  */
			static std::vector<VkLayerProperties> enumerateInstanceLayerProperties(void) {
				uint32_t layerCount;
				vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
				std::vector<VkLayerProperties> availableLayers(layerCount);
				vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
				return availableLayers;
			}

			/** @brief	Helper function to check layer support.
			  * @param	layers	Layers to check.
			  * @return	`true` if all layers are supported.
			  */
			static bool checkLayerSupport(const std::vector<const char*>& layers) {
				std::vector<VkLayerProperties> availableLayers = enumerateInstanceLayerProperties();
				for (const auto& layerName : layers) {
					bool layerFound = false;
					for (const auto& layerProperties : availableLayers) {
						if (strcmp(layerName, layerProperties.layerName) == 0) {
							layerFound = true;
							break;
						}
					}
					if (!layerFound) {
						return false;
					}
				}
				return true;
			}

			/** @brief	Wrapper function for vkEnumerateInstanceExtensionProperties.
			  * @param	pLayerName	The layer to retrieve extensions from.
			  * @return	Vector of instance extension properties.
			  */
			static std::vector<VkExtensionProperties> enumerateInstanceExtensionProperties(const char* pLayerName = nullptr) {
				uint32_t extensionCount;
				vkEnumerateInstanceExtensionProperties(pLayerName, &extensionCount, nullptr);
				std::vector<VkExtensionProperties> availableExtensions(extensionCount);
				vkEnumerateInstanceExtensionProperties(pLayerName, &extensionCount, availableExtensions.data());
				return availableExtensions;
			}

			/** @brief	Helper function to check instance extenson support.
			  * @param	extensions	The extensions to check.
			  * @param	pLayerName	The layer to retrieve extensions from.
			  * @return	`true` if all extensions are supported.
			  */
			static bool checkInstanceExtensionSupport(const std::vector<const char*>& extensions, const char* pLayerName = nullptr) {
				std::vector<VkExtensionProperties> availableExtensions = enumerateInstanceExtensionProperties(pLayerName);
				for (const auto& extensionName : extensions) {
					bool extensionFound = false;
					for (const auto& extensionProperties : availableExtensions) {
						if (strcmp(extensionName, extensionProperties.extensionName) == 0) {
							extensionFound = true;
							break;
						}
					}
					if (!extensionFound) {
						return false;
					}
				}
				return true;
			}

		private:

			VkInstance instance = nullptr;
			bool _offscreen = false;
			std::vector<const char*> _enabledLayers = {};
			std::vector<const char*> _enabledInstanceExtensions = {};
			VkDebugUtilsMessengerEXT _debugMessenger = nullptr;

			friend class InstanceBuilder;

		};

		/***********************************************************************
		 * @class InstanceBuilder
		 * @brief Helper class to create a VkInstance instance in Vulkan.
		 *
		 * This class is a helper class to simplify the process of creating a
		 * VkInstance instance in Vulkan.
		 ***********************************************************************/
		class InstanceBuilder {

		public:

			/** @brief	Default constructor.
			  */
			InstanceBuilder(void) {}

			/** @brief	Enable validation layer.
			  * @param	enable	Whether to enable validation layer.
			  */
			InstanceBuilder& enableValidation(bool enable = true) {
				this->_enableValidationLayer = enable;
				return *this;
			}

			/** @brief	Set offscreen rendering mode.
			  * @param	_offscreen	Whether to set offscreen rendering mode.
			  */
			InstanceBuilder& offscreen(bool _offscreen = true) {
				this->_offscreen = _offscreen;
				return *this;
			}

			/** @brief	Set application name.
			  * @param	_applicationName	Application name
			  */
			InstanceBuilder& applicationName(const std::string& _applicationName) {
				this->_applicationName = _applicationName;
				return *this;
			}

			/** @brief	Set application version.
			  * @param	variant		Variant version number.
			  * @param	major		Major version number.
			  * @param	minor		Minor version number.
			  * @param	patch		Patch version number.
			  */
			InstanceBuilder& applicationVersion(std::uint32_t variant, std::uint32_t major, std::uint32_t minor, std::uint32_t patch) {
				this->_applicationVersion = VK_MAKE_API_VERSION(variant, major, minor, patch);
				return *this;
			}

			/** @brief	Set application version.
			  * @param	_applicationVersion		Application version.
			  */
			InstanceBuilder& applicationVersion(std::uint32_t _applicationVersion) {
				this->_applicationVersion = _applicationVersion;
				return *this;
			}

			/** @brief	Set engine name.
			  * @param	_engineName		Engine name.
			  */
			InstanceBuilder& engineName(const std::string& _engineName) {
				this->_engineName = _engineName;
				return *this;
			}

			/** @brief	Set engine version.
			  * @param	variant		Variant version number.
			  * @param	major		Major version number.
			  * @param	minor		Minor version number.
			  * @param	patch		Patch version number.
			  */
			InstanceBuilder& engineVersion(std::uint32_t variant, std::uint32_t major, std::uint32_t minor, std::uint32_t patch) {
				this->_engineVersion = VK_MAKE_API_VERSION(variant, major, minor, patch);
				return *this;
			}

			/** @brief	Set engine version.
			  * @param	_engineVersion		Engine version.
			  */
			InstanceBuilder& engineVersion(std::uint32_t _engineVersion) {
				this->_engineVersion = _engineVersion;
				return *this;
			}

			/** @brief	Set API version.
			  * @param	variant		Variant version number.
			  * @param	major		Major version number.
			  * @param	minor		Minor version number.
			  * @param	patch		Patch version number.
			  */
			InstanceBuilder& apiVersion(std::uint32_t variant, std::uint32_t major, std::uint32_t minor, std::uint32_t patch) {
				this->_apiVersion = VK_MAKE_API_VERSION(variant, major, minor, patch);
				return *this;
			}

			/** @brief	Set API version.
			  * @param	_apiVersion		API version.
			  */
			InstanceBuilder& apiVersion(std::uint32_t _apiVersion) {
				this->_apiVersion = _apiVersion;
				return *this;
			}

			/** @brief	Enable a layer.
			  * @note	If you already called InstanceBuilder::enableValidationLayer to enable
			  *			the validation layer, DO NOT call this function to enable it again.
			  * @param	layerName	The name of the layer.
			  */
			InstanceBuilder& enableLayer(const char* layerName) {
				this->enabledLayers.push_back(layerName);
				return *this;
			}

			/** @brief	Enable an instance extension.
			  * @note	If you already called InstanceBuilder::setDebugUtilsMessenger to set
			  *			the debug messenger, "VK_EXT_debug_utils" extension
			  *			will be automatically added. DO NOT call this function to add it again,
			  *			If you did not turn the rendering mode to offscreen mode,
			  *			the extensions required for creating window on your OS will be
			  *			automatically added. DO NOT call this function to add them again.
			  * @param	extensionName	The name of the extension.
			  */
			InstanceBuilder& enableInstanceExtension(const char* extensionName) {
				this->enabledInstanceExtensions.push_back(extensionName);
				return *this;
			}

			/** @brief	Set debug messenger.
			  * @param	messageSeverity		The VkDebugUtilsMessengerCreateInfoEXT::messageSeverity field.
			  * @param	messageType			The VkDebugUtilsMessengerCreateInfoEXT::messageType field.
			  * @param	pfnUserCallback		The VkDebugUtilsMessengerCreateInfoEXT::pfnUserCallback field.
			  * @param	pUserData			The VkDebugUtilsMessengerCreateInfoEXT::pUserData field.
			  */
			InstanceBuilder& setDebugUtilsMessenger(
				VkDebugUtilsMessageSeverityFlagsEXT messageSeverity,
				VkDebugUtilsMessageTypeFlagsEXT messageType,
				PFN_vkDebugUtilsMessengerCallbackEXT pfnUserCallback,
				void* pUserData = nullptr
			) {
				this->_enableValidationLayer = true;
				this->_enableDebugUtilsMessenger = true;
				this->debugUtilsMessengerInfo.messageSeverity = messageSeverity;
				this->debugUtilsMessengerInfo.messageType = messageType;
				this->debugUtilsMessengerInfo.pfnUserCallback = pfnUserCallback;
				this->debugUtilsMessengerInfo.pUserData = pUserData;
				return *this;
			}

			/** @brief	Use default debug messenger.
			  */
			InstanceBuilder& useDefaultDebugUtilsMessenger(void) {
				this->setDebugUtilsMessenger(
					VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
					VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
					InstanceBuilder::defaultDebugCallback,
					nullptr
				);
				return *this;
			}

			/** @brief	Build a VkInstance instance.
			  * @return	A VkInstance wrapped in jjyou::vk::Instance.
			  */
			Instance build(void) const {
				std::vector<const char*> enabledLayers = this->enabledLayers;
				if (this->_enableValidationLayer) {
					enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
				}
				std::vector<const char*> enabledInstanceExtensions = this->enabledInstanceExtensions;
				if (!this->_offscreen) {
					enabledInstanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(_WIN32)
					enabledInstanceExtensions.push_back("VK_KHR_win32_surface");
#elif defined(__ANDROID__)
					enabledInstanceExtensions.push_back("VK_KHR_android_surface");
#elif defined(__linux__)
					enabledInstanceExtensions.push_back("VK_KHR_xcb_surface");
#elif defined(__APPLE__) && defined(__MACH__)
					enabledInstanceExtensions.push_back("VK_MVK_macos_surface");
#endif
				}
				if (this->_enableDebugUtilsMessenger) {
					enabledInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				}
				if (!Instance::checkLayerSupport(enabledLayers))
					JJYOU_VK_UTILS_THROW(VK_ERROR_LAYER_NOT_PRESENT);
				if (!Instance::checkInstanceExtensionSupport(enabledInstanceExtensions))
					JJYOU_VK_UTILS_THROW(VK_ERROR_EXTENSION_NOT_PRESENT);
				VkApplicationInfo appInfo{
					.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
					.pNext = nullptr,
					.pApplicationName = this->_applicationName.c_str(),
					.applicationVersion = this->_applicationVersion,
					.pEngineName = this->_engineName.c_str(),
					.engineVersion = this->_engineVersion,
					.apiVersion = this->_apiVersion
				};
				VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{
					.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
					.pNext = nullptr,
					.messageSeverity = this->debugUtilsMessengerInfo.messageSeverity,
					.messageType = this->debugUtilsMessengerInfo.messageType,
					.pfnUserCallback = this->debugUtilsMessengerInfo.pfnUserCallback,
					.pUserData = this->debugUtilsMessengerInfo.pUserData
				};
				VkInstanceCreateInfo instanceCreateInfo{
					.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
					.pNext = (this->_enableDebugUtilsMessenger) ? &debugCreateInfo : nullptr,
					.pApplicationInfo = &appInfo,
					.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
					.ppEnabledLayerNames = enabledLayers.data(),
					.enabledExtensionCount = static_cast<uint32_t>(enabledInstanceExtensions.size()),
					.ppEnabledExtensionNames = enabledInstanceExtensions.data()
				};
				Instance instance;
				JJYOU_VK_UTILS_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &instance.instance));
				instance._offscreen = this->_offscreen;
				instance._enabledLayers = enabledLayers;
				instance._enabledInstanceExtensions = enabledInstanceExtensions;
				if (this->_enableDebugUtilsMessenger) {
					PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance.instance, "vkCreateDebugUtilsMessengerEXT");
					if (vkCreateDebugUtilsMessengerEXT == nullptr)
						JJYOU_VK_UTILS_THROW(VK_ERROR_EXTENSION_NOT_PRESENT);
					JJYOU_VK_UTILS_CHECK(vkCreateDebugUtilsMessengerEXT(instance.instance, &debugCreateInfo, nullptr, &instance._debugMessenger));
				}
				return instance;
			}

		private:

			bool _enableValidationLayer = false;
			bool _offscreen = false;
			std::string _applicationName = "";
			std::uint32_t _applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
			std::string _engineName = "";
			std::uint32_t _engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
			std::uint32_t _apiVersion = VK_API_VERSION_1_0;
			std::vector<const char*> enabledLayers = {};
			std::vector<const char*> enabledInstanceExtensions = {};
			bool _enableDebugUtilsMessenger = false;
			struct {
				VkDebugUtilsMessageSeverityFlagsEXT messageSeverity;
				VkDebugUtilsMessageTypeFlagsEXT messageType;
				PFN_vkDebugUtilsMessengerCallbackEXT pfnUserCallback;
				void* pUserData;
			} debugUtilsMessengerInfo = {};

			// Default debug messenger callback
			static VKAPI_ATTR VkBool32 VKAPI_CALL defaultDebugCallback(
				VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
				VkDebugUtilsMessageTypeFlagsEXT messageType,
				const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
				void* pUserData
			) {
				auto msgSeverity = [](VkDebugUtilsMessageSeverityFlagBitsEXT s) -> std::string {
					std::string ret;
					if (s & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
						ret += "VERBOSE | ";
					if (s & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
						ret += "INFO | ";
					if (s & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
						ret += "WARNING | ";
					if (s & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
						ret += "ERROR | ";
					if (ret.length())
						return ret.substr(0, ret.length() - 2);
					return "NONE";
				};
				auto msgType = [](VkDebugUtilsMessageTypeFlagsEXT s) -> std::string {
					std::string ret;
					if (s & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
						ret += "GENERAL | ";
					if (s & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
						ret += "VALIDATION | ";
					if (s & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
						ret += "PERFORMANCE | ";
					if (s & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT)
						ret += "DEVICE ADDRESS BINDING | ";
					if (ret.length())
						return ret.substr(0, ret.length() - 2);
					return "NONE";
				};
				std::cerr << std::format("[Debug Callback] severity: {}, type: {}, message: {}", msgSeverity(messageSeverity), msgType(messageType), pCallbackData->pMessage) << std::endl;
				return VK_FALSE;
			}
		};

	}

}

#endif /* jjyou_vk_Instance_hpp */