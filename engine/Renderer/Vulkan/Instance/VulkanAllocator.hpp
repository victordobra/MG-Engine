#pragma once

#include "VulkanDevice.hpp"

#include <Core.hpp>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

namespace wfe {
	/// @brief An implementation of an efficient Vulkan device memory allocator.
	class VulkanAllocator {
	public:
		/// @brief An enum containing all supported memory types.
		enum MemoryType {
			/// @brief Lazily allocated GPU memory. Equivalent to VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT.
			MEMORY_TYPE_GPU_LAZY,
			/// @brief GPU memory that is not directly visible from the CPU. Equivalent to VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT.
			MEMORY_TYPE_GPU,
			/// @brief GPU memory that is directly visible from the CPU. Equivalent to VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT.
			MEMORY_TYPE_GPU_CPU_VISIBLE,
			/// @brief CPU memory that is directly visible from the GPU. Equivalent to VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT.
			MEMORY_TYPE_CPU_GPU_VISIBLE,
			/// @brief The number of memory types.
			MEMORY_TYPE_COUNT
		};
		/// @brief A scruct containing the current memory block's necessary info.
		struct MemoryBlock {
			/// @brief The offset from the start of the device memory the current block starts at.
			VkDeviceSize offset;
			/// @brief The size of the memory block.
			VkDeviceSize size;
			/// @brief The device memory the current memory block is in.
			VkDeviceMemory memory;
		};

		/// @brief Creates a Vulkan allocator.
		/// @param device The Vulkan device to create the allocator for.
		VulkanAllocator(VulkanDevice* device);
		VulkanAllocator(const VulkanAllocator&) = delete;
		VulkanAllocator(VulkanAllocator&&) noexcept = delete;

		VulkanAllocator& operator=(const VulkanAllocator&) = delete;
		VulkanAllocator& operator=(VulkanAllocator&&) = delete;

		/// @brief Gets the Vulkan function loader used by the allocator.
		/// @return A pointer to the Vulkan loader used by the allocator.
		const VulkanLoader* GetLoader() const {
			return device->GetLoader();
		}
		/// @brief Gets the Vulkan device that owns the allocator.
		/// @return A pointer to the Vulkan device wrapper object.
		VulkanDevice* GetDevice() {
			return device;
		}
		/// @brief Gets the Vulkan device that owns the allocator.
		/// @return A const pointer to the Vulkan device wrapper object.
		const VulkanDevice* GetDevice() const {
			return device;
		}
		/// @brief Gets the Vulkan physical device's memory properties.
		/// @return The struct containing the physical device's memory properties.
		const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const {
			return memoryProperties;
		}

		/// @brief Checks if dedicated memory is supported.
		/// @return True if dedicated memory is supported, otherwise false.
		bool8_t IsDedicatedMemorySupported() const {
			return dedicatedAllocSupported;
		}
		/// @brief Checks if binding multiple memory blocks in one command is supported.
		/// @return True if bind2 is supported.
		bool8_t IsBind2Supported() const {
			return bind2Supported;
		}

		/// @brief Gets the best memory type index in the given bitmask.
		/// @param memoryType The memory type whose index to get.
		/// @param memoryTypeBits A bitmask signifying all possible mmeory type indices.
		/// @return The best memory type index for the given memory type, or UINT32_T_MAX if no valid index exists.
		uint32_t GetMemoryTypeIndex(MemoryType memoryType, uint32_t memoryTypeBits) const;

