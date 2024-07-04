#include "VulkanRenderer.hpp"

namespace wfe {
	// Alloc callbacks
	static void* VKAPI_CALL AllocCallback(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocScope) {
		// Set the memory usage type based on the alloc scope
		MemoryUsageType memoryUsageType;
		if(allocScope == VK_SYSTEM_ALLOCATION_SCOPE_COMMAND)
			memoryUsageType = MEMORY_USAGE_TYPE_COMMAND;
		else
			memoryUsageType = MEMORY_USAGE_TYPE_RENDERER;

		// Allocate the requested memory
		PushMemoryUsageType(memoryUsageType);
		void* mem = AllocMemory(size);
		PopMemoryUsageType();

		return mem;
	}
	static void* VKAPI_CALL ReallocCallback(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocScope) {
		// Reallocate the given memory
		return ReallocMemory(pOriginal, size);
	}
	static void VKAPI_CALL FreeCallback(void* pUserData, void* pMemory) {
		// Free the given memory
		FreeMemory(pMemory);
	}

	const VkAllocationCallbacks VulkanRenderer::VULKAN_ALLOC_CALLBACKS = {
		nullptr,
		AllocCallback,
		ReallocCallback,
		FreeCallback,
		nullptr,
		nullptr
	};

	// Public functions
	VulkanRenderer::VulkanRenderer(Window* window, bool8_t debugEnabled, Logger* logger) : window(window), logger(logger) {
		// Set the renderer memory usage
		PushMemoryUsageType(MEMORY_USAGE_TYPE_RENDERER);

		// Create the loader
		loader = NewObject<VulkanLoader>();

		// Create the instance and set the loader's instance
		instance = NewObject<VulkanInstance>(loader, debugEnabled, logger);
		loader->LoadInstanceFunctions(instance->GetInstance());

		// Create the surface, if a window is given
		if(window) {
			surface = NewObject<VulkanSurface>(instance, window);
		} else {
			surface = nullptr;
		}

		// Try to create the device
		try {
			device = NewObject<VulkanDevice>(loader, instance, surface);
		} catch(const Renderer::UnsupportedAPIException& exception) {
			// Destroy the previously created objects
			if(surface)
				DestroyObject(surface);
			DestroyObject(instance);
			DestroyObject(loader);

			// Pass the exception back
			throw exception;
		}

		// Set the loader's device
		loader->LoadDeviceFunctions(device->GetDevice());

		// Create all command pools
		graphicsCommandPool = NewObject<VulkanCommandPool>(device, device->GetQueueFamilyIndices().graphicsIndex, 0);
		transferCommandPool = NewObject<VulkanCommandPool>(device, device->GetQueueFamilyIndices().transferIndex, 0);
		computeCommandPool = NewObject<VulkanCommandPool>(device, device->GetQueueFamilyIndices().computeIndex, 0);

		// Create the allocator
		allocator = NewObject<VulkanAllocator>(device);

		// Create the swap chain, if a window is given
		if(window) {
			swapChain = NewObject<VulkanSwapChain>(surface, device, allocator);
		} else {
			swapChain = nullptr;
		}

		// Pop the memory usage
		PopMemoryUsageType();
	}

	VulkanRenderer::~VulkanRenderer() {
		// Destroy the core objects
		if(swapChain)
			DestroyObject(swapChain);
		DestroyObject(allocator);
		DestroyObject(graphicsCommandPool);
		DestroyObject(transferCommandPool);
		DestroyObject(computeCommandPool);
		DestroyObject(device);
		if(surface)
			DestroyObject(surface);
		DestroyObject(instance);

		// Destroy the loader
		DestroyObject(loader);
	}
}