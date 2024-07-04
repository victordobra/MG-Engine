#pragma once

#include "Instance/VulkanAllocator.hpp"
#include "Instance/VulkanCommandPool.hpp"
#include "Instance/VulkanDevice.hpp"
#include "Instance/VulkanInstance.hpp"
#include "Instance/VulkanSurface.hpp"
#include "Instance/VulkanSwapChain.hpp"
#include "Loader/VulkanLoader.hpp"

#include <Core.hpp>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

namespace wfe {
	/// @brief A renderer that uses the Vulkan API.
	class VulkanRenderer {
	public:
	 	/// @brief The maximum number of frames in flight at any moment.
	 	static const size_t MAX_FRAMES_IN_FLIGHT = 2;
		/// @brief The allocation callbacks used by all Vulkan functions.
		static const VkAllocationCallbacks VULKAN_ALLOC_CALLBACKS;

		/// @brief Creates a renderer that uses the Vulkan API.
		/// @param window The window the renderer will display to, or nullptr if the renderer will be compute only.
		/// @param debugEnabled True if debugging should be enabled, otherwise false.
		/// @param logger The logger to use for general messages.
		VulkanRenderer(Window* window, bool8_t debugEnabled, Logger* logger);
		VulkanRenderer(const VulkanRenderer&) = delete;
		VulkanRenderer(VulkanRenderer&&) noexcept = delete;

		VulkanRenderer& operator=(const VulkanRenderer&) = delete;
		VulkanRenderer& operator=(VulkanRenderer&&) = delete;

		/// @brief Gets the Vulkan render's window.
		/// @return A pointer to renderer's window, or nullptr if the renderer is compute only.
		Window* GetWindow() {
			return window;
		}
		/// @brief Gets the Vulkan render's window.
		/// @return A const pointer to renderer's window, or nullptr if the renderer is compute only.
		const Window* GetWindow() const {
			return window;
		}
		/// @brief Gets the Vulkan renderer's logger.
		/// @return A pointer to the renderer's logger.
		Logger* GetLogger() {
			return logger;
		}
		/// @brief Gets the Vulkan renderer's logger.
		/// @return A pointer to the Vulkan renderer's logger.
		const Logger* GetLogger() const {
			return logger;
		}

		/// @brief Gets the Vulkan renderer's loader.
		/// @return A pointer to the Vulkan renderer's loader.
		VulkanLoader* GetLoader() {
			return loader;
		}
		/// @brief Gets the Vulkan renderer's loader.
		/// @return A const pointer to the Vulkan renderer's loader.
		const VulkanLoader* GetLoader() const {
			return loader;
		}
		/// @brief Gets the Vulkan renderer's instance.
		/// @return A pointer to the Vulkan renderer's instance.
		VulkanInstance* GetInstance() {
			return instance;
		}
		/// @brief Gets the Vulkan renderer's instance.
		/// @return A const pointer to the Vulkan renderer's instance.
		const VulkanInstance* GetInstance() const {
			return instance;
		}
		/// @brief Gets the Vulkan renderer's surface.
		/// @return A pointer to the Vulkan renderer's surface.
		VulkanSurface* GetSurface() {
			return surface;
		}
		/// @brief Gets the Vulkan renderer's surface.
		/// @return A const pointer to the Vulkan renderer's surface.
		const VulkanSurface* GetSurface() const {
			return surface;
		}
		/// @brief Gets the Vulkan renderer's device.
		/// @return A pointer to the Vulkan renderer's device.
		VulkanDevice* GetDevice() {
			return device;
		}
		/// @brief Gets the Vulkan renderer's device.
		/// @return A const pointer to the Vulkan renderer's device.
		const VulkanDevice* GetDevice() const {
			return device;
		}
		/// @brief Gets the Vulkan renderer's graphics command pool.
		/// @return A pointer to the Vulkan renderer's graphics command pool.
		VulkanCommandPool* GetGraphicsCommandPool() {
			return graphicsCommandPool;
		}
		/// @brief Gets the Vulkan renderer's graphics command pool.
		/// @return A const pointer to the Vulkan renderer's graphics command pool.
		const VulkanCommandPool* GetGraphicsCommandPool() const {
			return graphicsCommandPool;
		}
		/// @brief Gets the Vulkan renderer's transfer command pool.
		/// @return A pointer to the Vulkan renderer's transfer command pool.
		VulkanCommandPool* GetTransferCommandPool() {
			return transferCommandPool;
		}
		/// @brief Gets the Vulkan renderer's transfer command pool.
		/// @return A const pointer to the Vulkan renderer's transfer command pool.
		const VulkanCommandPool* GetTransferCommandPool() const {
			return transferCommandPool;
		}
		/// @brief Gets the Vulkan renderer's compute command pool.
		/// @return A pointer to the Vulkan renderer's compute command pool.
		VulkanCommandPool* GetComputeCommandPool() {
			return computeCommandPool;
		}
		/// @brief Gets the Vulkan renderer's compute command pool.
		/// @return A const pointer to the Vulkan renderer's compute command pool.
		const VulkanCommandPool* GetComputeCommandPool() const {
			return computeCommandPool;
		}
		/// @brief Gets the Vulkan renderer's allocator.
		/// @return A pointer to the Vulkan renderer's allocator.
		VulkanAllocator* GetAllocator() {
			return allocator;
		}
		/// @brief Gets the Vulkan renderer's allocator.
		/// @return A const pointer to the Vulkan renderer's allocator.
		const VulkanAllocator* GetAllocator() const {
			return allocator;
		}
		/// @brief Gets the Vulkan renderer's swap chain.
		/// @return A pointer to the Vulkan renderer's swap chain.
		VulkanSwapChain* GetSwapChain() {
			return swapChain;
		}
		/// @brief Gets the Vulkan renderer's swap chain.
		/// @return A const pointer to the Vulkan renderer's swap chain.
		const VulkanSwapChain* GetSwapChain() const {
			return swapChain;
		}

		/// @brief Destroys the Vulkan renderer.
		~VulkanRenderer();
	private:
		Window* window;
		Logger* logger;

		VulkanLoader* loader;
		VulkanInstance* instance;
		VulkanSurface* surface;
		VulkanDevice* device;
		VulkanCommandPool* graphicsCommandPool;
		VulkanCommandPool* transferCommandPool;
		VulkanCommandPool* computeCommandPool;
		VulkanAllocator* allocator;
		VulkanSwapChain* swapChain;
	};
}
