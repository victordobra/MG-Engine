#include "VulkanBuffer.hpp"
#include "Renderer/Core/GPUBuffer.hpp"
#include <vulkan/vk_enum_string_helper.h>

namespace wfe {
	// Internal helper functions
	void VulkanBuffer::CreateBuffer(VulkanAllocator::MemoryType memoryType) {
		// Save all of the device's queue families to an array
		VulkanDevice::QueueFamilyIndices indices = renderer->GetDevice()->GetQueueFamilyIndices();
		uint32_t indicesArr[4], indicesCount = 0;

		// Insert all unique indices into the index array; simple ifs should work here
		if(indices.graphicsIndex != UINT32_T_MAX)
			indicesArr[indicesCount++] = indices.graphicsIndex;
		if(indices.presentIndex != UINT32_T_MAX && indices.presentIndex != indices.graphicsIndex)
			indicesArr[indicesCount++] = indices.presentIndex;
		if(indices.transferIndex != indices.graphicsIndex && indices.transferIndex != indices.presentIndex)
			indicesArr[indicesCount++] = indices.transferIndex;
		if(indices.computeIndex != indices.graphicsIndex && indices.computeIndex != indices.presentIndex && indices.computeIndex != indices.transferIndex)
			indicesArr[indicesCount++] = indices.computeIndex;

		// Set the buffer create info
		VkBufferCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.size = (VkDeviceSize)size,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
			.sharingMode = VK_SHARING_MODE_CONCURRENT,
			.queueFamilyIndexCount = indicesCount,
			.pQueueFamilyIndices = indicesArr
		};

		// Create the buffer
		VkResult result = renderer->GetLoader()->vkCreateBuffer(renderer->GetDevice()->GetDevice(), &createInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &buffer);
		if(result != VK_SUCCESS)
			throw Exception("Failed to create Vulkan buffer! Error code: %s", string_VkResult(result));
		
		// Allocate the buffer's memory
		result = renderer->GetAllocator()->AllocBufferMemory(buffer, memoryType, bufferMemory);
		if(result != VK_SUCCESS)
			throw Exception("Failed to allocate Vulkan buffer memory! Error code: %s", string_VkResult(result));
	}

	// Public functions
	VulkanBuffer::VulkanBuffer(Renderer* renderer, uint64_t size, bool8_t canMap) : renderer((VulkanRenderer*)renderer->GetRendererBackend()), size(size) {
		// Set the memory type based on if the buffer can be mapped
		VulkanAllocator::MemoryType memoryType;
		if(canMap) {
			memoryType = VulkanAllocator::MEMORY_TYPE_GPU_CPU_VISIBLE;
		} else {
			memoryType = VulkanAllocator::MEMORY_TYPE_GPU;
		}

		// Create the buffer
		CreateBuffer(memoryType);
	}
	VulkanBuffer::VulkanBuffer(VulkanRenderer* renderer, VkDeviceSize size, VulkanAllocator::MemoryType memoryType) : renderer(renderer), size(size) {
		// Create the buffer
		CreateBuffer(memoryType);
	}

	void VulkanBuffer::MapMemory() {
		// TODO: Implement memory mapping in allocator
	}
	void VulkanBuffer::UnmapMemory() {
		// TODO: Implement memory mapping in allocator
	}

	VulkanBuffer::~VulkanBuffer() {
		// Destroy the buffer
		renderer->GetLoader()->vkDestroyBuffer(renderer->GetDevice()->GetDevice(), buffer, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);

		// Free the buffer's memory
		renderer->GetAllocator()->FreeMemory(bufferMemory);
	}
}