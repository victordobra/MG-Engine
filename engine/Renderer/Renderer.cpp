#include "Renderer.hpp"
#include "Vulkan/VulkanRenderer.hpp"

namespace wfe {
	Renderer::Renderer(Window* window, bool8_t debugEnabled, Logger* logger) {
		// Try to create the Vulkan renderer
		try {
			PushMemoryUsageType(MEMORY_USAGE_TYPE_RENDERER);
			rendererBackend = NewObject<VulkanRenderer>(window, debugEnabled, logger);
			PopMemoryUsageType();
			rendererBackendAPI = RENDERER_BACKEND_API_VULKAN;
			return;
		} catch(const UnsupportedAPIException&) { }

		// No renderer backend was successfully created; throw an exception
		throw UnsupportedAPIException("Failed to find an implemented renderer API supported by the current machine!");
	}

	Renderer::~Renderer() {
		// Destroy the renderer backend based on its API
		switch(rendererBackendAPI) {
		case RENDERER_BACKEND_API_VULKAN:
			DestroyObject((VulkanRenderer*)rendererBackend);
			break;
		}
	}
}