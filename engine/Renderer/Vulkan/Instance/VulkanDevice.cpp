#include "VulkanDevice.hpp"
#include "Renderer/Vulkan/VulkanRenderer.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Core/GPUCommandBuffer.hpp"

#if defined(WFE_PLATFORM_WINDOWS)
#include <vulkan/vulkan_win32.h>
#elif defined(WFE_PLATFORM_LINUX)
#include <vulkan/vulkan_xcb.h>
#endif
#include <vulkan/vk_enum_string_helper.h>

namespace wfe {
	// Constants
	static const uint32_t DISCRETE_GPU_SCORE = 128;
	static const uint32_t OPTIONAL_EXTENSION_SCORE_INCREASE = 8;
	static const uint32_t GRAPHICS_QUEUE_FAMILY_SCORE_INCREASE = 8;
	static const uint32_t PRESENT_QUEUE_FAMILY_SCORE_INCREASE = 8;
	static const uint32_t TRANSFER_QUEUE_FAMILY_SCORE_INCREASE = 8;
	static const uint32_t COMPUTE_QUEUE_FAMILY_SCORE_INCREASE = 8;

	static const set<const char_t*> DEFAULT_REQUIRED_DEVICE_EXTENSIONS {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	static const set<const char_t*> DEFAULT_OPTIONAL_DEVICE_EXTENSIONS {
		VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
		VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
		VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
		VK_KHR_BIND_MEMORY_2_EXTENSION_NAME
	};

	// Internal helper functions
	static size_t InternalCheckForExtensionSupport(const set<const char_t*>& extensionNames, bool8_t* supported, uint32_t supportedCount, VkExtensionProperties* supportedExtensions) {
		// Set every value in the supported array to false, if it exists
		if(supported)
			memset(supported, 0, extensionNames.size() * sizeof(bool8_t));

		// Loop through every supported extension and check if all extensions in the set are in the supported array
		size_t supportedExtensionCount = 0;
		VkExtensionProperties* supportedExtensionsEnd = supportedExtensions + supportedCount;

		for(VkExtensionProperties* extension = supportedExtensions; extension != supportedExtensionsEnd; ++extension) {
			// Check if the current extension is in the given set
			const char_t* const* currentExtension = extensionNames.find(extension->extensionName);
			if(currentExtension != extensionNames.end()) {
				// Set the corresponding bool, if a supported array was given, and increment the supported extension count
				if(supported)
					supported[currentExtension - extensionNames.begin()] = true;
				++supportedExtensionCount;
			}
		}

		// Return the number of supported extensions
		return supportedExtensionCount;
	}
	static VulkanDevice::QueueFamilyIndices InternalFindPhysicalDeviceQueueFamilyIndices(const VulkanLoader* loader, VkPhysicalDevice physicalDevice, VulkanSurface* surface, uint32_t queueFamilyCount, VkQueueFamilyProperties* queueFamilies) {
		// Find the best graphics and present queue family index
		VulkanDevice::QueueFamilyIndices queueFamilyIndices;

		for(uint32_t i = 0; i != queueFamilyCount; ++i) {
			// Check if the current queue supports graphics
			bool8_t graphicsSupport = queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;

			// Check if the current queue supports presenting
			VkBool32 presentSupport = VK_FALSE;
			if(surface) {
				loader->vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface->GetSurface(), &presentSupport);
#if defined(WFE_PLATFORM_WINDOWS)
				presentSupport = presentSupport && loader->vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, i);
#endif
			}

			// Exit the loop if both graphics and presenting are supported, as this is the best possible scenario
			if(graphicsSupport && presentSupport && queueFamilies[i].queueCount != 1) {
				queueFamilyIndices.graphicsIndex = i;
				queueFamilyIndices.presentIndex = i;
				break;
			}

			// Set the individual queue family index
			if(graphicsSupport && queueFamilyIndices.graphicsIndex == UINT32_T_MAX)
				queueFamilyIndices.graphicsIndex = i;
			else if(presentSupport)
				queueFamilyIndices.presentIndex = i;
		}

