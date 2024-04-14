/***********************************************************************
 * @file	Device.hpp
 * @author	jjyou
 * @date	2024-1-31
 * @brief	This file implements Device and DeviceBuilder class.
***********************************************************************/
#ifndef jjyou_vk_Device_hpp
#define jjyou_vk_Device_hpp

#include <vulkan/vulkan.h>
#include <vector>
#include <map>

#include "Instance.hpp"
#include "PhysicalDevice.hpp"
#include "utils.hpp"

namespace jjyou {

	namespace vk {

		//Forward declaration
		class Device;
		class DeviceBuilder;

		/***********************************************************************
		 * @class Device
		 * @brief C++ wrapper for VkDevice.
		 *
		 * This class is a C++ wrapper class of VkDevice. It wraps
		 * some frequently used vulkan functions into class methods.
		 ***********************************************************************/
		class Device {

		public:

			/** @brief	Default constructor.
			  */
			Device(void) {}

			/** @brief	Destructor.
			  */
			~Device(void) {}

			/** @brief	Check whether the wrapper class contains a VkDevice instance.
			  * @return `true` if not empty.
			  */
			bool has_value() const {
				return (this->device != nullptr);
			}

			/** @brief	Call the corresponding vkDestroyXXX function to destroy the wrapped instance.
			  */
			void destroy(void) {
				if (this->device != nullptr) {
					vkDestroyDevice(this->device, nullptr);
					this->device = nullptr;
				}
			}

			/** @brief	Get the wrapped VkDevice instance.
			  * @return The wrapped VkDevice instance.
			  */
			VkDevice get() const { return this->device; }

			/** @brief	Get graphics queues.
			  * @return Vector of graphics queues.
			  */
			const std::optional<VkQueue>& graphicsQueues(void) const { return this->_graphicsQueues; }

			/** @brief	Get compute queues.
			  * @return Vector of compute queues.
			  */
			const std::optional<VkQueue>& computeQueues(void) const { return this->_computeQueues; }

			/** @brief	Get transfer queues.
			  * @return Vector of transfer queues.
			  */
			const std::optional<VkQueue>& transferQueues(void) const { return this->_transferQueues; }

			/** @brief	Get present queues.
			  * @return Vector of present queues.
			  */
			const std::optional<VkQueue>& presentQueues(void) const { return this->_presentQueues; }

			/** @brief	Get all queues.
			  * @return Map of queue family index to vector of queues.
			  */
			const std::map<std::uint32_t, std::vector<VkQueue>>& queues(void) const { return this->_queues; }

		private:

			VkDevice device = nullptr;
			std::optional<VkQueue> _graphicsQueues = std::nullopt;
			std::optional<VkQueue> _computeQueues = std::nullopt;
			std::optional<VkQueue> _transferQueues = std::nullopt;
			std::optional<VkQueue> _presentQueues = std::nullopt;
			std::map<std::uint32_t, std::vector<VkQueue>> _queues = {};

			friend class DeviceBuilder;

		};

		/***********************************************************************
		 * @class DeviceBuilder
		 * @brief Helper class to build a VkDevice in Vulkan.
		 *
		 * This class helps to simplify the process of building a logical
		 * device in vulkan.
		 ***********************************************************************/
		class DeviceBuilder {

		public:

			/** @brief	Construct from jjyou::vk::Instance and jjyou::vk::PhysicalDevice.
			  * @note	In Vulkan, VkInstance is not required as an argument to create VkDevice.
			  *			However, here we need jjyou::vk::Instance to get the enabled layers.
			  *			Device specific layers have now been deprecated, but it is still
			  *			a good idea to set them anyway to be compatible with older implementations.
			  */
			DeviceBuilder(const Instance& instance, const PhysicalDevice& physicalDevice) : instance(instance), physicalDevice(physicalDevice) {}

