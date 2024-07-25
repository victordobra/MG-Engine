#include "VulkanImage.hpp"
#include "Renderer/Core/GPUImage.hpp"
#include <vulkan/vk_enum_string_helper.h>

namespace wfe {
	// Internal helper functions
	void VulkanImage::CreateImage(VkImageType imageType, VkFormat format, uint32_t mipLevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageViewType viewType, VulkanAllocator::MemoryType memoryType) {
		// Save all of the device's queue families to an array
		VulkanDevice::QueueFamilyIndices indices = renderer->GetDevice()->GetQueueFamilyIndices();
		uint32_t indicesArr[4], indicesCount = 0;

		// Insert all unique indices into the index array; simple ifs should work here
		if(indices.graphicsIndex != UINT32_T_MAX)
			indicesArr[indicesCount++] = indices.graphicsIndex;
		if(indices.presentIndex != UINT32_T_MAX && indices.presentIndex != indices.graphicsIndex)
			indicesArr[indicesCount++] = indices.presentIndex;
		if(indices.transferIndex != UINT32_T_MAX && indices.transferIndex != indices.graphicsIndex && indices.transferIndex != indices.presentIndex)
			indicesArr[indicesCount++] = indices.transferIndex;
		if(indices.computeIndex != UINT32_T_MAX && indices.computeIndex != indices.graphicsIndex && indices.computeIndex != indices.presentIndex && indices.computeIndex != indices.transferIndex)
			indicesArr[indicesCount++] = indices.computeIndex;

		// Set the image create info
		VkImageCreateInfo imageInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = imageType,
			.format = format,
			.extent = imageExtent,
			.mipLevels = mipLevels,
			.arrayLayers = arrayLayers,
			.samples = samples,
			.tiling = tiling,
			.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			.sharingMode = VK_SHARING_MODE_CONCURRENT,
			.queueFamilyIndexCount = indicesCount,
			.pQueueFamilyIndices = indicesArr,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};

		// Create the image
		VkResult result = renderer->GetLoader()->vkCreateImage(renderer->GetDevice()->GetDevice(), &imageInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &image);
		if(result != VK_SUCCESS)
			throw Exception("Failed to create Vulkan image! Error code: %s", string_VkResult(result));
		
		// Allocate the image's memory
		result = renderer->GetAllocator()->AllocImageMemory(image, memoryType, imageMemory);
		if(result != VK_SUCCESS)
			throw Exception("Failed to allocate Vulkan image memory! Error code: %s", string_VkResult(result));

		// Bind the image's memory
		result = renderer->GetAllocator()->BindImageMemories(1, &image, &imageMemory);
		if(result != VK_SUCCESS)
			throw Exception("Failed to bind Vulkan image memory! Error code: %s", string_VkResult(result));

		// Set the image aspect mask based on the image's format
		VkImageAspectFlags aspectMask;
		if(format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		} else if(format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_X8_D24_UNORM_PACK32) {
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		} else if(format == VK_FORMAT_S8_UINT) {
			aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
		} else {
			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		// Set the image's subresource range
		subresourceRange = {
			.aspectMask = aspectMask,
			.baseMipLevel = 0,
			.levelCount = mipLevels,
			.baseArrayLayer = 0,
			.layerCount = arrayLayers
		};
		
		// Set the image view create info
		VkImageViewCreateInfo imageViewInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = image,
			.viewType = viewType,
			.format = format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange = subresourceRange
		};

		// Create the image view
		result = renderer->GetLoader()->vkCreateImageView(renderer->GetDevice()->GetDevice(), &imageViewInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &imageView);
		if(result != VK_SUCCESS)
			throw Exception("Failed to create Vulkan image view! Error code: %s", string_VkResult(result));

		// Set the command buffer alloc info
		VkCommandBufferAllocateInfo allocInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = renderer->GetTransferCommandPool()->GetCommandPool(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};

		// Allocate the command buffer
		VkCommandBuffer commandBuffer;
		result = renderer->GetLoader()->vkAllocateCommandBuffers(renderer->GetDevice()->GetDevice(), &allocInfo, &commandBuffer);
		if(result != VK_SUCCESS)
			throw Exception("Failed to allocate Vulkan command buffer! Error code: %s", string_VkResult(result));
		