		// Find the best transfer queue family index
		uint32_t maxScore = 0;
		for(uint32_t i = 0; i != queueFamilyCount; ++i) {
			// Move on to the next queue family if it doesn't support transfer
			if(!(queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
				continue;
			
			// Calculate the current device's score
			uint32_t score = 1 + (((i != queueFamilyIndices.graphicsIndex) + (i != queueFamilyIndices.presentIndex)) << 1);
			score += (i == queueFamilyIndices.graphicsIndex && queueFamilies[i].queueCount != 1);
			score += (i == queueFamilyIndices.presentIndex && queueFamilies[i].queueCount != 1);

			// Check if the current score is the maximum score
			if(score > maxScore) {
				maxScore = score;
				queueFamilyIndices.transferIndex = i;
			}
		}

		// Find the best comput equeue family index
		maxScore = 0;
		for(uint32_t i = 0; i != queueFamilyCount; ++i) {
			// Move on to the next queue family if it doesn't support compute pipelines
			if(!(queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
				continue;

			// Calculate the current device's score
			uint32_t score = 1 + ((i != queueFamilyIndices.graphicsIndex) + (i != queueFamilyIndices.presentIndex) + (i != queueFamilyIndices.transferIndex)) * 3;
			score += (i == queueFamilyIndices.graphicsIndex && queueFamilies[i].queueCount != 1);
			score += (i == queueFamilyIndices.presentIndex && queueFamilies[i].queueCount != 1);
			score += (i == queueFamilyIndices.transferIndex && queueFamilies[i].queueCount != 1);

			// Check if the current score is the maximum score
			if(score > maxScore) {
				maxScore = score;
				queueFamilyIndices.computeIndex = i;
			}
		}

		return queueFamilyIndices;
	}
	static void AddQueueCreateInfo(uint32_t index, uint32_t& queueInfoCount, VkDeviceQueueCreateInfo* queueInfos, const float32_t* queuePriorities, VkQueueFamilyProperties* queueFamilies) {
		// Check if the given index is not valid
		if(index == UINT32_T_MAX)
			return;

		// Check if the current queue family is already in use
		bool8_t exists = false;
		for(uint32_t i = 0; i != queueInfoCount; ++i)
			if(queueInfos[i].queueFamilyIndex == index) {
				exists = true;

				// Check if there is still room for another queue in the current family
				if(queueFamilies[index].queueCount) {
					++queueInfos[i].queueCount;
					--queueFamilies[index].queueCount;
				}

				break;
			}
		
		// Set a new queue create info is the current one is not in use
		if(!exists) {
			queueInfos[queueInfoCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfos[queueInfoCount].pNext = nullptr;
			queueInfos[queueInfoCount].flags = 0;
			queueInfos[queueInfoCount].queueFamilyIndex = index;
			queueInfos[queueInfoCount].queueCount = 1;
			queueInfos[queueInfoCount].pQueuePriorities = queuePriorities;

			++queueInfoCount;
			--queueFamilies[index].queueCount;
		}
	}

	void VulkanDevice::GetPhysicalDeviceInfo(const set<const char_t*>& requiredExtensions, const set<const char_t*>& optionalExtensions) {
		// Get the physical device's properties and features
		loader->vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		loader->vkGetPhysicalDeviceFeatures(physicalDevice, &features);

		// Get the number of supported extensions
		uint32_t supportedCount;
		loader->vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &supportedCount, nullptr);

		// Allocate all required arrays
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkExtensionProperties* supportedExtensions = (VkExtensionProperties*)AllocMemory(supportedCount * sizeof(VkExtensionProperties) + optionalExtensions.size() * sizeof(bool8_t));
		PopMemoryUsageType();
		if(!supportedExtensions)
			throw BadAllocException("Failed to allocate Vulkan supported device extensions array!");
		
		bool8_t* supportedOptionalExtensions = (bool8_t*)(supportedExtensions + supportedCount);
		
		// Get the supported extensions
		loader->vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &supportedCount, supportedExtensions);

		// Add all required extensions to the extension set
		for(const char_t* extension : requiredExtensions)
			extensions.insert(extension);

		// Use the internal function to check for optional extension support
		InternalCheckForExtensionSupport(optionalExtensions, supportedOptionalExtensions, supportedCount, supportedExtensions);

		// Add all supported optional extensions to the extension set
		for(size_t i = 0; i != optionalExtensions.size(); ++i)
			if(supportedOptionalExtensions[i])
				extensions.insert(*(optionalExtensions.begin() + i));

		// Free the allocated arrays
		FreeMemory(supportedExtensions);
	}
	void VulkanDevice::CreateDevice(VulkanSurface* surface, bool8_t setQueueFamilyIndices) {
		// Get the number of queue families
		uint32_t queueFamilyCount;
		loader->vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		// Get the queue families
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)AllocMemory(queueFamilyCount * sizeof(VkQueueFamilyProperties));
		PopMemoryUsageType();
		if(!queueFamilies)
			throw BadAllocException("Failed to allocate Vulkan physical device queue families array!");
		
		loader->vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);
		
		if(setQueueFamilyIndices) {
			// Use the internal function to find the queue family indices
			indices = InternalFindPhysicalDeviceQueueFamilyIndices(loader, physicalDevice, surface, queueFamilyCount, queueFamilies);
		}

		// Set the queue create infos
		uint32_t queueInfoCount = 0;
		VkDeviceQueueCreateInfo queueInfos[4];
		float32_t queuePriorities[4] { 1.f, 1.f, 1.f, 1.f };

		AddQueueCreateInfo(indices.graphicsIndex, queueInfoCount, queueInfos, queuePriorities, queueFamilies);
		AddQueueCreateInfo(indices.presentIndex, queueInfoCount, queueInfos, queuePriorities, queueFamilies);
		AddQueueCreateInfo(indices.transferIndex, queueInfoCount, queueInfos, queuePriorities, queueFamilies);
		AddQueueCreateInfo(indices.computeIndex, queueInfoCount, queueInfos, queuePriorities, queueFamilies);

		// Set the device's create info
		VkDeviceCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.queueCreateInfoCount = queueInfoCount,
			.pQueueCreateInfos = queueInfos,
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = nullptr,
			.enabledExtensionCount = (uint32_t)extensions.size(),
			.ppEnabledExtensionNames = extensions.begin(),
			.pEnabledFeatures = &features
		};