		/// @brief Allocated a memory block for the given buffer.
		/// @param buffer The buffer to allocate the memory for.
		/// @param memoryType The memory type required for the buffer.
		/// @param memoryBlock A reference to the variable in which the final memory block's info will be written.
		/// @return VK_SUCCESS if the operation was completed successfully, otherwise a corresponding error code.
		VkResult AllocBufferMemory(VkBuffer buffer, MemoryType memoryType, MemoryBlock& memoryBlock);
		/// @brief Allocated a memory block for the given image.
		/// @param buffer The image to allocate the memory for.
		/// @param memoryType The memory type required for the image.
		/// @param memoryBlock A reference to the variable in which the final memory block's info will be written.
		/// @return VK_SUCCESS if the operation was completed successfully, otherwise a corresponding error code.
		VkResult AllocImageMemory(VkImage image, MemoryType memoryType, MemoryBlock& memoryBlock);
		/// @brief Frees the given memory block.
		/// @param memoryBlock The memory block to free.
		void FreeMemory(const MemoryBlock& memoryBlock);

		/// @brief Binds the given buffers with their corresponding memory blocks.
		/// @param bufferCount The number of buffers to bind.
		/// @param buffers A pointer to an array of buffers to bind.
		/// @param memoryBlocks A pointer to an array of memory blocks to bind to.
		/// @return VK_SUCCESS if the operation was completed successfully, otherwise a corresponding error code.
		VkResult BindBufferMemories(size_t bufferCount, VkBuffer* buffers, const MemoryBlock* memoryBlocks) const;
		/// @brief Binds the given images with their corresponding memory blocks.
		/// @param imageCount The number of images to bind.
		/// @param images A pointer to an array of images to bind.
		/// @param memoryBlocks A pointer to an array of memory blocks to bind to.
		/// @return VK_SUCCESS if the operation was completed successfully, otherwise a corresponding error code.
		VkResult BindImageMemories(size_t imageCount, VkImage* images, const MemoryBlock* memoryBlocks) const;

		/// @brief Gets the mapped memory of the given memory block.
		/// @param memoryBlock The memory block to get the mapped memory from.
		/// @return A void pointer to the block's mapped memory, or nullptr if the given memory block isn't mapped.
		void* GetMappedMemory(const MemoryBlock& memoryBlock);

		/// @brief Trims the allocator, freeing all unused resources.
		void Trim();

		/// @brief Destroys the Vulkan allocator.
		~VulkanAllocator();
	private:
		enum ResourceType {
			RESOURCE_TYPE_BUFFER,
			RESOURCE_TYPE_IMAGE,
			RESOURCE_TYPE_COUNT
		};
		struct FreeBlock {
			VkDeviceSize offset;
			VkDeviceSize size;
			size_t prev;
			size_t next;
		};
		struct FreeList {
			size_t first = SIZE_T_MAX;
			size_t last = SIZE_T_MAX;
		};
		struct TypeInfo {
			MemoryType memoryType;
			uint32_t realTypeIndex;
			vector<VkDeviceMemory> memoryBlocks[RESOURCE_TYPE_COUNT];
		};
		struct MemoryBlockInfo {
			FreeList freeList;
			void* mapped;
			uint32_t memoryTypeIndex;

			ResourceType resourceType;
			bool8_t dedicated;
		};
		struct MemoryHash {
			bool8_t operator()(const VkDeviceMemory& memory) {
				return Hash<uint64_t>()((uint64_t)memory);
			}
		};

		size_t InternalAllocFreeListBlock();
		VkResult InternalAllocDeviceMemory(VkDeviceSize size, VkDeviceSize freeSize, uint32_t memoryTypeIndex, ResourceType resourceType, VkBuffer dedicatedBuffer, VkImage dedicatedImage, VkDeviceMemory& memory);
		void InternalFreeDeviceMemory(VkDeviceMemory memory);
		VkResult InternalAllocMemory(const VkMemoryRequirements& memRequirements, uint32_t memoryTypeIndex, VkBuffer buffer, VkImage image, MemoryBlock& memoryBlock);

		VulkanDevice* device;
		VkPhysicalDeviceMemoryProperties memoryProperties;
		bool8_t dedicatedAllocSupported;
		bool8_t bind2Supported;

		vector<TypeInfo> typeInfos;
		unordered_map<VkDeviceMemory, MemoryBlockInfo, MemoryHash> memoryInfos;
		vector<FreeBlock> freeBlocks;
		FreeList freeBlockList;
	};
}