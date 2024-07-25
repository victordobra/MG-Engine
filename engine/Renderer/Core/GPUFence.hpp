#pragma once

#include <Core.hpp>
#include "Renderer/Renderer.hpp"
#include "Renderer/Vulkan/Core/VulkanFence.hpp"

namespace wfe {
	/// @brief An implementation of a GPU fence.
	class GPUFence {
	public:
		/// @brief Creates a GPU fence.
		/// @param renderer The renderer to create the fence in.
		/// @param signaled True if the fence starts off signaled, otherwise false.
		GPUFence(Renderer* renderer, bool8_t signaled) : api(renderer->GetRendererBackendAPI()) {
			// Use the constructor based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				new(internalData) VulkanFence(renderer, signaled);
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}

		GPUFence() = delete;
		GPUFence(const GPUFence&) = delete;
		GPUFence(GPUFence&&) noexcept = delete;

		GPUFence& operator=(const GPUFence&) = delete;
		GPUFence& operator=(GPUFence&&) = delete;

		/// @brief Checks if the fence is signaled.
		/// @return True if the fence is signaled, otherwise false.
		bool8_t IsSignaled() {
			// Call the is signaled function for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				return ((VulkanFence*)internalData)->IsSignaled();
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Waits for the fence to be signaled.
		/// @param timeout The maximum time, in nanoseconds, the command can waot for the fence to be signaled.
		/// @return True if the wait operation succeeded before the timeout, otherwise false.
		bool8_t Wait(uint64_t timeout) {
			// Call the wait function for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				return ((VulkanFence*)internalData)->Wait(timeout);
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Resets the fence back to the unsignaled state.
		void Reset() {
			// Call the reset function for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanFence*)internalData)->Reset();
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}

		/// @brief Gets the internal buffer implementation data, which can be used based on the renderer's API.
		/// @return A void pointer to the internal implementation's class.
		void* GetInternalData() {
			return internalData;
		}

		/// @brief Destroys the GPU fence.
		~GPUFence() {
			// Call the destructor for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanFence*)internalData)->~VulkanFence();
				break;
			}
		}
	private:
		char internalData[sizeof(VulkanFence)];
		Renderer::RendererBackendAPI api;
	};
}