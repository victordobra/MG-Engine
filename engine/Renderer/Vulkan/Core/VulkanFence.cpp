#include "VulkanFence.hpp"
#include "Renderer/Core/GPUFence.hpp"
#include <vulkan/vk_enum_string_helper.h>

namespace wfe {
	// Public functions
	VulkanFence::VulkanFence(Renderer* renderer, bool8_t signaled) : renderer((VulkanRenderer*)renderer->GetRendererBackend()) {
		// Set the fence create info
		VkFenceCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u
		};

		// Create the fence
		VkResult result = this->renderer->GetLoader()->vkCreateFence(this->renderer->GetDevice()->GetDevice(), &createInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &fence);
		if(result != VK_SUCCESS)
			throw Exception("Failed to create Vulkan fence! Error code: %s", string_VkResult(result));
	}
    VulkanFence::VulkanFence(VulkanRenderer* renderer, VkFenceCreateFlags flags) : renderer(renderer) {
		// Set the fence create info
		VkFenceCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = flags
		};

		// Create the fence
		VkResult result = renderer->GetLoader()->vkCreateFence(renderer->GetDevice()->GetDevice(), &createInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &fence);
		if(result != VK_SUCCESS)
			throw Exception("Failed to create Vulkan fence! Error code: %s", string_VkResult(result));
	}

	bool8_t VulkanFence::IsSignaled() {
		// Check the fence's status
		VkResult result = renderer->GetLoader()->vkGetFenceStatus(renderer->GetDevice()->GetDevice(), fence);
		if(result != VK_SUCCESS && result != VK_NOT_READY)
			throw Exception("Failed to get Vulkan fence status! Error code: %s", string_VkResult(result));
		
		return result == VK_SUCCESS;
	}
	bool8_t VulkanFence::Wait(uint64_t timeout) {
		// Wait for the fence to be signaled
		VkResult result = renderer->GetLoader()->vkWaitForFences(renderer->GetDevice()->GetDevice(), 1, &fence, VK_TRUE, timeout);
		if(result != VK_SUCCESS && result != VK_NOT_READY)
			throw Exception("Failed to wait for Vulkan fence! Error code: %s", string_VkResult(result));
		
		return result == VK_SUCCESS;
	}
	void VulkanFence::Reset() {
		// Reset the fence
		VkResult result = renderer->GetLoader()->vkResetFences(renderer->GetDevice()->GetDevice(), 1, &fence);
		if(result != VK_SUCCESS)
			throw Exception("Failed to reset Vulkan fence! Error code: %s", string_VkResult(result));
	}

	VulkanFence::~VulkanFence() {
		// Destroy the fence
		renderer->GetLoader()->vkDestroyFence(renderer->GetDevice()->GetDevice(), fence, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
	}
}