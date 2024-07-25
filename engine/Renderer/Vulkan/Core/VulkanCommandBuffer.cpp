#include "VulkanCommandBuffer.hpp"
#include "Renderer/Core/GPUCommandBuffer.hpp"
#include <vulkan/vk_enum_string_helper.h>

namespace wfe {
	// Internal helper functions
	void VulkanCommandBuffer::AllocCommandBuffer() {
		// Set the command buffer's command pool
		VkCommandPool commandPool;
		switch(type) {
		case GPU_COMMAND_BUFFER_TYPE_GRAPHICS:
			commandPool = renderer->GetGraphicsCommandPool()->GetCommandPool();
			break;
		case GPU_COMMAND_BUFFER_TYPE_COMPUTE:
			commandPool = renderer->GetComputeCommandPool()->GetCommandPool();
			break;
		case GPU_COMMAND_BUFFER_TYPE_TRANSFER:
			commandPool = renderer->GetTransferCommandPool()->GetCommandPool();
			break;
		}

		// Convert the command buffer level flag
		VkCommandBufferLevel commandBufferLevel;
		switch(level) {
		case GPU_COMMAND_BUFFER_LEVEL_PRIMARY:
			commandBufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			break;
		case GPU_COMMAND_BUFFER_LEVEL_SECONDARY:
			commandBufferLevel = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			break;
		}

		// Set the command buffer alloc info
		VkCommandBufferAllocateInfo allocInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = commandPool,
			.level = commandBufferLevel,
			.commandBufferCount = 1
		};

