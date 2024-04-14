/***********************************************************************
 * @file	Loader.hpp
 * @author	jjyou
 * @date	2024-1-29
 * @brief	This file implements Loader class.
***********************************************************************/
#ifndef jjyou_vk_Loader_hpp
#define jjyou_vk_Loader_hpp

#include <cstring>

namespace jjyou {

	namespace vk {


		/***********************************************************************
		 * @class Loader
		 * @brief Vulkan loader class.
		 *
		 * This class loads vulkan extension functions.
		 ***********************************************************************/
		class Loader {

		public:

			/** @brief Default constructor.
			  */
			Loader(void) {}

			/** @brief Load extension functions of a given layer.
			  * @param instance		The vulkan instance.
			  * @param layerName	The name of the layer to be loaded.
			  */
			void load(VkInstance instance, const char* layerName);

		public:

			#define JJYOU_VK_LOADER_DECLARE_FUNC_PTR(funcName) PFN_##funcName funcName = nullptr

			/** @name	VK_EXT_debug_utils
			  * @brief	Functions of "VK_EXT_debug_utils" extension.
			  */
			//@{
			JJYOU_VK_LOADER_DECLARE_FUNC_PTR(vkCmdBeginDebugUtilsLabelEXT);
			JJYOU_VK_LOADER_DECLARE_FUNC_PTR(vkCmdEndDebugUtilsLabelEXT);
			JJYOU_VK_LOADER_DECLARE_FUNC_PTR(vkCmdInsertDebugUtilsLabelEXT);
			JJYOU_VK_LOADER_DECLARE_FUNC_PTR(vkCreateDebugUtilsMessengerEXT);
			JJYOU_VK_LOADER_DECLARE_FUNC_PTR(vkDestroyDebugUtilsMessengerEXT);
			JJYOU_VK_LOADER_DECLARE_FUNC_PTR(vkQueueBeginDebugUtilsLabelEXT);
			JJYOU_VK_LOADER_DECLARE_FUNC_PTR(vkQueueEndDebugUtilsLabelEXT);
			JJYOU_VK_LOADER_DECLARE_FUNC_PTR(vkQueueInsertDebugUtilsLabelEXT);
			JJYOU_VK_LOADER_DECLARE_FUNC_PTR(vkSetDebugUtilsObjectNameEXT);
			JJYOU_VK_LOADER_DECLARE_FUNC_PTR(vkSetDebugUtilsObjectTagEXT);
			JJYOU_VK_LOADER_DECLARE_FUNC_PTR(vkSubmitDebugUtilsMessageEXT);
			//@}

			#undef JJYOU_VK_LOADER_DECLARE_FUNC_PTR

		};

	}

}

/*======================================================================
 | Implementation
 ======================================================================*/
 /// @cond

namespace jjyou {

	namespace vk {

		#define JJYOU_VK_LOADER_LOAD(funcName) this->funcName = (PFN_##funcName)vkGetInstanceProcAddr(instance, #funcName)

		inline void Loader::load(VkInstance instance, const char* layerName) {
			if (std::strcmp(layerName, "VK_EXT_debug_utils") == 0) {
				JJYOU_VK_LOADER_LOAD(vkCmdBeginDebugUtilsLabelEXT);
				JJYOU_VK_LOADER_LOAD(vkCmdEndDebugUtilsLabelEXT);
				JJYOU_VK_LOADER_LOAD(vkCmdInsertDebugUtilsLabelEXT);
				JJYOU_VK_LOADER_LOAD(vkCreateDebugUtilsMessengerEXT);
				JJYOU_VK_LOADER_LOAD(vkDestroyDebugUtilsMessengerEXT);
				JJYOU_VK_LOADER_LOAD(vkQueueBeginDebugUtilsLabelEXT);
				JJYOU_VK_LOADER_LOAD(vkQueueEndDebugUtilsLabelEXT);
				JJYOU_VK_LOADER_LOAD(vkQueueInsertDebugUtilsLabelEXT);
				JJYOU_VK_LOADER_LOAD(vkSetDebugUtilsObjectNameEXT);
				JJYOU_VK_LOADER_LOAD(vkSetDebugUtilsObjectTagEXT);
				JJYOU_VK_LOADER_LOAD(vkSubmitDebugUtilsMessageEXT);
			}
		}

		#undef JJYOU_VK_LOADER_LOAD

	}

}
/// @endcond


#endif /* jjyou_vk_Loader_hpp */