/***********************************************************************
 * @file	Swapchain.hpp
 * @author	jjyou
 * @date	2024-2-2
 * @brief	This file implements Swapchain and SwapchainBuilder class.
***********************************************************************/
#ifndef jjyou_vk_Swapchain_hpp
#define jjyou_vk_Swapchain_hpp

#include <vulkan/vulkan_raii.hpp>
#include "Context.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <optional>
#include <limits>
#include <algorithm>

namespace jjyou {

	namespace vk {

		class Swapchain;
		class SwapchainBuilder;

		/***********************************************************************
		 * @class Swapchain
		 * @brief C++ wrapper for Vulkan swapchain and related objects.
		 * @note  This class relies on the Context class. It is your responsibility to
		 *        make sure the Context instance is valid during the swapchain's lifetime.
		 * 
		 * This class is an abstraction layer over several Vulkan resources related to
		 * Vulkan swapchain, and have higher level methods compared with the original Vulkan API.
		 * This class owns the underlying Vulkan resources. Therefore it is
		 * movable but not copyable. Like Vulkan RAII header, it also has a constructor
		 * that takes a std::nullptr_t value to construct an empty context.
		 * Besides these, it has no other public constructors. You have to use
		 * SwapchainBuilder class to create it.
		 ***********************************************************************/
		class Swapchain {

		public:

			/** @brief	Construct an empty swapchain.
			  */
			Swapchain(std::nullptr_t) {}

			/** @brief	Copy constructor is disabled.
			  */
			Swapchain(const Swapchain&) = delete;

			/** @brief	Move constructor.
			  */
			Swapchain(Swapchain&& other) = default;

			/** @brief	Destructor.
			  */
			~Swapchain(void) = default;

			/** @brief	Copy assignment is disabled.
			  */
			Swapchain& operator=(const Swapchain&) = delete;

			/** @brief	Move assignment.
			  */
			Swapchain& operator=(Swapchain&& other) noexcept {
				if (this != &other) {
					this->_pContext = other._pContext;
					this->_surface = other._surface;
					this->_swapchain = std::move(other._swapchain);
					this->_numImages = other._numImages;
					this->_images = std::move(other._images);
					this->_imageViews = std::move(other._imageViews);
					this->_surfaceFormat = other._surfaceFormat;
					this->_presentMode = other._presentMode;
					this->_extent = other._extent;
				}
				return *this;
			}

			/** @brief	Get the number of images in the swapchain.
			  * @return The number of images in the swapchain.
			  */
			std::uint32_t numImages(void) const { return this->_numImages; }

			/** @brief	Get the underlying Vulkan swapchain.
			  * @return The Vulkan swapchain instance.
			  */
			const ::vk::raii::SwapchainKHR& swapchain(void) const { return this->_swapchain; }

			/** @brief	Get the swapchain image.
			  * @param	index_	The index of the swapchain image.
			  * @return The swapchain image.
			  */
			const ::vk::Image& image(std::uint32_t index_) const { return this->_images[static_cast<std::size_t>(index_)]; }

			/** @brief	Get the swapchain image view.
			  * @param	index_	The index of the swapchain image view.
			  * @return The swapchain image view.
			  */
			const ::vk::raii::ImageView& imageView(std::uint32_t index_) const { return this->_imageViews[static_cast<std::size_t>(index_)]; }

			/** @brief	Get the swapchain surface format.
			  * @return Swapchain surface format.
			  */
			::vk::SurfaceFormatKHR surfaceFormat(void) const { return this->_surfaceFormat; }

			/** @brief	Get the present mode.
			  * @return Swapchain present mode.
			  */
			::vk::PresentModeKHR presentMode(void) const { return this->_presentMode; }

			/** @brief	Get the swapchain extent.
			  * @return Swapchain extent.
			  */
			::vk::Extent2D extent(void) const { return this->_extent; }

		private:

			const Context* _pContext{ nullptr };
			::vk::SurfaceKHR _surface{ nullptr };
			::vk::raii::SwapchainKHR _swapchain{ nullptr };
			std::uint32_t _numImages{};
			std::vector<::vk::Image> _images = {};
			std::vector<::vk::raii::ImageView> _imageViews = {};
			::vk::SurfaceFormatKHR _surfaceFormat = {};
			::vk::PresentModeKHR _presentMode = {};
			::vk::Extent2D _extent = {};

			friend class SwapchainBuilder;

		};

