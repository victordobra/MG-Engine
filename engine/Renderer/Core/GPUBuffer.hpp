#pragma once

#include <Core.hpp>
#include "Renderer/Renderer.hpp"
#include "Renderer/Vulkan/Core/VulkanBuffer.hpp"

namespace wfe {
	/// @brief An implementation of a GPU memory buffer.
	class GPUBuffer {
	public:
		/// @brief Creates a GPU memory buffer.
		/// @param renderer The renderer to create the buffer in.
		/// @param size The size of the buffer.
		/// @param canMap True if the buffer can be mapped, otherwise false.
		GPUBuffer(Renderer* renderer, uint64_t size, bool8_t canMap) : api(renderer->GetRendererBackendAPI()) {
			// Use the constructor based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				new(internalData) VulkanBuffer(renderer, size, canMap);
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}

		GPUBuffer() = delete;
		GPUBuffer(const GPUBuffer&) = delete;
		GPUBuffer(GPUBuffer&&) noexcept = delete;

		GPUBuffer& operator=(const GPUBuffer&) = delete;
		GPUBuffer& operator=(GPUBuffer&&) noexcept = delete;

		/// @brief Maps the buffer's memory to RAM.
		void MapMemory() {
			// Call the map function for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanBuffer*)internalData)->MapMemory();
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Unmaps the buffer's memory from RAM.
		void UnmapMemory() {
			// Call the unmap function for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanBuffer*)internalData)->UnmapMemory();
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}

		/// @brief Gets the buffer's size.
		/// @return The buffer's size.
		uint64_t GetSize() const {
			// Call the get size function for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				return (uint64_t)((const VulkanBuffer*)internalData)->GetSize();
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Gets the buffer's mapped memory.
		/// @return A pointer to the buffer's mapped memory, or nullptr if the buffer isn't mapped.
		void* GetMappedMemory() {
			// Call the get mapped memory function for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				return ((VulkanBuffer*)internalData)->GetMappedMemory();
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Gets the buffer's mapped memory.
		/// @return A const pointer to the buffer's mapped memory, or nullptr if the buffer isn't mapped.
		const void* GetMappedMemory() const {
			// Call the get mapped memory function for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				return ((const VulkanBuffer*)internalData)->GetMappedMemory();
			default:
				throw Exception("Invalid renderer API!");
			}
		}

		/// @brief Gets the internal buffer implementation data, which can be used cased on the renderer's API.
		/// @return A void pointer to the internal implementation's class.
		void* GetInternalData() {
			return internalData;
		}

		/// @brief Destroys the GPU memory buffer.
		~GPUBuffer() {
			// Call the destructor for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanBuffer*)internalData)->~VulkanBuffer();
				break;
			}
		}
	private:
		Renderer::RendererBackendAPI api;
		char internalData[sizeof(VulkanBuffer)];
	};
}