		// Allocate the command buffer
		VkResult result = renderer->GetLoader()->vkAllocateCommandBuffers(renderer->GetDevice()->GetDevice(), &allocInfo, &commandBuffer);
		if(result != VK_SUCCESS)
			throw Exception("Failed to allocate Vulkan command buffer! Error code: %s", string_VkResult(result));
	}
	void VulkanCommandBuffer::TransitionImageLayout(VulkanImage* image, VkImageLayout newLayout) {
		// Get the image's current layout
		VkImageLayout oldLayout = imageLayouts.insert({ image, VK_IMAGE_LAYOUT_GENERAL }).first->second;

		// Exit the function if the old layout is the same as the new
		if(oldLayout == newLayout)
			return;
		
		// Set the image memory barrier info
		VkImageMemoryBarrier memoryBarrier {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
			.oldLayout = oldLayout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image->GetImage(),
			.subresourceRange = image->GetImageSubresourceRange()
		};

		// Record the image layout transition pipeline barrier
		renderer->GetLoader()->vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);

		// Set the new image layout
		imageLayouts[image] = newLayout;
	}

	// Public functions
	VkPipelineStageFlags VulkanCommandBuffer::PipelineStageToVkPipelineStageFlags(GPUPipelineStage pipelineStage) {
		// Add the equivalent Vulkan flag for eevry flag present in the pipeline stage
		VkPipelineStageFlags stageFlags = 0;

		if(pipelineStage & GPU_PIPELINE_STAGE_PIPELINE_START)
			stageFlags |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		if(pipelineStage & GPU_PIPELINE_STAGE_DRAW_INDIRECT)
			stageFlags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
		if(pipelineStage & GPU_PIPELINE_STAGE_VERTEX_INPUT)
			stageFlags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
		if(pipelineStage & GPU_PIPELINE_STAGE_VERTEX_SHADER)
			stageFlags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
		if(pipelineStage & GPU_PIPELINE_STAGE_FRAGMENT_SHADER)
			stageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		if(pipelineStage & GPU_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT)
			stageFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		if(pipelineStage & GPU_PIPELINE_STAGE_COMPUTE_SHADER)
			stageFlags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		if(pipelineStage & GPU_PIPELINE_STAGE_TRANSFER)
			stageFlags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		if(pipelineStage & GPU_PIPELINE_STAGE_PIPELINE_END)
			stageFlags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		if(pipelineStage & GPU_PIPELINE_STAGE_ALL_GRAPHICS)
			stageFlags |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
		if(pipelineStage & GPU_PIPELINE_STAGE_ALL_COMMANDS)
			stageFlags |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		
		return stageFlags;
	}

	VulkanCommandBuffer::VulkanCommandBuffer(Renderer* renderer, GPUCommandBufferLevel level, GPUCommandBufferType type) : renderer((VulkanRenderer*)renderer->GetRendererBackend()), level(level), type(type) {
		// Alocate the command buffer
		AllocCommandBuffer();
	}
	VulkanCommandBuffer::VulkanCommandBuffer(VulkanRenderer* renderer, GPUCommandBufferLevel level, GPUCommandBufferType type) : renderer(renderer), level(level), type(type) {
		// Alocate the command buffer
		AllocCommandBuffer();
	}

	void VulkanCommandBuffer::BeginRecording() {
		// Set the command buffer inheritance info
		VkCommandBufferInheritanceInfo inheritanceInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			.pNext = nullptr,
			.renderPass = renderer->GetSwapChain()->GetRenderPass(),
			.subpass = 0,
			.framebuffer = VK_NULL_HANDLE,
			.occlusionQueryEnable = VK_FALSE,
			.queryFlags = 0,
			.pipelineStatistics = 0
		};

		// Set the command buffer begin info
		VkCommandBufferBeginInfo beginInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
			.pInheritanceInfo = &inheritanceInfo
		};

		// Begin recording the command buffer
		VkResult result = renderer->GetLoader()->vkBeginCommandBuffer(commandBuffer, &beginInfo);
		if(result != VK_SUCCESS)
			throw Exception("Failed to begin recording Vulkan command buffer! Error code: %s", string_VkResult(result));
	}
	void VulkanCommandBuffer::EndRecording() {
		// Allocate the image memory barrier array
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkImageMemoryBarrier* memoryBarriers = (VkImageMemoryBarrier*)AllocMemory(sizeof(VkImageMemoryBarrier) * imageLayouts.size());
		PopMemoryUsageType();
		if(imageLayouts.size() && !memoryBarriers)
			throw BadAllocException("Failed to allocate Vulkan image memory barrier array!");
		
		// Set the image memory barrier infos
		VkImageMemoryBarrier* memoryBarrier = memoryBarriers;
		for(auto imageLayout : imageLayouts) {
			memoryBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			memoryBarrier->pNext = nullptr;
			memoryBarrier->srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
			memoryBarrier->dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
			memoryBarrier->oldLayout = imageLayout.second;
			memoryBarrier->newLayout = VK_IMAGE_LAYOUT_GENERAL;
			memoryBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			memoryBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			memoryBarrier->image = imageLayout.first->GetImage();
			memoryBarrier->subresourceRange = imageLayout.first->GetImageSubresourceRange();

			++memoryBarrier;
		}

		// Record the image layout reset pipeline barrier
		renderer->GetLoader()->vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, (uint32_t)imageLayouts.size(), memoryBarriers);

		// Free the image memory barriers array
		FreeMemory(memoryBarriers);

		// End recording the command buffer
		VkResult result = renderer->GetLoader()->vkEndCommandBuffer(commandBuffer);
		if(result != VK_SUCCESS)
			throw Exception("Failed to end recording Vulkan command buffer! Error code: %s", string_VkResult(result));
		
		// Clear the image layouts map
		imageLayouts.clear();
	}
	void VulkanCommandBuffer::Reset() {
		// Reset the command buffer
		VkResult result = renderer->GetLoader()->vkResetCommandBuffer(commandBuffer, 0);
		if(result != VK_SUCCESS)
			throw Exception("Failed to reset Vulkan command buffer! Error code: %s", string_VkResult(result));
	}

	void VulkanCommandBuffer::CmdClearColorImage(GPUImage& image, GPUColorImageClearValue clearValue) {
		// Transition the image's layout
		VulkanImage* vulkanImage = (VulkanImage*)image.GetInternalData();
		TransitionImageLayout(vulkanImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// Set the image's clear color value and subresource range
		VkClearColorValue clearColorValue {
			.float32 = { clearValue.clearFloat[0], clearValue.clearFloat[1], clearValue.clearFloat[2], clearValue.clearFloat[3] }
		};
		VkImageSubresourceRange subresourceRange = vulkanImage->GetImageSubresourceRange();

		// Record the clear command
		renderer->GetLoader()->vkCmdClearColorImage(commandBuffer, vulkanImage->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColorValue, 1, &subresourceRange);
	}
	void VulkanCommandBuffer::CmdClearDepthStencilImage(GPUImage& image, float32_t depthValue, uint32_t stencilValue) {
		// Transition the image's layout
		VulkanImage* vulkanImage = (VulkanImage*)image.GetInternalData();
		TransitionImageLayout(vulkanImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// Set the image's clear depth stencil value and subresource range
		VkClearDepthStencilValue clearDepthStencilValue {
			.depth = depthValue,
			.stencil = stencilValue
		};
		VkImageSubresourceRange subresourceRange = vulkanImage->GetImageSubresourceRange();

		// Record the clear command
		renderer->GetLoader()->vkCmdClearDepthStencilImage(commandBuffer, vulkanImage->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearDepthStencilValue, 1, &subresourceRange);
	}
	void VulkanCommandBuffer::CmdFillBuffer(GPUBuffer& buffer, uint64_t offset, uint64_t size, uint32_t data) {
		// Record the fill command
		renderer->GetLoader()->vkCmdFillBuffer(commandBuffer, ((VulkanBuffer*)buffer.GetInternalData())->GetBuffer(), (VkDeviceSize)offset, (VkDeviceSize)size, data);
	}
	void VulkanCommandBuffer::CmdUpdateBuffer(GPUBuffer& buffer, uint64_t offset, uint64_t size, void* data) {
		// Record the update command
		renderer->GetLoader()->vkCmdUpdateBuffer(commandBuffer, ((VulkanBuffer*)buffer.GetInternalData())->GetBuffer(), (VkDeviceSize)offset, (VkDeviceSize)size, data);
	}
	void VulkanCommandBuffer::CmdCopyBuffer(GPUBuffer& srcBuffer, GPUBuffer& dstBuffer, size_t regionCount, const GPUBufferCopyRegion* regions) {
		// Allocate the copy region array
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkBufferCopy* copyRegions = (VkBufferCopy*)AllocMemory(sizeof(VkBufferCopy) * regionCount);
		PopMemoryUsageType();
		if(!copyRegions)
			throw BadAllocException("Failed to allocate Vulkan buffer copy regions array!");
		
		// Set the buffer copy regions
		for(size_t i = 0; i != regionCount; ++i) {
			copyRegions[i].srcOffset = (VkDeviceSize)regions[i].srcOffset;
			copyRegions[i].dstOffset = (VkDeviceSize)regions[i].dstOffset;
			copyRegions[i].size = (VkDeviceSize)regions[i].size;
		}

		// Record the copy command
		renderer->GetLoader()->vkCmdCopyBuffer(commandBuffer, ((VulkanBuffer*)srcBuffer.GetInternalData())->GetBuffer(), ((VulkanBuffer*)dstBuffer.GetInternalData())->GetBuffer(), (uint32_t)regionCount, copyRegions);

		// Free the copy region array
		FreeMemory(copyRegions);
	}
	void VulkanCommandBuffer::CmdCopyImage(GPUImage& srcImage, GPUImage& dstImage, size_t regionCount, const GPUImageCopyRegion* regions) {
		// Get the images' old layouts
		VulkanImage* vulkanSrcImage = (VulkanImage*)srcImage.GetInternalData();
		VulkanImage* vulkanDstImage = (VulkanImage*)dstImage.GetInternalData();
		VkImageLayout srcOldLayout = imageLayouts.insert({ vulkanSrcImage, VK_IMAGE_LAYOUT_GENERAL }).first->second;
		VkImageLayout dstOldLayout = imageLayouts.insert({ vulkanDstImage, VK_IMAGE_LAYOUT_GENERAL }).first->second;

		// Set the image memory barrier infos
		VkImageMemoryBarrier memoryBarriers[2];
		uint32_t memoryBarrierCount = 0;

		if(srcOldLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
			memoryBarriers[memoryBarrierCount].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			memoryBarriers[memoryBarrierCount].pNext = nullptr;
			memoryBarriers[memoryBarrierCount].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
			memoryBarriers[memoryBarrierCount].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
			memoryBarriers[memoryBarrierCount].oldLayout = srcOldLayout;
			memoryBarriers[memoryBarrierCount].newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			memoryBarriers[memoryBarrierCount].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			memoryBarriers[memoryBarrierCount].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			memoryBarriers[memoryBarrierCount].image = vulkanSrcImage->GetImage();
			memoryBarriers[memoryBarrierCount].subresourceRange = vulkanSrcImage->GetImageSubresourceRange();

			++memoryBarrierCount;
			imageLayouts[vulkanSrcImage] = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		}
		if(dstOldLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			memoryBarriers[memoryBarrierCount].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			memoryBarriers[memoryBarrierCount].pNext = nullptr;
			memoryBarriers[memoryBarrierCount].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
			memoryBarriers[memoryBarrierCount].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
			memoryBarriers[memoryBarrierCount].oldLayout = dstOldLayout;
			memoryBarriers[memoryBarrierCount].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			memoryBarriers[memoryBarrierCount].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			memoryBarriers[memoryBarrierCount].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			memoryBarriers[memoryBarrierCount].image = vulkanDstImage->GetImage();
			memoryBarriers[memoryBarrierCount].subresourceRange = vulkanDstImage->GetImageSubresourceRange();

			++memoryBarrierCount;
			imageLayouts[vulkanDstImage] = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		}

		// Record the image layout transition pipeline barrier, if one is required
		if(memoryBarrierCount)
			renderer->GetLoader()->vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, memoryBarrierCount, memoryBarriers);
		
		// Allocate the image copy region array
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkImageCopy* copyRegions = (VkImageCopy*)AllocMemory(sizeof(VkImageCopy) * regionCount);
		PopMemoryUsageType();
		if(!copyRegions)
			throw BadAllocException("Failed to allocate Vulkan image copy regions array!");
		
		// Set the image copy regions
		for(size_t i = 0; i != regionCount; ++i) {
			copyRegions[i].srcSubresource = {
				.aspectMask = vulkanSrcImage->GetImageSubresourceRange().aspectMask,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1
			};
			copyRegions[i].srcOffset = {
				.x = (int32_t)regions[i].srcOffset.x,
				.y = (int32_t)regions[i].srcOffset.y,
				.z = (int32_t)regions[i].srcOffset.z
			};
			copyRegions[i].dstSubresource = {
				.aspectMask = vulkanDstImage->GetImageSubresourceRange().aspectMask,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1
			};
			copyRegions[i].dstOffset = {
				.x = (int32_t)regions[i].dstOffset.x,
				.y = (int32_t)regions[i].dstOffset.y,
				.z = (int32_t)regions[i].dstOffset.z
			};
			copyRegions[i].extent = {
				.width = regions[i].size.width,
				.height = regions[i].size.height,
				.depth = regions[i].size.depth
			};
		}

		// Record the copy command
		renderer->GetLoader()->vkCmdCopyImage(commandBuffer, vulkanSrcImage->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vulkanDstImage->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (uint32_t)regionCount, copyRegions);

		// Free the copy region array
		FreeMemory(copyRegions);
	}
	void VulkanCommandBuffer::CmdCopyBufferToImage(GPUBuffer& buffer, GPUImage& image, size_t regionCount, const GPUBufferImageCopyRegion* regions) {
		// Transition the image's layout
		VulkanImage* vulkanImage = (VulkanImage*)image.GetInternalData();
		TransitionImageLayout(vulkanImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// Allocate the copy region array
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkBufferImageCopy* copyRegions = (VkBufferImageCopy*)AllocMemory(sizeof(VkBufferImageCopy) * regionCount);
		PopMemoryUsageType();
		if(!copyRegions)
			throw BadAllocException("Failed to allocate Vulkan buffer image copy regions array!");

		// Set the copy regions
		for(size_t i = 0; i != regionCount; ++i) {
			copyRegions[i].bufferOffset = (VkDeviceSize)regions[i].bufferOffset;
			copyRegions[i].bufferRowLength = 0;
			copyRegions[i].bufferImageHeight = 0;
			copyRegions[i].imageSubresource = {
				.aspectMask = vulkanImage->GetImageSubresourceRange().aspectMask,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1
			};
			copyRegions[i].imageOffset = {
				.x = (int32_t)regions[i].imageOffset.x,
				.y = (int32_t)regions[i].imageOffset.y,
				.z = (int32_t)regions[i].imageOffset.z
			};
			copyRegions[i].imageExtent = {
				.width = regions[i].size.width,
				.height = regions[i].size.height,
				.depth = regions[i].size.depth
			};
		}

		// Record the copy command
		renderer->GetLoader()->vkCmdCopyBufferToImage(commandBuffer, ((VulkanBuffer*)buffer.GetInternalData())->GetBuffer(), vulkanImage->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (uint32_t)regionCount, copyRegions);

		// Free the copy region array
		FreeMemory(copyRegions);
	}
	void VulkanCommandBuffer::CmdCopyImageToBuffer(GPUImage& image, GPUBuffer& buffer, size_t regionCount, const GPUBufferImageCopyRegion* regions) {
		// Transition the image's layout
		VulkanImage* vulkanImage = (VulkanImage*)image.GetInternalData();
		TransitionImageLayout(vulkanImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		// Allocate the copy region array
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkBufferImageCopy* copyRegions = (VkBufferImageCopy*)AllocMemory(sizeof(VkBufferImageCopy) * regionCount);
		PopMemoryUsageType();
		if(!copyRegions)
			throw BadAllocException("Failed to allocate Vulkan buffer image copy regions array!");

		// Set the copy regions
		for(size_t i = 0; i != regionCount; ++i) {
			copyRegions[i].bufferOffset = (VkDeviceSize)regions[i].bufferOffset;
			copyRegions[i].bufferRowLength = 0;
			copyRegions[i].bufferImageHeight = 0;
			copyRegions[i].imageSubresource = {
				.aspectMask = vulkanImage->GetImageSubresourceRange().aspectMask,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1
			};
			copyRegions[i].imageOffset = {
				.x = (int32_t)regions[i].imageOffset.x,
				.y = (int32_t)regions[i].imageOffset.y,
				.z = (int32_t)regions[i].imageOffset.z
			};
			copyRegions[i].imageExtent = {
				.width = regions[i].size.width,
				.height = regions[i].size.height,
				.depth = regions[i].size.depth
			};
		}

		// Record the copy command
		renderer->GetLoader()->vkCmdCopyImageToBuffer(commandBuffer, vulkanImage->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ((VulkanBuffer*)buffer.GetInternalData())->GetBuffer(), (uint32_t)regionCount, copyRegions);

		// Free the copy region array
		FreeMemory(copyRegions);
	}

	void VulkanCommandBuffer::CmdRunCommandBuffers(size_t commandBufferCount, GPUCommandBuffer* commandBuffers) {
		// Allocate the command buffer array
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkCommandBuffer* vulkanCommandBuffers = (VkCommandBuffer*)AllocMemory(sizeof(VkCommandBuffer) * commandBufferCount);
		PopMemoryUsageType();
		if(!vulkanCommandBuffers)
			throw BadAllocException("Failed to allocate Vulkan command buffer array!");
		
		// Set the command buffer array
		for(size_t i = 0; i != commandBufferCount; ++i)
			vulkanCommandBuffers[i] = ((VulkanCommandBuffer*)commandBuffers[i].GetInternalData())->GetCommandBuffer();
		
		// Record the command buffer run
		renderer->GetLoader()->vkCmdExecuteCommands(commandBuffer, (uint32_t)commandBufferCount, vulkanCommandBuffers);

		// Free the command buffer array
		FreeMemory(vulkanCommandBuffers);
	}

	VulkanCommandBuffer::~VulkanCommandBuffer() {
		// Get the command buffer's command pool
		VkCommandPool commandPool;
		switch(type) {
		case GPU_COMMAND_BUFFER_TYPE_GRAPHICS:
			commandPool = renderer->GetGraphicsCommandPool()->GetCommandPool();
			break;
		case GPU_COMMAND_BUFFER_TYPE_COMPUTE:
			commandPool = renderer->GetComputeCommandPool()->GetCommandPool();
			break;
		case GPU_COMMAND_BUFFER_TYPE_TRANSFER:
			commandPool = renderer->GetTransferCommandPool()->GetCommandPool();
			break;
		}

		// Free the command buffer
		renderer->GetLoader()->vkFreeCommandBuffers(renderer->GetDevice()->GetDevice(), commandPool, 1, &commandBuffer);
	}
}