		// Create the device
		VkResult result = loader->vkCreateDevice(physicalDevice, &createInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &device);
		if(result != VK_SUCCESS)
			throw Exception("Failed to create Vulkan logical device! Error code: %s", string_VkResult(result));

		// Reset the queue counts
		if(indices.graphicsIndex != UINT32_T_MAX)
			queueFamilies[indices.graphicsIndex].queueCount = 0;
		if(indices.presentIndex != UINT32_T_MAX)
			queueFamilies[indices.presentIndex].queueCount = 0;
		if(indices.transferIndex != UINT32_T_MAX)
			queueFamilies[indices.transferIndex].queueCount = 0;
		if(indices.computeIndex != UINT32_T_MAX)
			queueFamilies[indices.computeIndex].queueCount = 0;
		
		// Get the device queues
		if(indices.graphicsIndex != UINT32_T_MAX)
			loader->vkGetDeviceQueue(device, indices.graphicsIndex, queueFamilies[indices.graphicsIndex].queueCount++, &graphicsQueue);
		if(indices.presentIndex != UINT32_T_MAX)
			loader->vkGetDeviceQueue(device, indices.presentIndex, queueFamilies[indices.presentIndex].queueCount++, &presentQueue);
		if(indices.transferIndex != UINT32_T_MAX)
			loader->vkGetDeviceQueue(device, indices.transferIndex, queueFamilies[indices.transferIndex].queueCount++, &transferQueue);
		if(indices.computeIndex != UINT32_T_MAX)
			loader->vkGetDeviceQueue(device, indices.computeIndex, queueFamilies[indices.computeIndex].queueCount++, &computeQueue);

