#pragma once

#include "Renderer/Renderer.hpp"
#include "VulkanDevice.hpp"

#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

namespace wfe {
	/// @brief A wrapper for multiple Vulkan command pools, one for each frame in flight.
	class VulkanCommandPool {
	public:
		/// @brief Creates a Vulkan command pool for the given device and queue family index.
		/// @param device The Vulkan to create the command pool for.
		/// @param queueFamilyIndex The queue family index corresponding to the command pool.
		/// @param commandPoolFlags The flags to use when creating the command pool.
		VulkanCommandPool(VulkanDevice* device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags commandPoolFlags);
		VulkanCommandPool(const VulkanCommandPool&) = delete;
		VulkanCommandPool(VulkanCommandPool&&) noexcept = delete;

		VulkanCommandPool& operator=(const VulkanCommandPool&) = delete;
		VulkanCommandPool& operator=(VulkanCommandPool&&) noexcept = delete;

		/// @brief Gets the Vulkan function loader used by the command pool.
		/// @return A pointer to the Vulkan loader used by the command pool.
		const VulkanLoader* GetLoader() const {
			return device->GetLoader();
		}
		/// @brief Gets the Vulkan device that owns the command pool.
		/// @return A pointer to the Vulkan device wrapper object.
		VulkanDevice* GetDevice() {
			return device;
		}
		/// @brief Gets the Vulkan device that owns the command pool.
		/// @return A const pointer to the Vulkan device wrapper object.
		const VulkanDevice* GetDevice() const {
			return device;
		}
		/// @brief Gets the Vulkan command pool of the implementation for the current thread.
		/// @return A handle to the Vulkan command pool.
		VkCommandPool GetCommandPool(size_t index) {
			// Get the current thread's command pools, create them if they don't exist
			Thread::ThreadID threadID = GetCurrentThreadID();
			if(!commandPools.count(threadID))
				CreateCommandPool(threadID);
			return commandPools[threadID];
		}

		/// @brief Destroys the command pool.
		~VulkanCommandPool();
	private:
		void CreateCommandPool(Thread::ThreadID threadID);

		VulkanDevice* device;
		VkCommandPoolCreateInfo createInfo;
		std::unordered_map<Thread::ThreadID, VkCommandPool> commandPools;
	};
}