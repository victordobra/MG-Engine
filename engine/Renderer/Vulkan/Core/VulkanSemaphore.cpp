#include "VulkanSemaphore.hpp"
#include "Renderer/Core/GPUSemaphore.hpp"
#include <vulkan/vk_enum_string_helper.h>

namespace wfe {
	// Public functions
	VulkanSemaphore::VulkanSemaphore(Renderer* renderer) : renderer((VulkanRenderer*)renderer->GetRendererBackend()) {
		// Set the semaphore create info
		VkSemaphoreCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0
		};

		// Create the semaphore
		VkResult result = this->renderer->GetLoader()->vkCreateSemaphore(this->renderer->GetDevice()->GetDevice(), &createInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &semaphore);
		if(result != VK_SUCCESS)
			throw Exception("Failed to create Vulkan semaphore! Error code: %s", string_VkResult(result));
	}
	VulkanSemaphore::VulkanSemaphore(VulkanRenderer* renderer) : renderer(renderer) {
		// Set the semaphore create info
		VkSemaphoreCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0
		};

		// Create the semaphore
		VkResult result = renderer->GetLoader()->vkCreateSemaphore(renderer->GetDevice()->GetDevice(), &createInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &semaphore);
		if(result != VK_SUCCESS)
			throw Exception("Failed to create Vulkan semaphore! Error code: %s", string_VkResult(result));
	}

	VulkanSemaphore::~VulkanSemaphore() {
		// Destroy the semaphore
		renderer->GetLoader()->vkDestroySemaphore(renderer->GetDevice()->GetDevice(), semaphore, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
	}
}