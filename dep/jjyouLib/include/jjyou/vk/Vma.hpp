/***********************************************************************
 * @file	Vma.hpp
 * @author	jjyou
 * @date	2024-2-6
 * @brief	This file implements VmaAllocation and VmaAllocator class.
 *			They are simply RAII wrappers for GPUOpen VMA library.
 * @sa		https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
***********************************************************************/
#ifndef jjyou_vk_Vma_hpp
#define jjyou_vk_Vma_hpp

#ifdef JJYOU_VK_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#endif

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>
#include <stdexcept>
#include <exception>
#include <string>

namespace jjyou {

	namespace vk {

		class VmaAllocation;
		class VmaAllocator;

		/***********************************************************************
		 * @class VmaAllocator
		 * @brief VmaAllocator class that follows RAII design pattern.
		 *
		 * This class is a RAII wrapper of VMA's VmaAllocator.
		 ***********************************************************************/
		class VmaAllocator {

		public:

			/** @brief	Construct an empty allocator.
			  */
			VmaAllocator(std::nullptr_t) {}

			/** @brief	Construct an allocator from VMA's CreateInfo structure.
			  */
			VmaAllocator(const VmaAllocatorCreateInfo& vmaAllocatorCreateInfo_) {
				VkResult result = vmaCreateAllocator(&vmaAllocatorCreateInfo_, &this->_allocator);
				if (result != VK_SUCCESS) {
					throw std::runtime_error(std::string("[VmaAllocator] Failed to create VmaAllocator. Error code: ") + std::to_string(result) + ".");
				}
			}

			/** @brief	Construct from non-RAII objects.
			  *
			  * This constructor constructs a RAII allocator from a non-RAII allocator.
			  * The resulting allocator will take the ownership of the non-RAII allocator.
			  */
			VmaAllocator(::VmaAllocator allocator_) : _allocator(allocator_) {}

			/** @brief	Copy constructor is disabled.
			  */
			VmaAllocator(const VmaAllocator&) = delete;

			/** @brief	Move constructor.
			  */
			VmaAllocator(VmaAllocator&& other_) noexcept : _allocator(other_._allocator) {
				other_._allocator = nullptr;
			}

			/** @brief	Copy assignment is disabled.
			  */
			VmaAllocator& operator=(const VmaAllocator&) = delete;

			/** @brief	Move assignment.
			  */
			VmaAllocator& operator=(VmaAllocator&& other_) noexcept {
				if (&other_ != this) {
					this->~VmaAllocator();
					this->_allocator = other_._allocator;
					other_._allocator = nullptr;
				}
				return *this;
			}

			/** @brief	Destructor.
			  */
			~VmaAllocator(void) {
				if (this->_allocator != nullptr) {
					vmaDestroyAllocator(this->_allocator);
					this->_allocator = nullptr;
				}
			}

			/** @brief	Dereference operator.
			  * @return	The underlying VMA's VmaAllocator instance.
			  */
			const ::VmaAllocator& operator*(void) const { return this->_allocator; }

			/** @brief	Release the ownership of the VMA's VmaAllocator instance.
			  */
			::VmaAllocator release(void) {
				::VmaAllocator res = this->_allocator;
				this->_allocator = nullptr;
				return res;
			}

		private:

			::VmaAllocator _allocator = nullptr;

		};

		/***********************************************************************
		 * @class VmaAllocation
		 * @brief VmaAllocation handle class that follows RAII design pattern.
		 *
		 * This class is a RAII wrapper of VMA's VmaAllocation.
		 ***********************************************************************/
		class VmaAllocation {

		public:

			/** @brief	Construct an empty allocation.
			  */
			VmaAllocation(std::nullptr_t) {}

			/** @brief	Construct from non-RAII objects.
			  *
			  * This constructor constructs a RAII allocation handle from a non-RAII allocation handle.
			  * The resulting allocation handle will take the ownership of the non-RAII allocation handle.
			  */
			VmaAllocation(const VmaAllocator& allocator_, ::VmaAllocation allocation_) : _allocator(*allocator_), _allocation(allocation_) {}

			/** @brief	Copy constructor is disabled.
			  */
			VmaAllocation(const VmaAllocation&) = delete;

			/** @brief	Move constructor.
			  */
			VmaAllocation(VmaAllocation&& other_) noexcept : _allocator(other_._allocator), _allocation(other_._allocation) {
				other_._allocator = nullptr;
				other_._allocation = nullptr;
			}

			/** @brief	Copy assignment is disabled.
			  */
			VmaAllocation& operator=(const VmaAllocation&) = delete;

			/** @brief	Move assignment.
			  */
			VmaAllocation& operator=(VmaAllocation&& other_) noexcept {
				if (&other_ != this) {
					this->~VmaAllocation();
					this->_allocator = other_._allocator;
					this->_allocation = other_._allocation;
					other_._allocator = nullptr;
					other_._allocation = nullptr;
				}
				return *this;
			}

			/** @brief	Destructor.
			  */
			~VmaAllocation(void) {
				if (this->_allocation != nullptr) {
					vmaFreeMemory(this->_allocator, this->_allocation);
					this->_allocator = nullptr;
					this->_allocation = nullptr;
				}
			}

			/** @brief	Dereference operator.
			  * @return	The underlying VMA's VmaAllocation instance.
			  */
			const ::VmaAllocation& operator*(void) const { return this->_allocation; }

			/** @brief	Release the ownership of the VMA's VmaAllocation instance.
			  */
			::VmaAllocation release(void) {
				::VmaAllocation res = this->_allocation;
				this->_allocator = nullptr;
				this->_allocation = nullptr;
				return res;
			}

		private:

			::VmaAllocator _allocator = nullptr;

			::VmaAllocation _allocation = nullptr;

			friend class VmaAllocator;

		};

	}

}

#endif /* jjyou_vk_MemoryAllocator_hpp */