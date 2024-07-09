#pragma once

#include <Core.hpp>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#include "Renderer/Renderer.hpp"
#include "Renderer/Vulkan/VulkanRenderer.hpp"

namespace wfe {
	/// @brief An implementation af a GPU semaphore using the Vulkan API.
	class VulkanSemaphore {
	public:
		/// @brief Creates a GPU semaphore using the Vulkan API.
		/// @param renderer The renderer to create the semaphore in.
		VulkanSemaphore(Renderer* renderer);
		/// @brief Creates a GPU semaphore using the Vulkan API.
		/// @param renderer The renderer to create the semaphore in.
		VulkanSemaphore(VulkanRenderer* renderer);

		VulkanSemaphore() = delete;
		VulkanSemaphore(const VulkanSemaphore&) = delete;
		VulkanSemaphore(VulkanSemaphore&&) noexcept = delete;

		VulkanSemaphore& operator=(const VulkanSemaphore&) = delete;
		VulkanSemaphore& operator=(VulkanSemaphore&&) noexcept = delete;

		/// @brief Gets the internal Vulkan semaphore's handle.
		/// @return The internal Vulkan semaphore's handle.
		VkSemaphore GetSemaphore() {
			return semaphore;
		}

		/// @brief Destroys the Vulkan GPU semaphore.
		~VulkanSemaphore();
	private:
		VulkanRenderer* renderer;
		VkSemaphore semaphore;
	};
}