		// Set the command buffer begin info
		VkCommandBufferBeginInfo beginInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};

		// Begin recording the command buffer
		result = renderer->GetLoader()->vkBeginCommandBuffer(commandBuffer, &beginInfo);
		if(result != VK_SUCCESS)
			throw Exception("Failed to begin recording Vulkan command buffer! Error code: %s", string_VkResult(result));
		
		// Set the image memory barrier info
		VkImageMemoryBarrier memoryBarrier {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_GENERAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {
				.aspectMask = aspectMask,
				.baseMipLevel = 0,
				.levelCount = mipLevels,
				.baseArrayLayer = 0,
				.layerCount = arrayLayers
			}
		};

		// Record the image layout transition to the command buffer
		renderer->GetLoader()->vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);

		// End recording the command buffer
		result = renderer->GetLoader()->vkEndCommandBuffer(commandBuffer);
		if(result != VK_SUCCESS)
			throw Exception("Failed to end recording Vulkan command buffer! Error code: %s", string_VkResult(result));
		
		// Set the fence create info
		VkFenceCreateInfo fenceInfo {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0
		};

		// Create the fence
		VkFence fence;
		result = renderer->GetLoader()->vkCreateFence(renderer->GetDevice()->GetDevice(), &fenceInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &fence);
		if(result != VK_SUCCESS)
			throw Exception("Failed to create Vulkan fence! Error code: %s", string_VkResult(result));
		
		// Set the command buffer submit info
		VkSubmitInfo submitInfo {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = 0,
			.pWaitSemaphores = nullptr,
			.pWaitDstStageMask = nullptr,
			.commandBufferCount = 1,
			.pCommandBuffers = &commandBuffer,
			.signalSemaphoreCount = 0,
			.pSignalSemaphores = nullptr
		};

		// Submit the command buffer
		result = renderer->GetLoader()->vkQueueSubmit(renderer->GetDevice()->GetTransferQueue(), 1, &submitInfo, fence);
		if(result != VK_SUCCESS)
			throw Exception("Failed to submit Vulkan command buffer! Error code: %s", string_VkResult(result));
		
		// Wait for the command buffer to finish execution
		result = renderer->GetLoader()->vkWaitForFences(renderer->GetDevice()->GetDevice(), 1, &fence, VK_TRUE, UINT64_T_MAX);
		if(result != VK_SUCCESS)
			throw Exception("Failed to wait for Vulkan fence! Error code: %s", string_VkResult(result));
		
		// Destroy the fence and free the command buffer
		renderer->GetLoader()->vkDestroyFence(renderer->GetDevice()->GetDevice(), fence, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
		renderer->GetLoader()->vkFreeCommandBuffers(renderer->GetDevice()->GetDevice(), renderer->GetTransferCommandPool()->GetCommandPool(), 1, &commandBuffer);
	}

	// Public functions
	VkImageType VulkanImage::ImageTypeToVkImageType(GPUImageType imageType) {
		switch(imageType) {
		case GPU_IMAGE_TYPE_1D:
			return VK_IMAGE_TYPE_1D;
		case GPU_IMAGE_TYPE_2D:
			return VK_IMAGE_TYPE_2D;
		case GPU_IMAGE_TYPE_3D:
			return VK_IMAGE_TYPE_3D;
		default:
			return VK_IMAGE_TYPE_MAX_ENUM;
		}
	}
	VkImageViewType VulkanImage::ImageTypeToVkImageViewType(GPUImageType imageType) {
		switch(imageType) {
		case GPU_IMAGE_TYPE_1D:
			return VK_IMAGE_VIEW_TYPE_1D;
		case GPU_IMAGE_TYPE_2D:
			return VK_IMAGE_VIEW_TYPE_2D;
		case GPU_IMAGE_TYPE_3D:
			return VK_IMAGE_VIEW_TYPE_3D;
		default:
			return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
		}
	}
	VkFormat VulkanImage::ImageFormatToVkFormat(GPUImageFormat imageFormat) {
		switch(imageFormat) {
		case GPU_IMAGE_FORMAT_R8_UNORM:
			return VK_FORMAT_R8_UNORM;
		case GPU_IMAGE_FORMAT_R8_SNORM:
			return VK_FORMAT_R8_SNORM;
		case GPU_IMAGE_FORMAT_R8_USCALED:
			return VK_FORMAT_R8_USCALED;
		case GPU_IMAGE_FORMAT_R8_SSCALED:
			return VK_FORMAT_R8_SSCALED;
		case GPU_IMAGE_FORMAT_R8_UINT:
			return VK_FORMAT_R8_UINT;
		case GPU_IMAGE_FORMAT_R8_SINT:
			return VK_FORMAT_R8_SINT;
		case GPU_IMAGE_FORMAT_R8_SRGB:
			return VK_FORMAT_R8_SRGB;
		case GPU_IMAGE_FORMAT_R8G8_UNORM:
			return VK_FORMAT_R8G8_UNORM;
		case GPU_IMAGE_FORMAT_R8G8_SNORM:
			return VK_FORMAT_R8G8_SNORM;
		case GPU_IMAGE_FORMAT_R8G8_USCALED:
			return VK_FORMAT_R8G8_USCALED;
		case GPU_IMAGE_FORMAT_R8G8_SSCALED:
			return VK_FORMAT_R8G8_SSCALED;
		case GPU_IMAGE_FORMAT_R8G8_UINT:
			return VK_FORMAT_R8G8_UINT;
		case GPU_IMAGE_FORMAT_R8G8_SINT:
			return VK_FORMAT_R8G8_SINT;
		case GPU_IMAGE_FORMAT_R8G8_SRGB:
			return VK_FORMAT_R8G8_SRGB;
		case GPU_IMAGE_FORMAT_R8G8B8_UNORM:
			return VK_FORMAT_R8G8B8_UNORM;
		case GPU_IMAGE_FORMAT_R8G8B8_SNORM:
			return VK_FORMAT_R8G8B8_SNORM;
		case GPU_IMAGE_FORMAT_R8G8B8_USCALED:
			return VK_FORMAT_R8G8B8_USCALED;
		case GPU_IMAGE_FORMAT_R8G8B8_SSCALED:
			return VK_FORMAT_R8G8B8_SSCALED;
		case GPU_IMAGE_FORMAT_R8G8B8_UINT:
			return VK_FORMAT_R8G8B8_UINT;
		case GPU_IMAGE_FORMAT_R8G8B8_SINT:
			return VK_FORMAT_R8G8B8_SINT;
		case GPU_IMAGE_FORMAT_R8G8B8_SRGB:
			return VK_FORMAT_R8G8B8_SRGB;
		case GPU_IMAGE_FORMAT_R8G8B8A8_UNORM:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case GPU_IMAGE_FORMAT_R8G8B8A8_SNORM:
			return VK_FORMAT_R8G8B8A8_SNORM;
		case GPU_IMAGE_FORMAT_R8G8B8A8_USCALED:
			return VK_FORMAT_R8G8B8A8_USCALED;
		case GPU_IMAGE_FORMAT_R8G8B8A8_SSCALED:
			return VK_FORMAT_R8G8B8A8_SSCALED;
		case GPU_IMAGE_FORMAT_R8G8B8A8_UINT:
			return VK_FORMAT_R8G8B8A8_UINT;
		case GPU_IMAGE_FORMAT_R8G8B8A8_SINT:
			return VK_FORMAT_R8G8B8A8_SINT;
		case GPU_IMAGE_FORMAT_R8G8B8A8_SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case GPU_IMAGE_FORMAT_R16_UNORM:
			return VK_FORMAT_R16_UNORM;
		case GPU_IMAGE_FORMAT_R16_SNORM:
			return VK_FORMAT_R16_SNORM;
		case GPU_IMAGE_FORMAT_R16_USCALED:
			return VK_FORMAT_R16_USCALED;
		case GPU_IMAGE_FORMAT_R16_SSCALED:
			return VK_FORMAT_R16_SSCALED;
		case GPU_IMAGE_FORMAT_R16_UINT:
			return VK_FORMAT_R16_UINT;
		case GPU_IMAGE_FORMAT_R16_SINT:
			return VK_FORMAT_R16_SINT;
		case GPU_IMAGE_FORMAT_R16_SFLOAT:
			return VK_FORMAT_R16_SFLOAT;
		case GPU_IMAGE_FORMAT_R16G16_UNORM:
			return VK_FORMAT_R16G16_UNORM;
		case GPU_IMAGE_FORMAT_R16G16_SNORM:
			return VK_FORMAT_R16G16_SNORM;
		case GPU_IMAGE_FORMAT_R16G16_USCALED:
			return VK_FORMAT_R16G16_USCALED;
		case GPU_IMAGE_FORMAT_R16G16_SSCALED:
			return VK_FORMAT_R16G16_SSCALED;
		case GPU_IMAGE_FORMAT_R16G16_UINT:
			return VK_FORMAT_R16G16_UINT;
		case GPU_IMAGE_FORMAT_R16G16_SINT:
			return VK_FORMAT_R16G16_SINT;
		case GPU_IMAGE_FORMAT_R16G16_SFLOAT:
			return VK_FORMAT_R16G16_SFLOAT;
		case GPU_IMAGE_FORMAT_R16G16B16_UNORM:
			return VK_FORMAT_R16G16B16_UNORM;
		case GPU_IMAGE_FORMAT_R16G16B16_SNORM:
			return VK_FORMAT_R16G16B16_SNORM;
		case GPU_IMAGE_FORMAT_R16G16B16_USCALED:
			return VK_FORMAT_R16G16B16_USCALED;
		case GPU_IMAGE_FORMAT_R16G16B16_SSCALED:
			return VK_FORMAT_R16G16B16_SSCALED;
		case GPU_IMAGE_FORMAT_R16G16B16_UINT:
			return VK_FORMAT_R16G16B16_UINT;
		case GPU_IMAGE_FORMAT_R16G16B16_SINT:
			return VK_FORMAT_R16G16B16_SINT;
		case GPU_IMAGE_FORMAT_R16G16B16_SFLOAT:
			return VK_FORMAT_R16G16B16_SFLOAT;
		case GPU_IMAGE_FORMAT_R16G16B16A16_UNORM:
			return VK_FORMAT_R16G16B16A16_UNORM;
		case GPU_IMAGE_FORMAT_R16G16B16A16_SNORM:
			return VK_FORMAT_R16G16B16A16_SNORM;
		case GPU_IMAGE_FORMAT_R16G16B16A16_USCALED:
			return VK_FORMAT_R16G16B16A16_USCALED;
		case GPU_IMAGE_FORMAT_R16G16B16A16_SSCALED:
			return VK_FORMAT_R16G16B16A16_SSCALED;
		case GPU_IMAGE_FORMAT_R16G16B16A16_UINT:
			return VK_FORMAT_R16G16B16A16_UINT;
		case GPU_IMAGE_FORMAT_R16G16B16A16_SFLOAT:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case GPU_IMAGE_FORMAT_R32_UINT:
			return VK_FORMAT_R32_UINT;
		case GPU_IMAGE_FORMAT_R32_SINT:
			return VK_FORMAT_R32_SINT;
		case GPU_IMAGE_FORMAT_R32_SFLOAT:
			return VK_FORMAT_R32_SFLOAT;
		case GPU_IMAGE_FORMAT_R32G32_UINT:
			return VK_FORMAT_R32G32_UINT;
		case GPU_IMAGE_FORMAT_R32G32_SINT:
			return VK_FORMAT_R32G32_SINT;
		case GPU_IMAGE_FORMAT_R32G32_SFLOAT:
			return VK_FORMAT_R32G32_SFLOAT;
		case GPU_IMAGE_FORMAT_R32G32B32_UINT:
			return VK_FORMAT_R32G32B32_UINT;
		case GPU_IMAGE_FORMAT_R32G32B32_SINT:
			return VK_FORMAT_R32G32B32_SINT;
		case GPU_IMAGE_FORMAT_R32G32B32_SFLOAT:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case GPU_IMAGE_FORMAT_R32G32B32A32_UINT:
			return VK_FORMAT_R32G32B32A32_UINT;
		case GPU_IMAGE_FORMAT_R32G32B32A32_SINT:
			return VK_FORMAT_R32G32B32A32_SINT;
		case GPU_IMAGE_FORMAT_R32G32B32A32_SFLOAT:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		default:
			return VK_FORMAT_MAX_ENUM;
		}
	}

	VulkanImage::VulkanImage(Renderer* renderer, uint32_t width, uint32_t height, uint32_t depth, GPUImageType imageType, GPUImageFormat imageFormat, bool8_t canMap) : renderer((VulkanRenderer*)renderer->GetRendererBackend()), imageExtent({ width, height, depth }) {
		// Create the image
		CreateImage(ImageTypeToVkImageType(imageType), ImageFormatToVkFormat(imageFormat), 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, ImageTypeToVkImageViewType(imageType), canMap ? VulkanAllocator::MEMORY_TYPE_GPU_CPU_VISIBLE : VulkanAllocator::MEMORY_TYPE_GPU);
	}
	VulkanImage::VulkanImage(VulkanRenderer* renderer, VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t mipLevels, uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageViewType viewType, VulkanAllocator::MemoryType memoryType) : renderer(renderer), imageExtent(extent) {
		// Create the image
		CreateImage(imageType, format, mipLevels, arrayLayers, samples, tiling, viewType, memoryType);
	}

	VulkanImage::~VulkanImage() {
		// Destroy the image and image view
		renderer->GetLoader()->vkDestroyImageView(renderer->GetDevice()->GetDevice(), imageView, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
		renderer->GetLoader()->vkDestroyImage(renderer->GetDevice()->GetDevice(), image, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);

		// Free the image's memory
		renderer->GetAllocator()->FreeMemory(imageMemory);
	}
}