			/** @brief	Specify the queues to be created along with the logical device.
			  * @note	By default, the builder will create exactly one graphics/compute/present/transfer
			  *			queue with priority=1.0f if jjyou::vk::PhysicalDevice has stored the index for
			  *			graphics/compute/present queue families (i.e. You have required these queues
			  *			from jjyou::vk::PhysicalDeviceSelector). You can use this function to overwrite
			  *			the default settings, or specify queues to be created from new families.
			  */
			DeviceBuilder& createQueues(std::uint32_t familyIndex, const std::vector<float>& priorities) {
				this->deviceQueuePriorities[familyIndex] = priorities;
				return *this;
			}

			/** @brief	Build a VkDevice instance.
			  * @return	A VkDevice wrapped in jjyou::vk::Device.
			  */
			Device build(void) const {
				std::map<std::uint32_t, std::vector<float>> deviceQueuePriorities = this->deviceQueuePriorities;
				if (this->physicalDevice.graphicsQueueFamily().has_value())
					deviceQueuePriorities.emplace(*this->physicalDevice.graphicsQueueFamily(), std::vector<float>{1.0f});
				if (this->physicalDevice.computeQueueFamily().has_value())
					deviceQueuePriorities.emplace(*this->physicalDevice.computeQueueFamily(), std::vector<float>{1.0f});
				if (this->physicalDevice.presentQueueFamily().has_value())
					deviceQueuePriorities.emplace(*this->physicalDevice.presentQueueFamily(), std::vector<float>{1.0f});
				if (this->physicalDevice.transferQueueFamily().has_value())
					deviceQueuePriorities.emplace(*this->physicalDevice.transferQueueFamily(), std::vector<float>{1.0f});
				for (auto iter = deviceQueuePriorities.cbegin(); iter != deviceQueuePriorities.cend(); ) {
					if (iter->second.empty())
						deviceQueuePriorities.erase(iter++);
					else
						++iter;
				}
				std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
				for (const auto& queueFamily : deviceQueuePriorities) {
					VkDeviceQueueCreateInfo queueCreateInfo{
						.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
						.pNext = nullptr,
						.flags = 0U,
						.queueFamilyIndex = queueFamily.first,
						.queueCount = static_cast<std::uint32_t>(queueFamily.second.size()),
						.pQueuePriorities = queueFamily.second.data()
					};
					queueCreateInfos.push_back(queueCreateInfo);
				}
				VkDeviceCreateInfo deviceCreateInfo{
					.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0U,
					.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size()),
					.pQueueCreateInfos = queueCreateInfos.data(),
					.enabledLayerCount = static_cast<uint32_t>(this->instance.enabledLayers().size()),
					.ppEnabledLayerNames = this->instance.enabledLayers().data(),
					.enabledExtensionCount = static_cast<std::uint32_t>(this->physicalDevice.enabledDeviceExtensions().size()),
					.ppEnabledExtensionNames = this->physicalDevice.enabledDeviceExtensions().data(),
					.pEnabledFeatures = &this->physicalDevice.enabledDeviceFeatures()
				};
				Device device;
				JJYOU_VK_UTILS_CHECK(vkCreateDevice(physicalDevice.get(), &deviceCreateInfo, nullptr, &device.device));
				for (const auto& queueFamily : deviceQueuePriorities) {
					std::vector<VkQueue>& queues = device._queues[queueFamily.first];
					queues.resize(queueFamily.second.size());
					for (int i = 0; i < queueFamily.second.size(); ++i)
						vkGetDeviceQueue(device.device, queueFamily.first, i, &queues[i]);
					if (queueFamily.first == this->physicalDevice.graphicsQueueFamily())
						device._graphicsQueues = queues.front();
					if (queueFamily.first == this->physicalDevice.computeQueueFamily())
						device._computeQueues = queues.front();
					if (queueFamily.first == this->physicalDevice.presentQueueFamily())
						device._presentQueues = queues.front();
					if (queueFamily.first == this->physicalDevice.transferQueueFamily())
						device._transferQueues = queues.front();
				}
				return device;
			}
		
		private:

			const jjyou::vk::Instance& instance;
			const jjyou::vk::PhysicalDevice& physicalDevice;
			std::map<std::uint32_t, std::vector<float>> deviceQueuePriorities;

		};

	}

}

#endif /* jjyou_vk_Device_hpp */