		/***********************************************************************
		 * @class SwapchainBuilder
		 * @brief Builder class to build a Swapchain instance.
		 *
		 * This class is the builder class to build a Swapchain instance.
		 ***********************************************************************/
		class SwapchainBuilder {

		public:

			/** @brief	Construct from Context and Vulkan surface.
			  * @param	context_	The Vulkan context.
			  * @param	surface_	The Vulkan surface.
			  */
			SwapchainBuilder(const Context& context_, const ::vk::raii::SurfaceKHR& surface_) : _pContext(&context_), _surface(*surface_) {}

			/** @brief	Set requested surface format.
			  * @param	format_		Surface format.
			  * 
			  *	If you have multiple preferred formats, please pass them from the most preferred one
			  *	to the least preferred one. If none of them are supported, the builder will use the
			  *	first available format returned by `vkGetPhysicalDeviceSurfaceFormatsKHR`.
			  * By default, there is no requested format.
			  */
			SwapchainBuilder& requestSurfaceFormat(::vk::SurfaceFormatKHR format_) {
				this->_requestSurfaceFormats.push_back(format_);
				return *this;
			}

			/** @brief	Set required surface format.
			  * @param	format_		Surface format.
			  *
			  *	If you set a required format, the builder will throw an error
			  * if that format is not supported.
			  * By default, there is no required format.
			  */
			SwapchainBuilder& requireSurfaceFormat(::vk::SurfaceFormatKHR format_) {
				this->_requireSurfaceFormat = format_;
				return *this;
			}

			/** @brief	Set requested present mode.
			  * @param	presentMode_	Present mode.
			  * 
			  *	If you have multiple preferred present modes, please pass them from the most preferred one
			  *	to the least preferred one. If none of them are supported, the builder will use
			  *	`VK_PRESENT_MODE_FIFO_KHR`.
			  * By default, there is no requested present mode.
			  */
			SwapchainBuilder& requestPresentMode(::vk::PresentModeKHR presentMode_) {
				this->_requestPresentModes.push_back(presentMode_);
				return *this;
			}

			/** @brief	Set required present mode.
			  * @param	presentMode_	Present mode.
			  * 
			  *	If you set a required present mode, the builder will throw an error
			  * if that mode is not supported.
			  * By default, there is no required present mode.
			  */
			SwapchainBuilder& requirePresentMode(::vk::PresentModeKHR presentMode_) {
				this->_requirePresentMode = presentMode_;
				return *this;
			}

