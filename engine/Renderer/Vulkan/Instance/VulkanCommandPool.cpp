#include "VulkanCommandPool.hpp"
#include "Renderer/Vulkan/VulkanRenderer.hpp"

#include <vulkan/vk_enum_string_helper.h>

namespace wfe {
	// Internal helper functions
	void VulkanCommandPool::CreateCommandPools(Thread::ThreadID threadID) {
		// Create the command pools
		ThreadCommandPools threadCommandPools;
		for(size_t i = 0; i != Renderer::MAX_FRAMES_IN_FLIGHT; ++i) {
			VkResult result = device->GetLoader()->vkCreateCommandPool(device->GetDevice(), &createInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &threadCommandPools.commandPools[i]);
			if(result != VK_SUCCESS)
				throw Exception("Failed to create Vulkan command pool! Error code: %s", string_VkResult(result));
		}
		
		// Add the command pools to the map
		commandPools.insert({ threadID, threadCommandPools });
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
		for(auto threadCommandPools : commandPools) {
			for(size_t i = 0; i != Renderer::MAX_FRAMES_IN_FLIGHT; ++i)
				device->GetLoader()->vkDestroyCommandPool(device->GetDevice(), threadCommandPools.second.commandPools[i], &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
		}
	}
}