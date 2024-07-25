#pragma once

#include "VulkanInstance.hpp"
#include "VulkanSurface.hpp"

#include <Core.hpp>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

namespace wfe {
	struct GPUCommandBufferSubmitInfo;
	class GPUFence;

	/// @brief A wrapper for a Vulkan logical device.
	class VulkanDevice {
	public:
		/// @brief A struct containing the queue family indices to be used by a device.
		struct QueueFamilyIndices {
			/// @brief The graphics queue family's index, or UINT32_T_MAX if one wasn't found.
			uint32_t graphicsIndex = UINT32_T_MAX;
			/// @brief The present queue family's index, or UINT32_T_MAX if one wasn't found.
			uint32_t presentIndex = UINT32_T_MAX;
			/// @brief The transfer queue family's index, or UINT32_T_MAX if one wasn't found.
			uint32_t transferIndex = UINT32_T_MAX;
			/// @brief The compute queue family's index, or UINT32_T_MAX if one wasn't found.
			uint32_t computeIndex = UINT32_T_MAX;
		};

		/// @brief Gets the extensions required by the Vulkan device implementation by default.
		/// @return A set containing the names of all Vulkan required extensions.
		static const set<const char_t*>& GetDefaultRequiredDeviceExtensions();
		/// @brief Gets the extensions optionally required by the Vulkan device implementation by default.
		/// @return A set containing the names of all Vulkan optional extensions.
		static const set<const char_t*>& GetDefaultOptionalDeviceExtensions();

		/// @brief Finds the best queue family indices for the given physical device.
		/// @param loader The Vulkan loader whose function pointers to use, or nullptr if the function will use the static Vulkan functions.
		/// @param physicalDevice The Vulkan physical device whose best queue family indices to find.
		/// @param surface The Vulkan surface to check for present support, or nullptr if presenting is not required.
		/// @return A struct containing optinal graphics, present, transfer and compute queue family indices.
		static QueueFamilyIndices FindPhysicalDeviceQueueFamilyIndices(const VulkanLoader* loader, VkPhysicalDevice physicalDevice, VulkanSurface* surface);
		/// @brief Checks if the given device extensions are supported by the given physical device.
		/// @param loader The Vulkan loader whose function pointers to use, or nullptr if the function will use the static Vulkan functions.
		/// @param physicalDevice The Vulkan physical device to check.
		/// @param extensionNames A set containing the names of the Vulkan device extensions to check.
		/// @param supported An optional pointer to an array of booleans, in which every index will be set to true if the corresponding extension is supported, otherwise to false. 
		/// @return The number of supported Vulkan device extensions from the given set.
		static size_t CheckForExtensionSupport(const VulkanLoader* loader, VkPhysicalDevice physicalDevice, const set<const char_t*>& extensionNames, bool8_t* supported = nullptr);
		/// @brief Gives the given Vulkan physical device a score based on its version, type, supported extensions and other criteria.
		/// @param loader The Vulkan loader whose function pointers to use, or nullptr if the function will use the static Vulkan functions.
		/// @param physicalDevice The Vulkan physical device to score.
		/// @param surface The Vulkan surface to check support for, or nullptr if presenting is not required.
		/// @param requiredExtensions A set containing the names of the Vulkan device extensions required by the physical device.
		/// @param optionalExtensions A set containing the names of the Vulkan device extensions optionally required by the physical device.
		/// @return The strictly positive score the Vulkan physical device was given, or 0 if it lacks any required features.
		static uint32_t GetPhysicalDeviceScore(const VulkanLoader* loader, VkPhysicalDevice physicalDevice, VulkanSurface* surface, const set<const char_t*>& requiredExtensions, const set<const char_t*>& optionalExtensions);
		/// @brief Finds the Vulkan physical device with the highest score based on the given conditions.
		/// @param instance The Vulkan instance whose physical devices to compare.
		/// @param surface The Vulkan surface to check support for, or nullptr if presenting is not required.
		/// @param requiredExtensions A set containing the names of the Vulkan device extensions required by the physical devices.
		/// @param optionalExtensions A set containing the names of the Vulkan device extensions optionally required by the physical devices.
		/// @return The handle of the Vulkan physical device with the highest score, or VK_NULL_HANDLE if no Vulkan physical device has the required features.
		static VkPhysicalDevice FindBestPhysicalDevice(VulkanInstance* instance, VulkanSurface* surface, const set<const char_t*>& requiredExtensions, const set<const char_t*>& optionalExtensions);

