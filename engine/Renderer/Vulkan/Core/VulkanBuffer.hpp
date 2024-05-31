#pragma once

#include <Core.hpp>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#include "Renderer/Renderer.hpp"
#include "Renderer/Vulkan/VulkanRenderer.hpp"

namespace wfe {
	/// @brief An implementation of a GPU memory buffer using the Vulkan API.
	class VulkanBuffer {
	public:
		/// @brief Creates a GPU memory buffer using the Vulkan API.
		/// @param renderer The renderer to create the buffer in.
		/// @param size The size of the buffer.
		/// @param canMap True if the buffer can be mapped, otherwise false.
		VulkanBuffer(Renderer* renderer, uint64_t size, bool8_t canMap);
		/// @brief Creates a GPU memory buffer using the Vulkan API.
		/// @param renderer The renderer to create the buffer in.
		/// @param size The size of the buffer.
		/// @param memoryType The memory type of the memory binded to the buffer.
		VulkanBuffer(VulkanRenderer* renderer, VkDeviceSize size, VulkanAllocator::MemoryType memoryType);

		VulkanBuffer() = delete;
		VulkanBuffer(const VulkanBuffer&) = delete;
		VulkanBuffer(VulkanBuffer&&) noexcept = delete;

		/// @brief Maps the buffer's memory to RAM.
		void MapMemory();
		/// @brief Unmaps the buffer's memory from RAM.
		void UnmapMemory();

		/// @brief Gets the internal Vulkan buffer's handle.
		/// @return The internal Vulkan buffer's handle.
		VkBuffer GetBuffer() {
			return buffer;
		}
		/// @brief Gets the internal Vulkan buffer's memory block.
		/// @return A struct containing info about the memory block bound to this buffer.
		VulkanAllocator::MemoryBlock GetBufferMemory() {
			return bufferMemory;
		}
		/// @brief Gets the buffer's size.
		/// @return The buffer's size.
		VkDeviceSize GetSize() const {
			return size;
		}
		/// @brief Gets the buffer's mapped memory.
		/// @return A pointer to the buffer's mapped memory, or nullptr if the buffer isn't mapped.
		void* GetMappedMemory() {
			return mappedMemory;
		}
		/// @brief Gets the buffer's mapped memory.
		/// @return A const pointer to the buffer's mapped memory, or nullptr if the buffer isn't mapped.
		const void* GetMappedMemory() const {
			return mappedMemory;
		}

		/// @brief Destroys the Vulkan GPU memory buffer.
		~VulkanBuffer();
	private:
		void CreateBuffer(VulkanAllocator::MemoryType memoryType);

		VulkanRenderer* renderer;
		VkBuffer buffer;
		VulkanAllocator::MemoryBlock bufferMemory;
		VkDeviceSize size;
		void* mappedMemory;
	};
}