			/** @brief	Build the swapchain and related resources.
			  * @param	extent_			Swapchain extent.
			  * @param	oldSwapchain_	The old swapchain. This parameter is a r-value because the
			  *							builder will take the ownership of it and reuse it to create
			  *							the new swapchain.
			  */
			Swapchain build(::vk::Extent2D extent_, Swapchain&& oldSwapchain_ = { nullptr }) const {
				::vk::SurfaceCapabilitiesKHR capabilities = this->_pContext->physicalDevice().getSurfaceCapabilitiesKHR(this->_surface);
				std::vector<::vk::SurfaceFormatKHR> supportedSurfaceFormats = this->_pContext->physicalDevice().getSurfaceFormatsKHR(this->_surface);
				std::vector<::vk::PresentModeKHR> supportedPresentModes = this->_pContext->physicalDevice().getSurfacePresentModesKHR(this->_surface);
				if (supportedSurfaceFormats.empty())
					throw std::runtime_error("[Vulkan SwapchainBuilder] No supported surface format for the surface.");
				if (supportedPresentModes.empty())
					throw std::runtime_error("[Vulkan SwapchainBuilder] No supported present mode for the surface.");
				::vk::SurfaceFormatKHR surfaceFormat{};
				if (this->_requireSurfaceFormat.has_value()) {
					if (std::find(supportedSurfaceFormats.cbegin(), supportedSurfaceFormats.cend(), *this->_requireSurfaceFormat) != supportedSurfaceFormats.cend())
						surfaceFormat = *this->_requireSurfaceFormat;
					else
						throw std::runtime_error("[Vulkan SwapchainBuilder] No supported surface format for the surface.");
				}
				else {
					surfaceFormat = supportedSurfaceFormats[0];
					for (const auto& requestedSurfaceFormat : this->_requestSurfaceFormats) {
						if (std::find(supportedSurfaceFormats.cbegin(), supportedSurfaceFormats.cend(), requestedSurfaceFormat) != supportedSurfaceFormats.cend()) {
							surfaceFormat = requestedSurfaceFormat;
							break;
						}
					}
				}
				::vk::PresentModeKHR presentMode{};
				if (this->_requirePresentMode.has_value()) {
					if (std::find(supportedPresentModes.cbegin(), supportedPresentModes.cend(), *this->_requirePresentMode) != supportedPresentModes.cend())
						presentMode = *this->_requirePresentMode;
					else
						throw std::runtime_error("[Vulkan SwapchainBuilder] No supported present mode for the surface.");
				}
				else {
					presentMode = ::vk::PresentModeKHR::eFifo;
					for (const auto& requestedPresentMode : this->_requestPresentModes) {
						if (std::find(supportedPresentModes.cbegin(), supportedPresentModes.cend(), requestedPresentMode) != supportedPresentModes.cend()) {
							presentMode = requestedPresentMode;
							break;
						}
					}
				}
				::vk::Extent2D extent{};
				if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max() && capabilities.currentExtent.height != std::numeric_limits<std::uint32_t>::max()) {
					extent = capabilities.currentExtent;
				}
				else {
					extent = extent_;
				}
				extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
				std::uint32_t minImageCount = capabilities.minImageCount + 1;
				if (capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount)
					minImageCount = capabilities.maxImageCount;
				::vk::SwapchainCreateInfoKHR swapchainCreateInfo = ::vk::SwapchainCreateInfoKHR()
					.setFlags(::vk::SwapchainCreateFlagsKHR(0U))
					.setSurface(this->_surface)
					.setMinImageCount(minImageCount)
					.setImageFormat(surfaceFormat.format)
					.setImageColorSpace(surfaceFormat.colorSpace)
					.setImageExtent(extent)
					.setImageArrayLayers(1)
					.setImageUsage(::vk::ImageUsageFlagBits::eColorAttachment)
					.setImageSharingMode(::vk::SharingMode::eExclusive)
					.setQueueFamilyIndices(*this->_pContext->queueFamilyIndex(Context::QueueType::Main))
					.setPreTransform(capabilities.currentTransform)
					.setCompositeAlpha(::vk::CompositeAlphaFlagBitsKHR::eOpaque)
					.setPresentMode(presentMode)
					.setClipped(VK_TRUE)
					.setOldSwapchain(*oldSwapchain_._swapchain);
				Swapchain swapchain{nullptr};
				swapchain._pContext = this->_pContext;
				swapchain._surface = this->_surface;
				swapchain._swapchain = ::vk::raii::SwapchainKHR(this->_pContext->device(), swapchainCreateInfo);
				swapchain._surfaceFormat = surfaceFormat;
				swapchain._presentMode = presentMode;
				swapchain._extent = extent;
				swapchain._images = swapchain._swapchain.getImages();
				swapchain._numImages = static_cast<std::uint32_t>(swapchain._images.size());
				swapchain._imageViews.reserve(swapchain._images.size());
				::vk::ImageViewCreateInfo imageViewCreateInfo = ::vk::ImageViewCreateInfo()
					.setFlags(::vk::ImageViewCreateFlags(0))
					.setImage(nullptr)
					.setViewType(::vk::ImageViewType::e2D)
					.setFormat(swapchain._surfaceFormat.format)
					.setComponents(::vk::ComponentMapping(::vk::ComponentSwizzle::eIdentity, ::vk::ComponentSwizzle::eIdentity, ::vk::ComponentSwizzle::eIdentity, ::vk::ComponentSwizzle::eIdentity))
					.setSubresourceRange(::vk::ImageSubresourceRange(::vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
				for (std::size_t i = 0; i < swapchain._images.size(); ++i) {
					imageViewCreateInfo.setImage(swapchain._images[i]);
					swapchain._imageViews.emplace_back(this->_pContext->device(), imageViewCreateInfo);
				}
				oldSwapchain_.~Swapchain();
				return swapchain;
			}

		private:

			const Context* _pContext;
			::vk::SurfaceKHR _surface;
			std::vector<::vk::SurfaceFormatKHR> _requestSurfaceFormats{};
			std::optional<::vk::SurfaceFormatKHR> _requireSurfaceFormat = std::nullopt;
			std::vector<::vk::PresentModeKHR> _requestPresentModes{};
			std::optional<::vk::PresentModeKHR> _requirePresentMode = std::nullopt;

		};

	}

}

#endif /* jjyou_vk_Swapchain_hpp */