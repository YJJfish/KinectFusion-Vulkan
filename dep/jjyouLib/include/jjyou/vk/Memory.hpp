/***********************************************************************
 * @file	Memory.hpp
 * @author	jjyou
 * @date	2024-2-6
 * @brief	This file implements Memory and MemoryAllocator class.
***********************************************************************/
#ifndef jjyou_vk_Memory_hpp
#define jjyou_vk_Memory_hpp

#include <vulkan/vulkan.h>

#include <limits>
#include <algorithm>

namespace jjyou {

	namespace vk {

		class Memory;
		class MemoryAllocator;

		class Memory {

		public:

			/** @brief	Default constructor.
			  */
			Memory(void) {}

			/** @brief	Copy constructor is disabled.
			  */
			Memory(const Memory&) = delete;

			/** @brief	Move constructor.
			  */
			Memory(Memory&& other) noexcept : _pAllocator(other._pAllocator), _memory(other._memory), _size(other._size), _offset(other._offset), _mappedAddress(other._mappedAddress) {
				other._pAllocator = nullptr;
				other._memory = nullptr;
				other._offset = 0ULL;
				other._size = 0ULL;
				other._mappedAddress = nullptr;
			}

			/** @brief	Copy assignment is disabled.
			  */
			Memory& operator=(const Memory&) = delete;

			/** @brief	Move assignment.
			  */
			Memory& operator=(Memory&& other) noexcept {
				if (&other != this) {
					this->~Memory();
					this->_pAllocator = other._pAllocator;
					this->_memory = other._memory;
					this->_offset = other._offset;
					this->_size = other._size;
					this->_mappedAddress = other._mappedAddress;
					other._pAllocator = nullptr;
					other._memory = nullptr;
					other._offset = 0ULL;
					other._size = 0ULL;
					other._mappedAddress = nullptr;
				}
				return *this;
			}

			/** @brief	Destructor.
			  */
			~Memory(void);

			/** @brief	Check whether the wrapper class contains a VkDeviceMemory instance.
			  * @return `true` if not empty.
			  */
			bool has_value() const {
				return (this->_pAllocator != nullptr);
			}

			/** @brief	Get the wrapped VkDeviceMemory instance.
			  * @return The wrapped VkDeviceMemory instance.
			  */
			VkDeviceMemory memory() const { return this->_memory; }

			/** @brief	Get memory size.
			  * @return Memory size.
			  */
			VkDeviceSize size(void) const { return this->_size; }
			
			/** @brief	Get memory offset.
			  * @return Memory offset.
			  */
			VkDeviceSize offset(void) const { return this->_offset; }

			/** @brief	Get memory mapped host address.
			  * @return Mapped host address.
			  */
			void* mappedAddress(void) const { return this->_mappedAddress; }

		private:

			MemoryAllocator* _pAllocator = nullptr;

			VkDeviceMemory _memory = nullptr;

			VkDeviceSize _size = 0ULL;

			VkDeviceSize _offset = 0ULL;

			void* _mappedAddress = nullptr;

			friend class MemoryAllocator;

		};

		class MemoryAllocator {

		public:

			/** @brief	Default constructor.
			  */
			MemoryAllocator(void) {}

			/** @brief	Initialize allocator.
			  * @param	device	Vulkan logical device.
			  */
			void init(VkDevice device) {
				this->device = device;
			}

			/** @brief	Destory allocator.
			  */
			void destory(void) {
				this->device = nullptr;
			}
			
			/** @brief	Allocate memory.
			  */
			VkResult allocate(const VkMemoryAllocateInfo* pAllocateInfo, Memory& memory) {
				VkResult res = vkAllocateMemory(this->device, pAllocateInfo, nullptr, &memory._memory);
				if (res == VK_SUCCESS) {
					memory._pAllocator = this;
					memory._offset = 0ULL;
					memory._size = pAllocateInfo->allocationSize;
					memory._mappedAddress = nullptr;
				}
				return res;
			}

			/** @brief	Free memory.
			  */
			void free(Memory& memory) {
				vkFreeMemory(this->device, memory._memory, nullptr);
				memory._pAllocator = nullptr;
				memory._memory = nullptr;
				memory._offset = 0ULL;
				memory._size = 0ULL;
				memory._mappedAddress = nullptr;
			}

			VkResult map(Memory& memory) {
				if (memory._mappedAddress != nullptr)
					return VK_SUCCESS;
				VkResult res = vkMapMemory(this->device, memory._memory, memory._offset, memory._size, 0, &memory._mappedAddress);
				return res;
			}

			VkResult unmap(Memory& memory) {
				vkUnmapMemory(this->device, memory._memory);
				memory._mappedAddress = nullptr;
				return VK_SUCCESS;
			}

		private:

			VkDevice device = nullptr;

		};

	}

}

/*======================================================================
 | Implementation
 ======================================================================*/
 /// @cond

namespace jjyou {

	namespace vk {

		inline Memory::~Memory(void) {
			if (this->_pAllocator != nullptr) {
				this->_pAllocator->free(*this);
			}
		}

	}

}

/// @endcond

#endif /* jjyou_vk_Memory_hpp */