		// Free the queue families array
		FreeMemory(queueFamilies);
	}

	// Public functions
	const set<const char_t*>& VulkanDevice::GetDefaultRequiredDeviceExtensions() {
		return DEFAULT_REQUIRED_DEVICE_EXTENSIONS;
	}
	const set<const char_t*>& VulkanDevice::GetDefaultOptionalDeviceExtensions() {
		return DEFAULT_OPTIONAL_DEVICE_EXTENSIONS;
	}

	VulkanDevice::QueueFamilyIndices VulkanDevice::FindPhysicalDeviceQueueFamilyIndices(const VulkanLoader* loader, VkPhysicalDevice physicalDevice, VulkanSurface* surface) {
		// Get the number of queue families
		uint32_t queueFamilyCount;
		loader->vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		// Get the queue families
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)AllocMemory(queueFamilyCount * sizeof(VkQueueFamilyProperties));
		PopMemoryUsageType();
		if(!queueFamilies)
			throw BadAllocException("Failed to allocate Vulkan physical device queue families array!");
		
		loader->vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);
		
		// Use the internal function to find the queue family indices
		QueueFamilyIndices indices = InternalFindPhysicalDeviceQueueFamilyIndices(loader, physicalDevice, surface, queueFamilyCount, queueFamilies);

		// Free the queue families array
		FreeMemory(queueFamilies);

		return indices;
	}
	size_t VulkanDevice::CheckForExtensionSupport(const VulkanLoader* loader, VkPhysicalDevice physicalDevice, const set<const char_t*>& extensionNames, bool8_t* supported) {
		// Get the number of supported extensions
		uint32_t supportedCount;
		loader->vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &supportedCount, nullptr);

		// Get the supported extensions
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkExtensionProperties* supportedExtensions = (VkExtensionProperties*)AllocMemory(supportedCount * sizeof(VkExtensionProperties));
		PopMemoryUsageType();
		if(!supportedExtensions)
			throw BadAllocException("Failed to allocate Vulkan supported device extensions array!");
		
		loader->vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &supportedCount, supportedExtensions);

		// Use the internal function to check for extension support
		size_t extensionCount = InternalCheckForExtensionSupport(extensionNames, supported, supportedCount, supportedExtensions);

		// Free the extension array
		FreeMemory(supportedExtensions);

		return extensionCount;
	}
	uint32_t VulkanDevice::GetPhysicalDeviceScore(const VulkanLoader* loader, VkPhysicalDevice physicalDevice, VulkanSurface* surface, const set<const char_t*>& requiredExtensions, const set<const char_t*>& optionalExtensions) {
		// Start with an innitial score of 1
		uint32_t score = 1;

		// Get the physical device's properties
		VkPhysicalDeviceProperties deviceProperties;
		loader->vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

		// Increase the score if the device is discrete
		score += (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) * DISCRETE_GPU_SCORE;

		// Get the number of supported extensions
		uint32_t supportedExtensionCount;
		loader->vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &supportedExtensionCount, nullptr);

		// Get the supported extensions
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkExtensionProperties* supportedExtensions = (VkExtensionProperties*)AllocMemory(supportedExtensionCount * sizeof(VkExtensionProperties));
		PopMemoryUsageType();
		if(!supportedExtensions)
			throw BadAllocException("Failed to allocate Vulkan supported device extensions array!");

		loader->vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &supportedExtensionCount, supportedExtensions);

		// Check if all required extensions are supported
		if(InternalCheckForExtensionSupport(requiredExtensions, nullptr, supportedExtensionCount, supportedExtensions) != requiredExtensions.size()) {
			// Free the extension array and exit the function
			FreeMemory(supportedExtensions);
			return 0;
		}

		// Increase the score for all supported optional extensions
		score += (uint32_t)InternalCheckForExtensionSupport(optionalExtensions, nullptr, supportedExtensionCount, supportedExtensions) * OPTIONAL_EXTENSION_SCORE_INCREASE;

		// Free the extension array
		FreeMemory(supportedExtensions);

		// Get the device's queue family indices
		QueueFamilyIndices indices = FindPhysicalDeviceQueueFamilyIndices(loader, physicalDevice, surface);

		// Check if presenting is required, but not supported
		if(surface && (indices.graphicsIndex == UINT32_T_MAX || indices.presentIndex == UINT32_T_MAX))
			return 0;
		
		// Increase the score based on the queue family indices
		score += (indices.graphicsIndex != UINT32_T_MAX) * GRAPHICS_QUEUE_FAMILY_SCORE_INCREASE;
		score += (indices.presentIndex != UINT32_T_MAX) * COMPUTE_QUEUE_FAMILY_SCORE_INCREASE;
		score += (indices.transferIndex != UINT32_T_MAX) * TRANSFER_QUEUE_FAMILY_SCORE_INCREASE;
		score += (indices.computeIndex != UINT32_T_MAX) * COMPUTE_QUEUE_FAMILY_SCORE_INCREASE;

		return score;
	}
	VkPhysicalDevice VulkanDevice::FindBestPhysicalDevice(VulkanInstance* instance, VulkanSurface* surface, const set<const char_t*>& requiredExtensions, const set<const char_t*>& optionalExtensions) {
		// Get the number of physical devices
		uint32_t deviceCount;
		instance->GetLoader()->vkEnumeratePhysicalDevices(instance->GetInstance(), &deviceCount, nullptr);

		// Get the physical devices
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkPhysicalDevice* devices = (VkPhysicalDevice*)AllocMemory(deviceCount * sizeof(VkPhysicalDevice));
		PopMemoryUsageType();
		if(!devices)
			throw BadAllocException("Failed to allocate Vulkan physical devices array!");
		
		instance->GetLoader()->vkEnumeratePhysicalDevices(instance->GetInstance(), &deviceCount, devices);

		// Loop through every physical device and pich the one with the max score
		uint32_t maxScore = 0;
		VkPhysicalDevice maxDevice = VK_NULL_HANDLE;

		VkPhysicalDevice* devicesEnd = devices + deviceCount;
		for(VkPhysicalDevice* device = devices; device != devicesEnd; ++device) {
			// Caculate the current physical device's score
			uint32_t score = GetPhysicalDeviceScore(instance->GetLoader(), *device, surface, requiredExtensions, optionalExtensions);

			// Check if the current score is the highest score
			if(score > maxScore) {
				maxScore = score;
				maxDevice = *device;
			}
		}

		return maxDevice;
	}

	VulkanDevice::VulkanDevice(const VulkanLoader* loader, VulkanInstance* instance, VulkanSurface* surface) : loader(loader), instance(instance) {
		// Find the best physical device based on default criteria
		physicalDevice = FindBestPhysicalDevice(instance, surface, DEFAULT_REQUIRED_DEVICE_EXTENSIONS, DEFAULT_OPTIONAL_DEVICE_EXTENSIONS);
		if(!physicalDevice)
			throw Renderer::UnsupportedAPIException("Failed to find Vulkan physical device that supports all required criteria!");
		
		// Get the physical device's info
		GetPhysicalDeviceInfo(DEFAULT_REQUIRED_DEVICE_EXTENSIONS, DEFAULT_OPTIONAL_DEVICE_EXTENSIONS);

		// Create the logical device and get its queues
		CreateDevice(surface, true);
	}
	VulkanDevice::VulkanDevice(const VulkanLoader* loader, VulkanInstance* instance, VulkanSurface* surface, const set<const char_t*>& requiredExtensions, const set<const char_t*>& optionalExtensions) : loader(loader), instance(instance) {
		// Find the best physical device based the given criteria
		physicalDevice = FindBestPhysicalDevice(instance, surface, requiredExtensions, optionalExtensions);

		// Get the physical device's info
		GetPhysicalDeviceInfo(requiredExtensions, optionalExtensions);

		// Create the logical device and get its queues
		CreateDevice(surface, true);
	}
	VulkanDevice::VulkanDevice(const VulkanLoader* loader, VulkanInstance* instance, VkPhysicalDevice physicalDevice, const set<const char_t*>& enabledExtensions, const QueueFamilyIndices& queueFamilyIndices) : loader(loader), instance(instance), physicalDevice(physicalDevice), indices(queueFamilyIndices) {
		// Get the physical device's properties and features
		loader->vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		loader->vkGetPhysicalDeviceFeatures(physicalDevice, &features);

		// Create the logical device and get its queues
		CreateDevice(nullptr, false);
	}

	void VulkanDevice::RunCommandBuffers(size_t submitCount, const GPUCommandBufferSubmitInfo* submits, GPUFence* fence) {
		// Get the most demanding command buffer type
		GPUCommandBufferType commandType = GPU_COMMAND_BUFFER_TYPE_TRANSFER;
		for(size_t i = 0; i != submitCount; ++i) {
			for(auto* commandBuffer : submits[i].commandBuffers) {
				if(commandBuffer->GetType() < commandType)
					commandType = commandBuffer->GetType();
			}
		}

		// Get the queue to submit the command buffers to
		VkQueue submitQueue;
		switch(commandType) {
		case GPU_COMMAND_BUFFER_TYPE_GRAPHICS:
			submitQueue = graphicsQueue;
			break;
		case GPU_COMMAND_BUFFER_TYPE_COMPUTE:
			submitQueue = computeQueue;
			break;
		case GPU_COMMAND_BUFFER_TYPE_TRANSFER:
			submitQueue = transferQueue;
			break;
		}

		// Count the number of wait semaphores, signal semaphores and command buffers in the submits
		size_t waitSemaphoreCount = 0, signalSemaphoreCount = 0, commandBufferCount = 0;
		for(size_t i = 0; i != submitCount; ++i) {
			waitSemaphoreCount = submits[i].waitSemaphores.size();
			signalSemaphoreCount = submits[i].signalSemaphores.size();
			commandBufferCount = submits[i].commandBuffers.size();
		}

		// Allocate all required arrays
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkSubmitInfo* submitInfos = (VkSubmitInfo*)AllocMemory(sizeof(VkSubmitInfo) * submitCount + sizeof(VkSemaphore) * waitSemaphoreCount + sizeof(VkPipelineStageFlags) * waitSemaphoreCount + sizeof(VkSemaphore) * signalSemaphoreCount + sizeof(VkCommandBuffer) * commandBufferCount);
		PopMemoryUsageType();
		if(!submitInfos)
			throw BadAllocException("Failed to allocate Vulkan command buffer submit info!");
		
		VkSemaphore* waitSemaphores = (VkSemaphore*)(submitInfos + submitCount);
		VkPipelineStageFlags* waitDstStageMasks = (VkPipelineStageFlags*)(waitSemaphores + waitSemaphoreCount);
		VkSemaphore* signalSemaphores = (VkSemaphore*)(waitDstStageMasks + waitSemaphoreCount);
		VkCommandBuffer* commandBuffers = (VkCommandBuffer*)(signalSemaphores + signalSemaphoreCount);

		// Set the submit infos
		for(size_t i = 0; i != submitCount; ++i) {
			// Set the current submit info
			submitInfos[i].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfos[i].pNext = nullptr;
			submitInfos[i].waitSemaphoreCount = (uint32_t)submits[i].waitSemaphores.size();
			submitInfos[i].pWaitSemaphores = waitSemaphores;
			submitInfos[i].pWaitDstStageMask = waitDstStageMasks;
			submitInfos[i].commandBufferCount = (uint32_t)submits[i].commandBuffers.size();
			submitInfos[i].pCommandBuffers = commandBuffers;
			submitInfos[i].signalSemaphoreCount = (uint32_t)submits[i].signalSemaphores.size();
			submitInfos[i].pSignalSemaphores = signalSemaphores;

			// Set the current submit's wait semaphores and target stage masks
			for(size_t j = 0; j != submits[i].waitSemaphores.size(); ++j)
				waitSemaphores[j] = ((VulkanSemaphore*)submits[i].waitSemaphores[j]->GetInternalData())->GetSemaphore();
			waitSemaphores += submits[i].waitSemaphores.size();

			for(size_t j = 0; j != submits[i].waitStages.size(); ++j)
				waitDstStageMasks[j] = VulkanCommandBuffer::PipelineStageToVkPipelineStageFlags(submits[i].waitStages[j]);
			waitDstStageMasks += submits[i].waitStages.size();

			// Set the current submit's signal semaphores
			for(size_t j = 0; j != submits[i].signalSemaphores.size(); ++j)
				signalSemaphores[j] = ((VulkanSemaphore*)submits[i].signalSemaphores[j]->GetInternalData())->GetSemaphore();
			signalSemaphores += submits[i].signalSemaphores.size();

			// Set the current submit's command buffers
			for(size_t j = 0; j != submits[i].commandBuffers.size(); ++j)
				commandBuffers[j] = ((VulkanCommandBuffer*)submits[i].commandBuffers[j]->GetInternalData())->GetCommandBuffer();
			commandBuffers += submits[i].commandBuffers.size();
		}

		// Get the signal fence's handle
		VkFence fenceHandle;
		if(fence) {
			fenceHandle = ((VulkanFence*)fence->GetInternalData())->GetFence();
		} else {
			fenceHandle = VK_NULL_HANDLE;
		}

		// Submit the command buffers
		VkResult result = loader->vkQueueSubmit(submitQueue, (uint32_t)submitCount, submitInfos, fenceHandle);
		if(result != VK_SUCCESS)
			throw Exception("Failed to submit Vulkan command buffers for execution! Error code: %s", string_VkResult(result));
	}

	VulkanDevice::~VulkanDevice() {
		// Wait for the device to idle and destroy it
		loader->vkDeviceWaitIdle(device);
		loader->vkDestroyDevice(device, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
	}
}