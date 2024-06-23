#include "VulkanAllocator.hpp"
#include "Renderer/Vulkan/VulkanRenderer.hpp"

namespace wfe {
	// Constants
	static const VkDeviceSize MEMORY_BLOCK_SIZES[] {
		0x4000000, // MEMORY_TYPE_GPU_LAZY
		0x4000000, // MEMORY_TYPE_GPU
		0x1000000, // MEMORY_TYPE_GPU_CPU_VISIBLE
		0x1000000  // MEMORY_TYPE_CPU_GPU_VISIBLE
	};
	static const VkMemoryPropertyFlags MEMORY_TYPE_FLAGS[] {
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,                                    // MEMORY_TYPE_GPU_LAZY
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,                                                                              // MEMORY_TYPE_GPU
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // MEMORY_TYPE_GPU_CPU_VISIBLE
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT                                        // MEMORY_TYPE_CPU_GPU_VISIBLE
	};
	static const size_t FREE_BLOCK_START_COUNT = 16;

	// Internal helper functions
	size_t VulkanAllocator::InternalAllocFreeListBlock() {
		// Check if there are no unused free blocks
		if(freeBlockList.first == SIZE_T_MAX) {
			// Save the old free block count
			size_t oldSize = freeBlocks.size();

			// Double the free block count
			freeBlocks.resize(freeBlocks.size() << 1);

			// Set the new values
			freeBlocks[oldSize].prev = SIZE_T_MAX;
			for(size_t i = oldSize; i != freeBlocks.size() - 1; ++i) {
				freeBlocks[i].next = i + 1;
				freeBlocks[i + 1].prev = i;
			}
			freeBlocks[freeBlocks.size() - 1].next = SIZE_T_MAX;

			// Set the free list's new start and end
			freeBlockList.first = oldSize;
			freeBlockList.last = freeBlocks.size() - 1;
		}

		// Save the first free block's index
		size_t freeBlockIndex = freeBlockList.first;
		
		// Remove the block from the free block list
		freeBlockList.first = freeBlocks[freeBlockIndex].next;
		if(freeBlocks[freeBlockIndex].next != SIZE_T_MAX) {
			freeBlocks[freeBlocks[freeBlockIndex].next].prev = SIZE_T_MAX;
		} else {
			freeBlockList.last = SIZE_T_MAX;
		}

		return freeBlockIndex;
	}
	VkResult VulkanAllocator::InternalAllocDeviceMemory(VkDeviceSize size, VkDeviceSize freeSize, uint32_t memoryTypeIndex, ResourceType resourceType, VkBuffer dedicatedBuffer, VkImage dedicatedImage, VkDeviceMemory& memory) {
		// Set the alloc info
		VkMemoryAllocateInfo allocInfo {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
			.pNext = nullptr,
			.allocationSize = size,
			.memoryTypeIndex = typeInfos[memoryTypeIndex].realTypeIndex
		};

		// Set the dedicated alloc info and add it to the pNext chain, if requested
		VkMemoryDedicatedAllocateInfo dedicatedAllocInfo {
			.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
			.pNext = nullptr,
			.image = dedicatedImage,
			.buffer = dedicatedBuffer
		};
		if(dedicatedAllocSupported && (dedicatedBuffer || dedicatedImage))
			allocInfo.pNext = &dedicatedAllocInfo;
		
		// Allocate the memory
		VkResult result = device->GetLoader()->vkAllocateMemory(device->GetDevice(), &allocInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &memory);
		if(result != VK_SUCCESS)
			return result;

		// Map the memory, if accessible by the CPU
		void* mapped;
		if(typeInfos[memoryTypeIndex].memoryType == MEMORY_TYPE_GPU_CPU_VISIBLE || typeInfos[memoryTypeIndex].memoryType == MEMORY_TYPE_CPU_GPU_VISIBLE) {
			result = device->GetLoader()->vkMapMemory(device->GetDevice(), memory, 0, VK_WHOLE_SIZE, 0, &mapped);
			if(result != VK_SUCCESS)
				return result;
		} else {
			mapped = nullptr;
		}
		
		// Set the free block's info, if any free memory is available
		size_t freeBlockIndex;
		if(freeSize) {
			// Allocate the free block
			freeBlockIndex = InternalAllocFreeListBlock();

			// Set its info
			freeBlocks[freeBlockIndex].offset = size - freeSize;
			freeBlocks[freeBlockIndex].size = freeSize;
			freeBlocks[freeBlockIndex].prev = SIZE_T_MAX;
			freeBlocks[freeBlockIndex].next = SIZE_T_MAX;
		} else {
			freeBlockIndex = SIZE_T_MAX;
		}

		// Set the memory block's info
		MemoryBlockInfo memoryInfo {
			.freeList = {
				.first = freeBlockIndex,
				.last = freeBlockIndex
			},
			.mapped = mapped,
			.memoryTypeIndex = memoryTypeIndex,
			.resourceType = resourceType,
			.dedicated = dedicatedBuffer || dedicatedImage
		};

		// Add the memory block to the map
		memoryInfos.insert({ memory, memoryInfo });

		// Add the memory block to the type's info
		typeInfos[memoryTypeIndex].memoryBlocks[resourceType].push_back(memory);

		return VK_SUCCESS;
	}
	void VulkanAllocator::InternalFreeDeviceMemory(VkDeviceMemory memory) {
		// Get the memory block's info and remove it from the map
		MemoryBlockInfo memoryInfo = memoryInfos[memory];
		memoryInfos.erase(memory);

		// Find and remove the memory block from the type's info
		vector<VkDeviceMemory>& typeMemoryVector = typeInfos[memoryInfo.memoryTypeIndex].memoryBlocks[memoryInfo.resourceType];
		for(auto iter = typeMemoryVector.begin(); iter != typeMemoryVector.end(); ++iter) {
			if(*iter == memory) {
				typeMemoryVector.erase(iter);
				break;
			}
		}

		// Add all previous blocks to the free block list
		for(size_t index = memoryInfo.freeList.first; index != SIZE_T_MAX;) {
			// Save the next index
			size_t nextIndex = freeBlocks[index].next;

			// Add the block to the free block list
			freeBlocks[index].prev = SIZE_T_MAX;
			freeBlocks[index].next = freeBlockList.first;

			if(freeBlockList.first != SIZE_T_MAX) {
				freeBlocks[freeBlockList.first].prev = index;
			} else {
				freeBlockList.last = index;
			}
			freeBlockList.first = index;

			// Move on to the next block
			index = nextIndex;
		}

		// Unmap the memory, if accessible by the CPU
		MemoryType memoryType = typeInfos[memoryInfo.memoryTypeIndex].memoryType;
		if(memoryType == MEMORY_TYPE_GPU_CPU_VISIBLE || memoryType == MEMORY_TYPE_CPU_GPU_VISIBLE)
			device->GetLoader()->vkUnmapMemory(device->GetDevice(), memory);

		// Free the memory
		device->GetLoader()->vkFreeMemory(device->GetDevice(), memory, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
	}
	VkResult VulkanAllocator::InternalAllocMemory(const VkMemoryRequirements& memRequirements, uint32_t memoryTypeIndex, VkBuffer buffer, VkImage image, MemoryBlock& memoryBlock) {
		// Get the resurce type based on whether the buffer or the image handles are set
		ResourceType resourceType;
		if(buffer) {
			resourceType = RESOURCE_TYPE_BUFFER;
		} else {
			resourceType = RESOURCE_TYPE_IMAGE;
		}

		// Get a reference to the current type's info
		TypeInfo& typeInfo = typeInfos[memoryTypeIndex];

		// Check if the current memory block is too large for the allocator's blocks
		if(memRequirements.size >= MEMORY_BLOCK_SIZES[typeInfo.memoryType]) {
			// Allocate a dedicated memory block
			VkResult result = InternalAllocDeviceMemory(memRequirements.size, 0, memoryTypeIndex, resourceType, buffer, VK_NULL_HANDLE, memoryBlock.memory);
			if(result != VK_SUCCESS)
				return result;
			
			// Set the memory block's info and exit the function
			memoryBlock.offset = 0;
			memoryBlock.size = memRequirements.size;

			return VK_SUCCESS;
		}

		// Loop through the current type's memory blocks
		for(auto memory : typeInfo.memoryBlocks[resourceType]) {
			// Get the corresponding memory block info
			MemoryBlockInfo& memoryInfo = memoryInfos.at(memory);

			// Loop through the current memory block's free blocks
			for(size_t index = memoryInfo.freeList.first; index != SIZE_T_MAX; index = freeBlocks[index].next) {
				// Get the required alignment for the requested resource
				VkDeviceSize alignedOffset = (freeBlocks[index].offset + memRequirements.alignment - 1) & ~(memRequirements.alignment - 1);
				VkDeviceSize alignment = alignedOffset - freeBlocks[index].offset;

				// Check if the current free block has enough room for the resource and its alignment
				if(freeBlocks[index].size < alignment + memRequirements.size)
					continue;
				
				// Set the memory block's info
				memoryBlock.memory = memory;
				memoryBlock.offset = alignedOffset;
				memoryBlock.size = memRequirements.size;
				
				// Calculate the remaining size for the free block after the allocated resource
				VkDeviceSize leftoverFreeSize = freeBlocks[index].size - alignment - memRequirements.size;

				// Rearrange the free block for the current case
				if(alignment && leftoverFreeSize) {
					// Allocate a brand new free list block
					size_t newIndex = InternalAllocFreeListBlock();

					// Insert the free block right after the current free block
					size_t nextIndex = freeBlocks[index].next;

					freeBlocks[newIndex].prev = index;
					freeBlocks[newIndex].next = nextIndex;

					freeBlocks[index].next = newIndex;
					if(nextIndex != SIZE_T_MAX) {
						freeBlocks[nextIndex].prev = newIndex;
					} else {
						memoryInfo.freeList.last = newIndex;
					}

					// Set the two free blocks' infos
					freeBlocks[index].size = alignment;
					freeBlocks[newIndex].offset = alignedOffset + memRequirements.size;
					freeBlocks[newIndex].size = leftoverFreeSize;
				} else if(alignment) {
					// Set the free block's new info
					freeBlocks[index].size = alignment;
				} else if(leftoverFreeSize) {
					// Set the free block's new info
					freeBlocks[index].offset += memRequirements.size;
					freeBlocks[index].size = leftoverFreeSize;
				} else {
					// Remove the free block from its free list
					if(freeBlocks[index].prev != SIZE_T_MAX) {
						freeBlocks[freeBlocks[index].prev].next = freeBlocks[index].next;
					} else {
						memoryInfo.freeList.first = freeBlocks[index].next;
					}
					if(freeBlocks[index].next != SIZE_T_MAX) {
						freeBlocks[freeBlocks[index].next].prev = freeBlocks[index].prev;
					} else {
						memoryInfo.freeList.last = freeBlocks[index].prev;
					}
					
					// Add the free block to the free block list
					freeBlocks[index].prev = SIZE_T_MAX;
					freeBlocks[index].next = freeBlockList.first;

					if(freeBlockList.first != SIZE_T_MAX) {
						freeBlocks[freeBlockList.first].prev = index;
					} else {
						freeBlockList.last = index;
					}
					freeBlockList.first = index;
				}

				// Exit the function
				return VK_SUCCESS;
			}
		}

		// No suitable memory was found; allocate a new memory block
		VkResult result = InternalAllocDeviceMemory(MEMORY_BLOCK_SIZES[typeInfo.memoryType], MEMORY_BLOCK_SIZES[typeInfo.memoryType] - memRequirements.size, memoryTypeIndex, resourceType, VK_NULL_HANDLE, VK_NULL_HANDLE, memoryBlock.memory);
		if(result != VK_SUCCESS)
			return result;
		
		// Set the memory block's remaining info
		memoryBlock.offset = 0;
		memoryBlock.size = memRequirements.size;

		return VK_SUCCESS;
	}

	// Public functions
	VulkanAllocator::VulkanAllocator(VulkanDevice* device) : device(device), freeBlocks(FREE_BLOCK_START_COUNT) {
		// Get the device's memory properties 
		device->GetLoader()->vkGetPhysicalDeviceMemoryProperties(device->GetPhysicalDevice(), &memoryProperties);

		// Check for dedicated alloc and bind2 support
		dedicatedAllocSupported = device->GetDeviceProperties().apiVersion >= VK_API_VERSION_1_1 || (device->GetEnabledExtensions().count(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME) && device->GetEnabledExtensions().count(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME));
		bind2Supported = device->GetDeviceProperties().apiVersion >= VK_API_VERSION_1_1 || device->GetEnabledExtensions().count(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);

		// Assign a memory type to every device memory type
		for(uint32_t i = 0; i != memoryProperties.memoryTypeCount; ++i) {
			// Loop through the memory types and set the first type which has all flags satisfied
			MemoryType memoryType = MEMORY_TYPE_COUNT;
			for(uint32_t j = 0; j != (uint32_t)MEMORY_TYPE_COUNT; ++j) {
				if(MEMORY_TYPE_FLAGS[j] & memoryProperties.memoryTypes[i].propertyFlags == MEMORY_TYPE_FLAGS[j]) {
					memoryType = (MemoryType)j;
					break;
				}
			}

			// Skip the current type if no memory type was satisfied
			if(memoryType == MEMORY_TYPE_COUNT)
				continue;
			
			// Add the new memory type to the vector
			TypeInfo typeInfo {
				.memoryType = memoryType,
				.realTypeIndex = i
			};
			typeInfos.push_back(typeInfo);
		}

		// Initialize the free block free list
		freeBlockList.first = 0;
		freeBlockList.last = freeBlocks.size() - 1;

		freeBlocks[0].prev = SIZE_T_MAX;
		for(size_t i = 0; i != freeBlocks.size() - 1; ++i) {
			freeBlocks[i].next = i + 1;
			freeBlocks[i + 1].prev = i;
		}
		freeBlocks[freeBlocks.size() - 1].next = SIZE_T_MAX;
	}

	uint32_t VulkanAllocator::GetMemoryTypeIndex(MemoryType memoryType, uint32_t memoryTypeBits) const {
		// Consider all memory types with all required functionality
		for(; memoryType != MEMORY_TYPE_COUNT; memoryType = (MemoryType)((uint32_t)memoryType + 1)) {
			// Loop through all memory types
			for(uint32_t i = 0; i != (uint32_t)typeInfos.size(); ++i) {
				// Check if the current memory type's real index is in the given mask
				if(!((1 << typeInfos[i].realTypeIndex) & memoryTypeBits))
					continue;
				
				// Return the current type intex if the requested type is supported
				if(typeInfos[i].memoryType == memoryType)
					return i;
			}
		}

		return UINT32_T_MAX;
	}

	VkResult VulkanAllocator::AllocBufferMemory(VkBuffer buffer, MemoryType memoryType, MemoryBlock& memoryBlock) {
		// Get the buffer's memory requirements
		VkMemoryRequirements memRequirements;
		if(dedicatedAllocSupported) {
			// Set the memory requirements struct with the dedicated alloc requirements in the pNext chain
			VkMemoryDedicatedRequirementsKHR dedicatedMemoryRequirements {
				.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS_KHR,
				.pNext = nullptr
			};

			VkMemoryRequirements2KHR memoryRequirements {
				.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR,
				.pNext = &dedicatedMemoryRequirements
			};

			// Get the buffer's memory requirements
			VkBufferMemoryRequirementsInfo2KHR memoryRequirementsInfo {
				.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR,
				.pNext = nullptr,
				.buffer = buffer
			};

			device->GetLoader()->vkGetBufferMemoryRequirements2KHR(device->GetDevice(), &memoryRequirementsInfo, &memoryRequirements);

			// Proceed if the buffer requires or prefers a dedicated allocation
			if(dedicatedMemoryRequirements.requiresDedicatedAllocation || dedicatedMemoryRequirements.prefersDedicatedAllocation) {
				// Get the buffer's memory type index
				uint32_t memoryTypeIndex = GetMemoryTypeIndex(memoryType, memoryRequirements.memoryRequirements.memoryTypeBits);
				if(memoryTypeIndex == UINT32_T_MAX)
					return VK_ERROR_FEATURE_NOT_PRESENT;
				
				// Set the dedicated alloc info
				VkMemoryDedicatedAllocateInfoKHR dedicatedAllocInfo {
					.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR,
					.pNext = nullptr,
					.image = VK_NULL_HANDLE,
					.buffer = buffer
				};

				// Set the memory alloc info
				VkMemoryAllocateInfo allocInfo {
					.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
					.pNext = &dedicatedAllocInfo,
					.allocationSize = memoryRequirements.memoryRequirements.size,
					.memoryTypeIndex = typeInfos[memoryTypeIndex].realTypeIndex
				};

				// Allocate the buffer's memory
				VkResult result = InternalAllocDeviceMemory(memoryRequirements.memoryRequirements.size, 0, memoryTypeIndex, RESOURCE_TYPE_BUFFER, buffer, VK_NULL_HANDLE, memoryBlock.memory);
				if(result != VK_SUCCESS)
					return result;
				
				// Set the memory block's remaining info and exit the function
				memoryBlock.offset = 0;
				memoryBlock.size = memoryRequirements.memoryRequirements.size;

				return VK_SUCCESS;
			} else {
				// Save the retrieved memory requirements
				memRequirements = memoryRequirements.memoryRequirements;
			}
		} else {
			// Get the buffer's memory requirements
			device->GetLoader()->vkGetBufferMemoryRequirements(device->GetDevice(), buffer, &memRequirements);
		}

		// Get the buffer's memory type index
		uint32_t memoryTypeIndex = GetMemoryTypeIndex(memoryType, memRequirements.memoryTypeBits);
		if(memoryTypeIndex == UINT32_T_MAX)
			return VK_ERROR_FEATURE_NOT_PRESENT;

		// Allocate the buffer's memory
		return InternalAllocMemory(memRequirements, memoryTypeIndex, buffer, VK_NULL_HANDLE, memoryBlock);
	}
	VkResult VulkanAllocator::AllocImageMemory(VkImage image, MemoryType memoryType, MemoryBlock& memoryBlock) {
		// Get the image's memory requirements
		VkMemoryRequirements memRequirements;
		if(dedicatedAllocSupported) {
			// Set the memory requirements struct with the dedicated alloc requirements in the pNext chain
			VkMemoryDedicatedRequirementsKHR dedicatedMemoryRequirements {
				.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS_KHR,
				.pNext = nullptr
			};

			VkMemoryRequirements2KHR memoryRequirements {
				.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR,
				.pNext = &dedicatedMemoryRequirements
			};

			// Get the image's memory requirements
			VkImageMemoryRequirementsInfo2KHR memoryRequirementsInfo {
				.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR,
				.pNext = nullptr,
				.image = image
			};

			device->GetLoader()->vkGetImageMemoryRequirements2KHR(device->GetDevice(), &memoryRequirementsInfo, &memoryRequirements);

			// Proceed if the image requires or prefers a dedicated allocation
			if(dedicatedMemoryRequirements.requiresDedicatedAllocation || dedicatedMemoryRequirements.prefersDedicatedAllocation) {
				// Get the image's memory type index
				uint32_t memoryTypeIndex = GetMemoryTypeIndex(memoryType, memoryRequirements.memoryRequirements.memoryTypeBits);
				if(memoryTypeIndex == UINT32_T_MAX)
					return VK_ERROR_FEATURE_NOT_PRESENT;
				
				// Set the dedicated alloc info
				VkMemoryDedicatedAllocateInfoKHR dedicatedAllocInfo {
					.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR,
					.pNext = nullptr,
					.image = image,
					.buffer = VK_NULL_HANDLE
				};

				// Set the memory alloc info
				VkMemoryAllocateInfo allocInfo {
					.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
					.pNext = &dedicatedAllocInfo,
					.allocationSize = memoryRequirements.memoryRequirements.size,
					.memoryTypeIndex = typeInfos[memoryTypeIndex].realTypeIndex
				};

				// Allocate the image's memory
				VkResult result = InternalAllocDeviceMemory(memoryRequirements.memoryRequirements.size, 0, memoryTypeIndex, RESOURCE_TYPE_BUFFER, VK_NULL_HANDLE, image, memoryBlock.memory);
				if(result != VK_SUCCESS)
					return result;
				
				// Set the memory block's remaining info and exit the function
				memoryBlock.offset = 0;
				memoryBlock.size = memoryRequirements.memoryRequirements.size;

				return VK_SUCCESS;
			} else {
				// Save the retrieved memory requirements
				memRequirements = memoryRequirements.memoryRequirements;
			}
		} else {
			// Get the image's memory requirements
			device->GetLoader()->vkGetImageMemoryRequirements(device->GetDevice(), image, &memRequirements);
		}

		// Get the image's memory type index
		uint32_t memoryTypeIndex = GetMemoryTypeIndex(memoryType, memRequirements.memoryTypeBits);
		if(memoryTypeIndex == UINT32_T_MAX)
			return VK_ERROR_FEATURE_NOT_PRESENT;

		// Allocate the image's memory
		return InternalAllocMemory(memRequirements, memoryTypeIndex, VK_NULL_HANDLE, image, memoryBlock);
	}
	void VulkanAllocator::FreeMemory(const MemoryBlock& memoryBlock) {
		// Get the memory block's info
		MemoryBlockInfo& memoryInfo = memoryInfos[memoryBlock.memory];

		// Check if the memory block is a dedicated allocation
		if(memoryInfo.dedicated) {
			// Simply free the memory block and exit the function
			InternalFreeDeviceMemory(memoryBlock.memory);
			return;
		}

		// Find the free blocks before and after the given memory block
		size_t prevIndex = SIZE_T_MAX;
		size_t nextIndex = memoryInfo.freeList.first;
		for(; nextIndex != SIZE_T_MAX && freeBlocks[nextIndex].offset < memoryBlock.offset; nextIndex = freeBlocks[nextIndex].next) {
			// Set the new previous index
			prevIndex = nextIndex;
		}

		// Check if any of those blocks are adjacent to the freed block
		bool8_t prevAdjacent = prevIndex != SIZE_T_MAX && freeBlocks[prevIndex].offset + freeBlocks[prevIndex].size == memoryBlock.offset;
		bool8_t nextAdjacent = nextIndex != SIZE_T_MAX && memoryBlock.offset + memoryBlock.size == freeBlocks[nextIndex].offset;

		if(prevAdjacent && nextAdjacent) {
			// Set the new free block's info
			freeBlocks[prevIndex].size += memoryBlock.size + freeBlocks[nextIndex].size;

			// Remove the next free block from the free list
			freeBlocks[prevIndex].next = freeBlocks[nextIndex].next;
			if(freeBlocks[nextIndex].next != SIZE_T_MAX) {
				freeBlocks[freeBlocks[nextIndex].next].prev = prevIndex;
			} else {
				memoryInfo.freeList.last = prevIndex;
			}

			// Add the free block to the free block list
			freeBlocks[nextIndex].prev = SIZE_T_MAX;
			freeBlocks[nextIndex].next = freeBlockList.first;

			if(freeBlockList.first != SIZE_T_MAX) {
				freeBlocks[freeBlockList.first].prev = nextIndex;
			} else {
				freeBlockList.last = nextIndex;
			}
			freeBlockList.first = nextIndex;
		} else if(prevAdjacent) {
			// Set the new free block's info
			freeBlocks[prevIndex].size += memoryBlock.size;
		} else if(nextAdjacent) {
			// Set the new free block's info
			freeBlocks[nextIndex].offset = memoryBlock.offset;
			freeBlocks[nextIndex].size += memoryBlock.size;
		} else {
			// Aloocate a new free block
			size_t newIndex = InternalAllocFreeListBlock();

			// Set the new free block's info
			freeBlocks[newIndex].offset = memoryBlock.offset;
			freeBlocks[newIndex].size = memoryBlock.size;

			// Insert the new free block in the free list
			freeBlocks[newIndex].prev = prevIndex;
			freeBlocks[newIndex].next = nextIndex;

			if(prevIndex != SIZE_T_MAX) {
				freeBlocks[prevIndex].next = newIndex;
			} else {
				memoryInfo.freeList.first = newIndex;
			}
			if(nextIndex != SIZE_T_MAX) {
				freeBlocks[nextIndex].prev = newIndex;
			} else {
				memoryInfo.freeList.last = newIndex;
			}
		}
	}

	VkResult VulkanAllocator::BindBufferMemories(size_t bufferCount, VkBuffer* buffers, const MemoryBlock* memoryBlocks) const {
		// Check if bind2 is supported
		if(bind2Supported) {
			// Allocate a bind info array
			PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
			VkBindBufferMemoryInfoKHR* bindInfos = (VkBindBufferMemoryInfoKHR*)AllocMemory(bufferCount * sizeof(VkBindBufferMemoryInfoKHR));
			PopMemoryUsageType();
			if(!bindInfos)
				return VK_ERROR_OUT_OF_HOST_MEMORY;
		
			// Set the bind infos
			for(size_t i = 0; i != bufferCount; ++i) {
				bindInfos[i].sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO_KHR;
				bindInfos[i].pNext = nullptr;
				bindInfos[i].buffer = buffers[i];
				bindInfos[i].memory = memoryBlocks[i].memory;
				bindInfos[i].memoryOffset = memoryBlocks[i].offset;
			}

			// Bind the buffer memories
			VkResult result = device->GetLoader()->vkBindBufferMemory2KHR(device->GetDevice(), (uint32_t)bufferCount, bindInfos);

			// Free the bind infos array and exit the function
			wfe::FreeMemory(bindInfos);

			return result;
		} else {
			// Bind every buffer with its corresponding memory block individually
			for(size_t i = 0; i != bufferCount; ++i) {
				VkResult result = device->GetLoader()->vkBindBufferMemory(device->GetDevice(), buffers[i], memoryBlocks[i].memory, memoryBlocks[i].offset);
				if(result != VK_SUCCESS)
					return result;
			}

			return VK_SUCCESS;
		}
	}
	VkResult VulkanAllocator::BindImageMemories(size_t imageCount, VkImage* images, const MemoryBlock* memoryBlocks) const {
		// Check if bind2 is supported
		if(bind2Supported) {
			// Allocate a bind info array
			PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
			VkBindImageMemoryInfoKHR* bindInfos = (VkBindImageMemoryInfoKHR*)AllocMemory(imageCount * sizeof(VkBindImageMemoryInfoKHR));
			PopMemoryUsageType();
			if(!bindInfos)
				return VK_ERROR_OUT_OF_HOST_MEMORY;
		
			// Set the bind infos
			for(size_t i = 0; i != imageCount; ++i) {
				bindInfos[i].sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO_KHR;
				bindInfos[i].pNext = nullptr;
				bindInfos[i].image = images[i];
				bindInfos[i].memory = memoryBlocks[i].memory;
				bindInfos[i].memoryOffset = memoryBlocks[i].offset;
			}

			// Bind the image memories
			VkResult result = device->GetLoader()->vkBindImageMemory2KHR(device->GetDevice(), (uint32_t)imageCount, bindInfos);

			// Free the bind infos array and exit the function
			wfe::FreeMemory(bindInfos);

			return result;
		} else {
			// Bind every image with its corresponding memory block individually
			for(size_t i = 0; i != imageCount; ++i) {
				VkResult result = device->GetLoader()->vkBindImageMemory(device->GetDevice(), images[i], memoryBlocks[i].memory, memoryBlocks[i].offset);
				if(result != VK_SUCCESS)
					return result;
			}

			return VK_SUCCESS;
		}
	}

	void* VulkanAllocator::GetMappedMemory(const MemoryBlock& memoryBlock) {
		// Get the parent device memory's mapped memory
		void* mapped = memoryInfos[memoryBlock.memory].mapped;

		// Exit the function if the parent device memory isn't mapped
		if(!mapped)
			return nullptr;
		
		// Offset the mapped memory by the memory block's offset
		return (char_t*)mapped + memoryBlock.offset;
	}

	void VulkanAllocator::Trim() {
		// Loop through all memory blocks and find the empty ones
		vector<VkDeviceMemory> emptyMemories;

		for(auto memory : memoryInfos) {
			// Check if the current memory block is empty (has a free block that spans the entire memory block)
			if(memory.second.freeList.first != SIZE_T_MAX && freeBlocks[memory.second.freeList.first].size == MEMORY_BLOCK_SIZES[typeInfos[memory.second.memoryTypeIndex].memoryType]) {
				// Add the current memory block to the empty block vector
				emptyMemories.push_back(memory.first);
			}
		}

		// Free the empty memory blocks
		for(VkDeviceMemory memory : emptyMemories)
			InternalFreeDeviceMemory(memory);
	}

	VulkanAllocator::~VulkanAllocator() {
		// Free every single memory block
		for(auto memory : memoryInfos) {
			// Free the current memory block
			device->GetLoader()->vkFreeMemory(device->GetDevice(), memory.first, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
		}
	}
}