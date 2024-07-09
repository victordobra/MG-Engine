#pragma once

#include <Core.hpp>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#include "Renderer/Renderer.hpp"
#include "Renderer/Vulkan/VulkanRenderer.hpp"

namespace wfe {
	/// @brief An implementtion of a GPU fence using the Vulkan API.
	class VulkanFence {
	public:
		/// @brief Creates a GPU fence using the Vulkan API.
		/// @param renderer The renderer to create the fence in.
		/// @param signaled True if the fence starts off signaled, otherwise false.
		VulkanFence(Renderer* renderer, bool8_t signaled);
		/// @brief Creates a GPU fence using the Vulkan API.
		/// @param renderer The renderer to create the fence in.
		/// @param flags The fence's create flags.
		VulkanFence(VulkanRenderer* renderer, VkFenceCreateFlags flags);

		VulkanFence() = delete;
		VulkanFence(const VulkanFence&) = delete;
		VulkanFence(VulkanFence&&) noexcept = delete;

		VulkanFence& operator=(const VulkanFence&) = delete;
		VulkanFence& operator=(VulkanFence&&) noexcept = delete;
	
		/// @brief Gets the internal Vulkan fence's handle.
		/// @return The internal Vulkan fence's handle.
		VkFence GetFence() {
			return fence;
		}
		/// @brief Checks if the fence is signaled.
		/// @return True if the fence is signaled, otherwise false.
		bool8_t IsSignaled();
		/// @brief Waits for the fence to be signaled.
		/// @param timeout The maximum time, in nanoseconds, the command can waot for the fence to be signaled.
		/// @return True if the wait operation succeeded before the timeout, otherwise false.
		bool8_t Wait(uint64_t timeout);
		/// @brief Resets the fence back to the unsignaled state.
		void Reset();

		/// @brief Destroys the Vulkan GPU fence.
		~VulkanFence();
	private:
		VulkanRenderer* renderer;
		VkFence fence;
	};
}