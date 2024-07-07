#include "VulkanCommandPool.hpp"
#include "Renderer/Vulkan/VulkanRenderer.hpp"

#include <vulkan/vk_enum_string_helper.h>

namespace wfe {
	// Internal helper functions
	void VulkanCommandPool::CreateCommandPool(Thread::ThreadID threadID) {
		// Create the command pool
		VkCommandPool commandPool;
		VkResult result = device->GetLoader()->vkCreateCommandPool(device->GetDevice(), &createInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &commandPool);
		if(result != VK_SUCCESS)
			throw Exception("Failed to create Vulkan command pool! Error code: %s", string_VkResult(result));
		
		// Add the command pool to the map
		commandPools.insert({ threadID, commandPool });
	}

	// Public functions
	VulkanCommandPool::VulkanCommandPool(VulkanDevice* device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags commandPoolFlags) : device(device) {
		// Set the command pool create info
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = commandPoolFlags;
		createInfo.queueFamilyIndex = queueFamilyIndex;
	}

	VulkanCommandPool::~VulkanCommandPool() {
		// Destroy the command pools
		for(auto commandPool : commandPools)
			device->GetLoader()->vkDestroyCommandPool(device->GetDevice(), commandPool.second, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
	}
}