#pragma once

#include <Core.hpp>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#include "Renderer/Core/GPUCommandBufferStructs.hpp"
#include "Renderer/Core/GPUBuffer.hpp"
#include "Renderer/Core/GPUImage.hpp"

namespace wfe {
	class GPUCommandBuffer;

	/// @brief An implementation of a GPU command buffer using the Vulkan API.
	class VulkanCommandBuffer {
	public:
		/// @brief Converts the given pipeline stage to its corresponding VkPipelineStageFlags.
		/// @param pipelineStage The pipeline stage to convert.
		/// @return THe corresponding VkPipelineStageFlags.
		static VkPipelineStageFlags PipelineStageToVkPipelineStageFlags(GPUPipelineStage pipelineStage);

		/// @brief Creates a GPU command buffer using the Vulkan API.
		/// @param renderer The renderer to create the command buffer in.
		/// @param level The command buffer's submission level.
		/// @param type The command buffer's type.
		VulkanCommandBuffer(Renderer* renderer, GPUCommandBufferLevel level, GPUCommandBufferType type);
		/// @brief Creates a GPU command buffer using the Vulkan API.
		/// @param renderer The renderer to create the command buffer in.
		/// @param level The command buffer's submission level.
		/// @param type The command buffer's type.
		VulkanCommandBuffer(VulkanRenderer* renderer, GPUCommandBufferLevel level, GPUCommandBufferType type);

		/// @brief Gets the command buffer's level.
		/// @return The command buffer's level.
		GPUCommandBufferLevel GetLevel() const {
			return level;
		}
		/// @brief Gets the command buffer's type.
		/// @return The command buffer's type.
		GPUCommandBufferType GetType() const {
			return type;
		}
		/// @brief Gets the internal Vulkan command buffer's handle.
		/// @return The internal Vulkan command buffer's handle.
		VkCommandBuffer GetCommandBuffer() {
			return commandBuffer;
		}

		/// @brief Begins recording the command buffer.
		void BeginRecording();
		/// @brief Ends recording the command buffer.
		void EndRecording();
		/// @brief Resets the command buffer.
		void Reset();

		/// @brief Records a command which clears the given color image.
		/// @param image The color image to clear.
		/// @param clearValue A union containing the clear values used on the image.
		void CmdClearColorImage(GPUImage& image, GPUColorImageClearValue clearValue);
		/// @brief Records a command which clears the given depth/stencil image.
		/// @param image The depth/stencil image to clear.
		/// @param depthValue The depth clear value used on the image (if a depth component exists).
		/// @param stencilValue The stencil clear value used on the iamge (if a stencil component exists).
		void CmdClearDepthStencilImage(GPUImage& image, float32_t depthValue, uint32_t stencilValue);
		/// @brief Records a command which fills the given buffer.
		/// @param buffer The buffer to fill.
		/// @param offset The offset into the buffer at which to start filling. Must be a multiple of 4.
		/// @param size The number of bytes to fill. Must me a multiple of 4.
		/// @param data The 4-byte word repeated across the filled region.
		void CmdFillBuffer(GPUBuffer& buffer, uint64_t offset, uint64_t size, uint32_t data);
		/// @brief Records a command which updates the given buffer. Only recommended for small updates.
		/// @param buffer The buffer to update.
		/// @param offset The offset into the buffer at which to start updating. Must be a multiple of 4.
		/// @param size The updated area's size. Must be a multiple of 4.
		/// @param data The source data for the buffer update.
		void CmdUpdateBuffer(GPUBuffer& buffer, uint64_t offset, uint64_t size, void* data);
		/// @brief Records a command which copies data from one buffer to another.
		/// @param srcBuffer The source buffer.
		/// @param dstBuffer The destination buffer.
		/// @param regionCount The number of copy regions.
		/// @param regions A pointer to the array of copy regions.
		void CmdCopyBuffer(GPUBuffer& srcBuffer, GPUBuffer& dstBuffer, size_t regionCount, const GPUBufferCopyRegion* regions);
		/// @brief Records a command which copies data from one image to another.
		/// @param srcImage The source image.
		/// @param dstImage The destination image.
		/// @param regionCount The number of copy regions.
		/// @param regions A pointer to the array of copy regions.
		void CmdCopyImage(GPUImage& srcImage, GPUImage& dstImage, size_t regionCount, const GPUImageCopyRegion* regions);
		/// @brief Records a command which copies data from a buffer to an image.
		/// @param buffer The source buffer.
		/// @param image The destination image.
		/// @param regionCount The number of copy regions.
		/// @param regions A pointer to the array of copy regions. 
		void CmdCopyBufferToImage(GPUBuffer& buffer, GPUImage& image, size_t regionCount, const GPUBufferImageCopyRegion* regions);
		/// @brief Records a command which copies data from an image to a buffer.
		/// @param image The source image.
		/// @param buffer The destination buffer.
		/// @param regionCount The number of copy regions.
		/// @param regions A pointer to the array of copy regions. 
		void CmdCopyImageToBuffer(GPUImage& image, GPUBuffer& buffer, size_t regionCount, const GPUBufferImageCopyRegion* regions);

		/// @brief Records a command which runs one or more secondary command buffers.
		/// @param commandBufferCount The number of command buffers to run.
		/// @param commandBuffers A pointer to the array of command buffers.
		void CmdRunCommandBuffers(size_t commandBufferCount, GPUCommandBuffer* commandBuffers);

		/// @brief Destroys the Vulkan GPU command buffer.
		~VulkanCommandBuffer();
	private:
		void AllocCommandBuffer();
		void TransitionImageLayout(VulkanImage* image, VkImageLayout newLayout);

		VulkanRenderer* renderer;
		GPUCommandBufferLevel level;
		GPUCommandBufferType type;
		VkCommandBuffer commandBuffer;
		std::unordered_map<VulkanImage*, VkImageLayout> imageLayouts;
	};
}