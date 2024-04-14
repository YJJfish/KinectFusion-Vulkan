/***********************************************************************
 * @file	utils.hpp
 * @author	jjyou
 * @date	2024-1-29
 * @brief	This file implements utility functions for vulkan development.
 *			This file is still under development. Some of the functions
 *			may be moved to other files in future versions.
***********************************************************************/
#ifndef jjyou_vk_utils_hpp
#define jjyou_vk_utils_hpp

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <exception>
#include <format>
#include <optional>

namespace jjyou {

	namespace vk {

		namespace utils {

			/** @name	Macros
			  * @brief	Useful macros for vulkan development.
			  */
			//@{
			#define JJYOU_VK_UTILS_THROW(err) \
				throw std::runtime_error(std::format("Vulkan error in file {} line {}: {}", __FILE__, __LINE__, string_VkResult(err)))
			
			#define JJYOU_VK_UTILS_CHECK(value) \
				if (VkResult err = (value); err != VK_SUCCESS) { JJYOU_VK_UTILS_THROW(err); }
			
			//@}



			/** @name	Functions
			  * @brief	Useful functions for vulkan development.
			  */
			//@{

		

			//@}
		
		}

	}

}

#endif /* jjyou_vk_utils_hpp */