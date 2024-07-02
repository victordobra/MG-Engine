#pragma once

#include <Core.hpp>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#include "Renderer/Renderer.hpp"
#include "Renderer/Core/GPUImageEnums.hpp"
#include "Renderer/Vulkan/VulkanRenderer.hpp"

namespace wfe {
    class VulkanImage {
    public:
		/// @brief Converts the given image type to its corresponding VkImageType.
		/// @param imageType The image type to convert.
		/// @return The corresponding VkImageType.
		static VkImageType ImageTypeToVkImageType(GPUImageType imageType);
		/// @brief Converts the given image type to its corresponding VkImageViewType.
		/// @param imageType The image type to convert.
		/// @return The corresponding VkImageViewType.
		static VkImageViewType ImageTypeToVkImageViewType(GPUImageType imageType);
		/// @brief Converts the given image format to its corresponding VkFormat.
		/// @param imageFormat The image format to convert.
		/// @return The corresponding VkFormat.
		static VkFormat ImageFormatToVkFormat(GPUImageFormat imageFormat);

		/// @brief Creates a GPU image using the Vulkan API.
		/// @param renderer The renderer to create the image in.
		/// @param width The image's width.
		/// @param height The image's height.
		/// @param depth The image's depth.
		/// @param imageType The image's type.
		/// @param imageFormat The image's format.
		/// @param canMap True if the image can be mapped, otherwise false.
		VulkanImage(Renderer* renderer, uint32_t width, uint32_t height, uint32_t depth, GPUImageType imageType, GPUImageFormat imageFormat, bool8_t canMap);
		/// @brief Creates a GPU image using the Vulkan API.
		/// @param renderer The Vulkan renderer to create the image in.
		/// @param imageType The image's type.
		/// @param format The image's format.
		/// @param extent The image's extent.
		/// @param mipLevels The number of levels of detail available for minified sampling.
		/// @param arrayLayers The number of layers in the image.
		/// @param samples The number of samples per texel.
		/// @param tiling The tiling arrangement of the texel blocks in memory.
		/// @param viewType The image view's type.
		/// @param memoryType The memory type of the memory binded to the buffer.
		VulkanImage(VulkanRenderer* renderer, VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t mipLevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageViewType viewType, VulkanAllocator::MemoryType memoryType);

		VulkanImage() = delete;
		VulkanImage(const VulkanImage&) = delete;
		VulkanImage(VulkanImage&&) noexcept = delete;

		VulkanImage& operator=(const VulkanImage&) = delete;
		VulkanImage& operator=(VulkanImage&&) = delete;

		/// @brief Gets the internal Vulkan image's handle.
		/// @return The internal Vulkan image's handle.
		VkImage GetImage() {
			return image;
		}
		/// @brief Gets the internal Vulkan image view's handle.
		/// @return The internal Vulkan image view's handle.
		VkImageView GetImageView() {
			return imageView;
		}
		/// @brief Gets the internal Vulkan image's memory block.
		/// @return A struct containing info about the memory block bound to this image.
		VulkanAllocator::MemoryBlock GetImageMemory() {
			return imageMemory;
		}
		/// @brief Gets the image's extent.
		/// @return The image's extent.
		VkExtent3D GetImageExtent() const {
			return imageExtent;
		}
		/// @brief Gets the image's width.
		/// @return The image's width.
		uint32_t GetWidth() const {
			return imageExtent.width;
		}
		/// @brief Gets the image's height.
		/// @return The image's height.
		uint32_t GetHeight() const {
			return imageExtent.height;
		}
		/// @brief Gets the image's depth.
		/// @return The image's depth.
		uint32_t GetDepth() const {
			return imageExtent.depth;
		}
		/// @brief Gets the image's mapped memory.
		/// @return A pointer to the image's mapped memory, or nullptr if the image isn't mapped.
		void* GetMappedMemory() {
			return renderer->GetAllocator()->GetMappedMemory(imageMemory);
		}
		/// @brief Gets the image's mapped memory.
		/// @return A const pointer to the image's mapped memory, or nullptr if the image isn't mapped.
		const void* GetMappedMemory() const {
			return renderer->GetAllocator()->GetMappedMemory(imageMemory);
		}

		/// @brief Destroys the Vulkan GPU image.
		~VulkanImage();
	private:
		void CreateImage(VkImageType imageType, VkFormat format, uint32_t mipLevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageViewType viewType, VulkanAllocator::MemoryType memoryType);

		VulkanRenderer* renderer;
		VkImage image;
		VkImageView imageView;
		VulkanAllocator::MemoryBlock imageMemory;
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkExtent3D imageExtent;
    };
}