		/// @brief Creates a Vulkan logical device using the physical device with the highest score based on the default extensions.
		/// @param loader The Vulkan loader whose function pointers to use, or nullptr if the function will use the static Vulkan functions.
		/// @param instance The Vulkan instance to create the logical device in.
		/// @param surface The Vulkan surface to check support for, or nullptr if presenting is not required.
		VulkanDevice(const VulkanLoader* loader, VulkanInstance* instance, VulkanSurface* surface);
		/// @brief Creates a Vulkan logical device using the physical device with the highest score based on the given extensions.
		/// @param loader The Vulkan loader whose function pointers to use, or nullptr if the function will use the static Vulkan functions.
		/// @param instance The Vulkan instance to create the logical device in.
		/// @param surface The Vulkan surface to check support for, or nullptr if presenting is not required.
		/// @param requiredExtensions A set containing the names of the Vulkan device extensions required by the device.
		/// @param optionalExtensions A set containing the names of the Vulkan device extensions optionally required by the device.
		VulkanDevice(const VulkanLoader* loader, VulkanInstance* instance, VulkanSurface* surface, const set<const char_t*>& requiredExtensions, const set<const char_t*>& optionalExtensions);
		/// @brief Creates a Vulkan logical device using the given physical device with the given extensions.
		/// @param loader The Vulkan loader whose function pointers to use, or nullptr if the function will use the static Vulkan functions.
		/// @param instance The Vulkan instance to create the logical device in.
		/// @param physicalDevice The Vulkan physical device to create the logical device for.
		/// @param enabledExtensions A set containing the names of the Vulkan device extensions to enable in the device.
		/// @param queueFamilyIndices A struct containing the queue family indices to use for the graphics, present, transfer and compute queues.
		VulkanDevice(const VulkanLoader* loader, VulkanInstance* instance, VkPhysicalDevice physicalDevice, const set<const char_t*>& enabledExtensions, const QueueFamilyIndices& queueFamilyIndices);
		VulkanDevice(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&&) noexcept = delete;

		VulkanDevice& operator=(const VulkanDevice&) = delete;
		VulkanDevice& operator=(VulkanDevice&&) = delete;

		/// @brief Gets the Vulkan function loader used by the device.
		/// @return A pointer to the Vulkan loader used by the device.
		const VulkanLoader* GetLoader() const {
			return loader;
		}
		/// @brief Gets the Vulkan instance that owns the device.
		/// @return A pointer to the Vulkan instance wrapper object.
		VulkanInstance* GetInstance() {
			return instance;
		}
		/// @brief Gets the Vulkan instance that owns the device.
		/// @return A const pointer to the Vulkan instance wrapper object.
		const VulkanInstance* GetInstance() const {
			return instance;
		}

		/// @brief Gets the Vulkan physical device this logical device was created for.
		/// @return A handle to the current Vulkan physical device.
		VkPhysicalDevice GetPhysicalDevice() {
			return physicalDevice;
		}
		/// @brief Gets the Vulkan logical device of the implementation.
		/// @return A handle to the Vulkan logical device.
		VkDevice GetDevice() {
			return device;
		}
		/// @brief Gets the Vulkan graphics queue of the device.
		/// @return A handle to the Vulkan graphics queue of the device, or VK_NULL_HANDLE if graphics aren't used on the device.
		VkQueue GetGraphicsQueue() {
			return graphicsQueue;
		}
		/// @brief Gets the Vulkan present queue of the device.
		/// @return A handle to the Vulkan present queue of the device, or VK_NULL_HANDLE if presenting isn't used on the device.
		VkQueue GetPresentQueue() {
			return presentQueue;
		}
		/// @brief Gets the Vulkan transfer queue of the device.
		/// @return A handle to the Vulkan transfer queue of the device.
		VkQueue GetTransferQueue() {
			return transferQueue;
		}
		/// @brief Gets the Vulkan compute queue of the device.
		/// @return A handle to the Vulkan compute queue of the device.
		VkQueue GetComputeQueue() {
			return computeQueue;
		}

		/// @brief Gets the Vulkan device's enabled extensions.
		/// @return A set containing the names of all enabled extensions.
		const set<const char_t*>& GetEnabledExtensions() const {
			return extensions;
		}
		/// @brief Gets the Vulkan queue family indices used by the device.
		/// @return A struct containing the graphics, present, compute and transfer queue family indices.
		const QueueFamilyIndices& GetQueueFamilyIndices() const {
			return indices;
		}
		/// @brief Gets the Vulkan physical device's properties.
		/// @return The struct containing the phyical device's general properties.
		const VkPhysicalDeviceProperties& GetDeviceProperties() const {
			return properties;
		}
		/// @brief Gets the Vulkan physical device's features.
		/// @return The struct containing the physical device's features.
		const VkPhysicalDeviceFeatures& GetDeviceFeatures() const {
			return features;
		}

		/// @brief Runs the given command buffers.
		/// @param submitCount The number of command buffer submits to run.
		/// @param submits A pointer to the array of command buffer submits.
		/// @param fence A pointer to the fence to signal once all command buffers finish execution, or nullptr if no fence will be signaled.
		void RunCommandBuffers(size_t submitCount, const GPUCommandBufferSubmitInfo* submits, GPUFence* fence);

		/// @brief Destroys the Vulkan logical device.
		~VulkanDevice();
	private:
		void GetPhysicalDeviceInfo(const set<const char_t*>& requiredExtensions, const set<const char_t*>& optionalExtensions);
		void CreateDevice(VulkanSurface* surface, bool8_t setQueueFamilyIndices);

		const VulkanLoader* loader;
		VulkanInstance* instance;

		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkQueue transferQueue;
		VkQueue computeQueue;

		set<const char_t*> extensions;
		QueueFamilyIndices indices;
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
	};
}