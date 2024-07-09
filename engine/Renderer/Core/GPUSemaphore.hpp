#pragma once

#include <Core.hpp>
#include "Renderer/Renderer.hpp"
#include "Renderer/Vulkan/Core/VulkanSemaphore.hpp"

namespace wfe {
	/// @brief An implementation of a GPU semaphore.
	class GPUSemaphore {
	public:
		/// @brief Creates a GPU semaphore.
		/// @param renderer The renderer to create the semaphore in.
		GPUSemaphore(Renderer* renderer) : api(renderer->GetRendererBackendAPI()) {
			// Use the constructor based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				new(internalData) VulkanSemaphore(renderer);
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		
		GPUSemaphore() = delete;
		GPUSemaphore(const GPUSemaphore&) = delete;
		GPUSemaphore(GPUSemaphore&&) noexcept = delete;

		GPUSemaphore& operator=(const GPUSemaphore&) = delete;
		GPUSemaphore& operator=(GPUSemaphore&&) = delete;

		/// @brief Gets the internal buffer implementation data, which can be used based on the renderer's API.
		/// @return A void pointer to the internal implementation's class.
		void* GetInternalData() {
			return internalData;
		}

		/// @brief Destroys the GPU semaphore.
		~GPUSemaphore() {
			// Call the destructor for the renderer's API
			switch(api) {
				((VulkanSemaphore*)internalData)->~VulkanSemaphore();
				break;
			}
		}
	private:
		Renderer::RendererBackendAPI api;
		char internalData[sizeof(VulkanSemaphore)];
	};
}