#include "VulkanSwapChain.hpp"
#include "Renderer/Vulkan/VulkanRenderer.hpp"

#include <vulkan/vk_enum_string_helper.h>

namespace wfe {
	// Constants
	static const vector<VkFormat> DEPTH_FORMATS = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	static const VkCompositeAlphaFlagBitsKHR COMPOSITE_ALPHA = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	static const VkBool32 CLIPPED = VK_TRUE;

	// Window resize event callback
	static void* WindowResizeEventCallback(void* args, void* userData) {
		// Recreate the current swap chain
		VulkanSwapChain* swapChain = (VulkanSwapChain*)userData;
		swapChain->RecreateSwapChain();

		return nullptr;
	}

	// Internal helper functions
	void VulkanSwapChain::CreateSwapChain(VkSwapchainKHR oldSwapChain) {
		// Get the surface's capabilities
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		device->GetLoader()->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->GetPhysicalDevice(), surface->GetSurface(), &surfaceCapabilities);

		// Set the swap chain's extent
		if(surfaceCapabilities.currentExtent.width == UINT32_T_MAX || surfaceCapabilities.currentExtent.height == UINT32_T_MAX) {
			// Set the swap chain's extent to the window's size
			swapChainExtent.width = surface->GetWindow()->GetWindowInfo().width;
			swapChainExtent.height = surface->GetWindow()->GetWindowInfo().height;

			// Clamp the extents within their limits
			if(swapChainExtent.width < surfaceCapabilities.minImageExtent.width) {
				swapChainExtent.width = surfaceCapabilities.minImageExtent.width;
			} else if(swapChainExtent.width > surfaceCapabilities.maxImageExtent.width) {
				swapChainExtent.width = surfaceCapabilities.maxImageExtent.width;
			}

			if(swapChainExtent.height < surfaceCapabilities.minImageExtent.height) {
				swapChainExtent.height = surfaceCapabilities.minImageExtent.height;
			} else if(swapChainExtent.height > surfaceCapabilities.maxImageExtent.height) {
				swapChainExtent.height = surfaceCapabilities.maxImageExtent.height;
			}
		} else {
			// Set the swap chain's extent to the surface's extent
			swapChainExtent = surfaceCapabilities.currentExtent;
		}

		// Exit the function if the surface's window is minimized
		if(!swapChainExtent.width || !swapChainExtent.height) {
			swapChain = VK_NULL_HANDLE;
			return;
		}

		// Set the swap chain's minimum image count
		uint32_t minImageCount = surfaceCapabilities.minImageCount + 1;
		if(minImageCount > surfaceCapabilities.maxImageCount)
			minImageCount = surfaceCapabilities.maxImageCount;

		// Set the swap chain's queue family info
		VulkanDevice::QueueFamilyIndices queueFamilyIndices = device->GetQueueFamilyIndices();

		uint32_t swapChainIndices[] = { queueFamilyIndices.graphicsIndex, queueFamilyIndices.presentIndex };
		uint32_t swapChainIndexCount;
		VkSharingMode swapChainSharingMode;

		if(queueFamilyIndices.graphicsIndex == queueFamilyIndices.presentIndex) {
			swapChainSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapChainIndexCount = 1;
		} else {
			swapChainSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapChainIndexCount = 2;
		}

		// Set the swap chain's create info
		VkSwapchainCreateInfoKHR createInfo {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.surface = surface->GetSurface(),
			.minImageCount = minImageCount,
			.imageFormat = settings.imageFormat,
			.imageColorSpace = settings.colorSpace,
			.imageExtent = swapChainExtent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = swapChainSharingMode,
			.queueFamilyIndexCount = swapChainIndexCount,
			.pQueueFamilyIndices = swapChainIndices,
			.preTransform = surfaceCapabilities.currentTransform,
			.compositeAlpha = settings.compositeAlpha,
			.presentMode = settings.presentMode,
			.clipped = settings.clipped,
			.oldSwapchain = oldSwapChain
		};

		// Create the swap chain
		VkResult result = device->GetLoader()->vkCreateSwapchainKHR(device->GetDevice(), &createInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &swapChain);
		if(result != VK_SUCCESS)
			throw Exception("Failed to create Vulkan swap chain! Error code: %s", string_VkResult(result));
	}
	void VulkanSwapChain::GetSwapChainImageViews() {
		// Exit hte function if the swap chain does not exist
		if(!swapChain)
			return;

		// Get the swap chain's image count
		uint32_t imageCount;
		device->GetLoader()->vkGetSwapchainImagesKHR(device->GetDevice(), swapChain, &imageCount, nullptr);

		// Get the swap chain's images
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkImage* swapChainImagesArr = (VkImage*)AllocMemory(sizeof(VkImage) * imageCount);
		PopMemoryUsageType();
		if(!swapChainImagesArr)
			throw BadAllocException("Failed to allocate Vulkan swap chain images array!");
		
		device->GetLoader()->vkGetSwapchainImagesKHR(device->GetDevice(), swapChain, &imageCount, swapChainImagesArr);

		// Copy the images to the swap chain image vector
		swapChainImages.resize(imageCount);
		for(uint32_t i = 0; i != imageCount; ++i)
			swapChainImages[i].image = swapChainImagesArr[i];
		
		// Free the swap chain image array
		FreeMemory(swapChainImagesArr);
		
		// Set the swap chain image views' create infos
		VkImageViewCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = settings.imageFormat,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_R,
				.g = VK_COMPONENT_SWIZZLE_G,
				.b = VK_COMPONENT_SWIZZLE_B,
				.a = VK_COMPONENT_SWIZZLE_A
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		// Create every swap chain image view
		for(auto& swapChainImage : swapChainImages) {
			// Set the create info's image
			createInfo.image = swapChainImage.image;

			// Create the image view
			VkResult result = device->GetLoader()->vkCreateImageView(device->GetDevice(), &createInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &swapChainImage.imageView);
			if(result != VK_SUCCESS)
				throw Exception("Failed to create Vulkan swap chain image view! Error code: %s", string_VkResult(result));
		}
	}
	void VulkanSwapChain::CreateSwapChainDepthImages() {
		// Exit hte function if the swap chain does not exist
		if(!swapChain)
			return;

		// Set the swap chain depth images' queue family info
		VulkanDevice::QueueFamilyIndices queueFamilyIndices = device->GetQueueFamilyIndices();

		uint32_t depthImageIndices[] = { queueFamilyIndices.graphicsIndex, queueFamilyIndices.presentIndex };
		uint32_t depthImageIndexCount;
		VkSharingMode depthImageSharingMode;

		if(queueFamilyIndices.graphicsIndex == queueFamilyIndices.presentIndex) {
			depthImageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			depthImageIndexCount = 1;
		} else {
			depthImageSharingMode = VK_SHARING_MODE_CONCURRENT;
			depthImageIndexCount = 2;
		}

		// Set the depth image create info
		VkImageCreateInfo imageInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = settings.depthFormat,
			.extent = { swapChainExtent.width, swapChainExtent.height, 1 },
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			.sharingMode = depthImageSharingMode,
			.queueFamilyIndexCount = depthImageIndexCount,
			.pQueueFamilyIndices = depthImageIndices,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};

		// Set the depth image view create info
		VkImageViewCreateInfo imageViewInfo {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = settings.depthFormat,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_R,
				.g = VK_COMPONENT_SWIZZLE_G,
				.b = VK_COMPONENT_SWIZZLE_B,
				.a = VK_COMPONENT_SWIZZLE_A
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		// Create every depth image
		for(auto& swapChainImage : swapChainImages) {
			// Create the depth image
			VkResult result = device->GetLoader()->vkCreateImage(device->GetDevice(), &imageInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &swapChainImage.depthImage);
			if(result != VK_SUCCESS)
				throw Exception("Failed to create Vulkan swap chain depth image! Error code: %s", string_VkResult(result));
			
			// Allocate the depth image's memory
			result = allocator->AllocImageMemory(swapChainImage.depthImage, VulkanAllocator::MEMORY_TYPE_GPU, swapChainImage.depthImageMemory);
			if(result != VK_SUCCESS)
				throw Exception("Failed to allocate Vulkan swap chain depth image memory! Error code: %s", string_VkResult(result));
			
			// Create the depth image view
			imageViewInfo.image = swapChainImage.depthImage;
			result = device->GetLoader()->vkCreateImageView(device->GetDevice(), &imageViewInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &swapChainImage.depthImageView);
			if(result != VK_SUCCESS)
				throw Exception("Failed to create Vulkan swap chain depth image view! Error code: %s", string_VkResult(result));
		}

		// Allocate the depth image and memory arrays
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkImage* depthImages = (VkImage*)AllocMemory(sizeof(VkImage) * swapChainImages.size() + sizeof(VulkanAllocator::MemoryBlock) * swapChainImages.size());
		PopMemoryUsageType();
		if(!depthImages)
			throw BadAllocException("Failed to allocate Vulkan swap chain depth image and memory bind array!");
		
		VulkanAllocator::MemoryBlock* depthImageMemories = (VulkanAllocator::MemoryBlock*)(depthImages + swapChainImages.size());

		// Fill the depth image and memory arrays
		for(size_t i = 0; i != swapChainImages.size(); ++i) {
			depthImages[i] = swapChainImages[i].depthImage;
			depthImageMemories[i] = swapChainImages[i].depthImageMemory;
		}

		// Bind the depth image memories
		VkResult result = allocator->BindImageMemories(swapChainImages.size(), depthImages, depthImageMemories);
		if(result != VK_SUCCESS)
			throw Exception("Failed to bind Vulkan swap chain depth image memories! Error code: %s", string_VkResult(result));

		// Free the depth image and memory arrays
		FreeMemory(depthImages);
	}
	void VulkanSwapChain::CreateRenderPass() {
		// Exit hte function if the swap chain does not exist
		if(!swapChain)
			return;

		// Set the attachment descriptions
		VkAttachmentDescription attachments[] = {
			{
				.flags = 0,
				.format = settings.imageFormat,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			},
			{
				.flags = 0,
				.format = settings.depthFormat,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			}
		};

		// Set the attachment references
		VkAttachmentReference colorAttachmentRef {
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};
		VkAttachmentReference depthAttachmentRef {
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		};

		// Set the subpass description
		VkSubpassDescription subpass {
			.flags = 0,
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = 0,
			.pInputAttachments = nullptr,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentRef,
			.pResolveAttachments = nullptr,
			.pDepthStencilAttachment = &depthAttachmentRef,
			.preserveAttachmentCount = 0,
			.pPreserveAttachments = nullptr
		};

		// Set the render pass create info
		VkRenderPassCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.attachmentCount = 2,
			.pAttachments = attachments,
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 0,
			.pDependencies = nullptr
		};

		// Create the render pass
		VkResult result = device->GetLoader()->vkCreateRenderPass(device->GetDevice(), &createInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &renderPass);
		if(result != VK_SUCCESS)
			throw Exception("Failed to create Vulkan render pass! Error code: %s", string_VkResult(result));
	}
	void VulkanSwapChain::CreateFramebuffers() {
		// Exit hte function if the swap chain does not exist
		if(!swapChain)
			return;

		// Set the framebffer create info
		VkFramebufferCreateInfo createInfo {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.renderPass = renderPass,
			.attachmentCount = 2,
			.width = swapChainExtent.width,
			.height = swapChainExtent.height,
			.layers = 1
		};

		// Create every framebuffer
		for(auto& swapChainImage : swapChainImages) {
			// Set the create info's attachments
			VkImageView attachments[] { swapChainImage.imageView, swapChainImage.depthImageView };
			createInfo.pAttachments = attachments;

			// Create the framebuffer
			VkResult result = device->GetLoader()->vkCreateFramebuffer(device->GetDevice(), &createInfo, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS, &swapChainImage.framebuffer);
			if(result != VK_SUCCESS)
				throw Exception("Failed to create Vulkan swap chain framebuffer! Error code: %s", string_VkResult(result));
		}
	}

	// Public functions
	VulkanSwapChain::SwapChainSettings VulkanSwapChain::GetDefaultSwapChainSettings(VulkanSurface* surface, VulkanDevice* device) {
		// Get the number of supported surface formats and present modes
		uint32_t formatCount, presentModeCount;
		device->GetLoader()->vkGetPhysicalDeviceSurfaceFormatsKHR(device->GetPhysicalDevice(), surface->GetSurface(), &formatCount, nullptr);
		device->GetLoader()->vkGetPhysicalDeviceSurfacePresentModesKHR(device->GetPhysicalDevice(), surface->GetSurface(), &presentModeCount, nullptr);

		// Allocate all required arrays
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkSurfaceFormatKHR* surfaceFormats = (VkSurfaceFormatKHR*)AllocMemory(sizeof(VkSurfaceFormatKHR) * formatCount + sizeof(VkPresentModeKHR) * presentModeCount);
		PopMemoryUsageType();
		if(!surfaceFormats)
			throw BadAllocException("Failed to allocate supported Vulkan surface formats and present modes array!");
		
		VkPresentModeKHR* presentModes = (VkPresentModeKHR*)(surfaceFormats + formatCount);

		// Get the supported surface formats and present modes
		device->GetLoader()->vkGetPhysicalDeviceSurfaceFormatsKHR(device->GetPhysicalDevice(), surface->GetSurface(), &formatCount, surfaceFormats);
		device->GetLoader()->vkGetPhysicalDeviceSurfacePresentModesKHR(device->GetPhysicalDevice(), surface->GetSurface(), &presentModeCount, presentModes);

		// Set the image format and color space based on their scores
		SwapChainSettings settings;

		settings.imageFormat = surfaceFormats[0].format;
		settings.colorSpace = surfaceFormats[0].colorSpace;

		uint32_t maxScore = ((settings.imageFormat == VK_FORMAT_R8G8B8A8_SRGB) || (settings.imageFormat == VK_FORMAT_B8G8R8A8_SRGB)) + (settings.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);

		for(uint32_t i = 1; i != formatCount; ++i) {
			// Set the current surface format's score
			uint32_t score = ((surfaceFormats[i].format == VK_FORMAT_R8G8B8A8_SRGB) || (surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB)) + (surfaceFormats[i].colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);

			// Check if the current score is the maximum score
			if(score > maxScore) {
				// Set the new max score and surface format
				maxScore = score;
				settings.imageFormat = surfaceFormats[i].format;
				settings.colorSpace = surfaceFormats[i].colorSpace;
			}
		}

		// Check for any supported depth formats
		for(auto depthFormat : DEPTH_FORMATS) {
			// Get the depth format's properties
			VkFormatProperties depthProperties;
			device->GetLoader()->vkGetPhysicalDeviceFormatProperties(device->GetPhysicalDevice(), depthFormat, &depthProperties);

			// Set the depth format if it has the depth stencil feature
			if(depthProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				settings.depthFormat = depthFormat;
				break;
			}
		}

		// Set the present mode to FIFO, or mailpox, if supported
		settings.presentMode = VK_PRESENT_MODE_FIFO_KHR;

		for(uint32_t i = 0; i != presentModeCount; ++i) {
			if(presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
				settings.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
		}

		// Set the other settings
		settings.compositeAlpha = COMPOSITE_ALPHA;
		settings.clipped = CLIPPED;

		return settings;
	}
	bool8_t VulkanSwapChain::CheckSwapChainSettingsSupport(VulkanSurface* surface, VulkanDevice* device, const SwapChainSettings& settings) {
		// Get the number of supported surface formats and present modes
		uint32_t formatCount, presentModeCount;
		device->GetLoader()->vkGetPhysicalDeviceSurfaceFormatsKHR(device->GetPhysicalDevice(), surface->GetSurface(), &formatCount, nullptr);
		device->GetLoader()->vkGetPhysicalDeviceSurfacePresentModesKHR(device->GetPhysicalDevice(), surface->GetSurface(), &presentModeCount, nullptr);

		// Allocate all required arrays
		PushMemoryUsageType(MEMORY_USAGE_TYPE_COMMAND);
		VkSurfaceFormatKHR* surfaceFormats = (VkSurfaceFormatKHR*)AllocMemory(sizeof(VkSurfaceFormatKHR) * formatCount + sizeof(VkPresentModeKHR) * presentModeCount);
		PopMemoryUsageType();
		if(!surfaceFormats)
			throw BadAllocException("Failed to allocate supported Vulkan surface formats and present modes array!");
		
		VkPresentModeKHR* presentModes = (VkPresentModeKHR*)(surfaceFormats + formatCount);
		
		// Get the supported surface formats and present modes
		device->GetLoader()->vkGetPhysicalDeviceSurfaceFormatsKHR(device->GetPhysicalDevice(), surface->GetSurface(), &formatCount, surfaceFormats);
		device->GetLoader()->vkGetPhysicalDeviceSurfacePresentModesKHR(device->GetPhysicalDevice(), surface->GetSurface(), &presentModeCount, presentModes);

		// Check if the settings' format is supported
		bool8_t supported = false;
		for(uint32_t i = 0; i != formatCount; ++i) {
			if(surfaceFormats[i].format == settings.imageFormat && surfaceFormats[i].colorSpace == settings.colorSpace) {
				supported = true;
				break;
			}
		}

		// Exit the function if the surface format is not supported
		if(!supported) {
			FreeMemory(surfaceFormats);
			return false;
		}
		
		// Check if the settings' present mode is supported
		for(uint32_t i = 0; i != presentModeCount; ++i) {
			if(presentModes[i] == settings.presentMode) {
				supported = true;
				break;
			}
		}

		// Exit the function if the present mode is not supported
		if(!supported) {
			FreeMemory(surfaceFormats);
			return false;
		}

		// Free the supported surface format and present modes array
		FreeMemory(surfaceFormats);
		
		// Get the depth format's properties
		VkFormatProperties depthProperties;
		device->GetLoader()->vkGetPhysicalDeviceFormatProperties(device->GetPhysicalDevice(), settings.depthFormat, &depthProperties);

		// Check if the depth format has the depth stencil feature
		return depthProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	VulkanSwapChain::VulkanSwapChain(VulkanSurface* surface, VulkanDevice* device, VulkanAllocator* allocator) : surface(surface), device(device), allocator(allocator), settings(GetDefaultSwapChainSettings(surface, device)) {
		// Create the swap chain's components
		CreateSwapChain(VK_NULL_HANDLE);
		GetSwapChainImageViews();
		CreateSwapChainDepthImages();
		CreateRenderPass();
		CreateFramebuffers();

		// Add the window resize event listener
		surface->GetWindow()->GetResizeEvent().AddListener(Event::Listener(WindowResizeEventCallback, this));
	}
	VulkanSwapChain::VulkanSwapChain(VulkanSurface* surface, VulkanDevice* device, VulkanAllocator* allocator, const SwapChainSettings& swapChainSettings) : surface(surface), device(device), allocator(allocator), settings(swapChainSettings) {
		// Check if the given settings are supported
		if(!CheckSwapChainSettingsSupport(surface, device, settings))
			throw Exception("Unsupported Vulkan swap chain settings!");
		
		// Create the swap chain's components
		CreateSwapChain(VK_NULL_HANDLE);
		GetSwapChainImageViews();
		CreateSwapChainDepthImages();
		CreateRenderPass();
		CreateFramebuffers();

		// Add the window resize event listener
		surface->GetWindow()->GetResizeEvent().AddListener(Event::Listener(WindowResizeEventCallback, this));
	}

	void VulkanSwapChain::RecreateSwapChain() {
		// Destroy the old swap chain's components, if it exists
		if(swapChain) {
			// Destroy the swap chain's images
			for(auto& swapChainImage : swapChainImages) {
				// Destroy the framebuffer
				device->GetLoader()->vkDestroyFramebuffer(device->GetDevice(), swapChainImage.framebuffer, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);

				// Destroy the depth image
				device->GetLoader()->vkDestroyImageView(device->GetDevice(), swapChainImage.depthImageView, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
				allocator->FreeMemory(swapChainImage.depthImageMemory);
				device->GetLoader()->vkDestroyImage(device->GetDevice(), swapChainImage.depthImage, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);

				// Destroy the color image view
				device->GetLoader()->vkDestroyImageView(device->GetDevice(), swapChainImage.imageView, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
			}

			// Destroy the render pass
			device->GetLoader()->vkDestroyRenderPass(device->GetDevice(), renderPass, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
		}

		// Save the old swap chain
		VkSwapchainKHR oldSwapChain = swapChain;

		// Create the new swap chain
		CreateSwapChain(oldSwapChain);

		// Destroy the old swap chain, if it exists
		if(oldSwapChain)
			device->GetLoader()->vkDestroySwapchainKHR(device->GetDevice(), oldSwapChain, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
		
		// Create the swap chain's components
		GetSwapChainImageViews();
		CreateSwapChainDepthImages();
		CreateRenderPass();
		CreateFramebuffers();
	}

	bool8_t VulkanSwapChain::SetSwapChainSettings(const SwapChainSettings& newSettings) {
		// Exit the function if the swap chain's settings are not supported
		if(!CheckSwapChainSettingsSupport(surface, device, newSettings))
			return false;

		// Set the new settings and recreate the swap chain
		settings = newSettings;
		RecreateSwapChain();

		return true;
	}

	VulkanSwapChain::~VulkanSwapChain() {
		// Remove the window resize event listener
		surface->GetWindow()->GetResizeEvent().AddListener(Event::Listener(WindowResizeEventCallback, this));

		// Destroy the swap chain, if it exists
		if(swapChain) {
			// Destroy the swap chain's images
			for(auto& swapChainImage : swapChainImages) {
				// Destroy the framebuffer
				device->GetLoader()->vkDestroyFramebuffer(device->GetDevice(), swapChainImage.framebuffer, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);

				// Destroy the depth image
				device->GetLoader()->vkDestroyImageView(device->GetDevice(), swapChainImage.depthImageView, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
				allocator->FreeMemory(swapChainImage.depthImageMemory);
				device->GetLoader()->vkDestroyImage(device->GetDevice(), swapChainImage.depthImage, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);

				// Destroy the color image view
				device->GetLoader()->vkDestroyImageView(device->GetDevice(), swapChainImage.imageView, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
			}

			// Destroy the render pass
			device->GetLoader()->vkDestroyRenderPass(device->GetDevice(), renderPass, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);

			// Destroy the swap chain
			device->GetLoader()->vkDestroySwapchainKHR(device->GetDevice(), swapChain, &VulkanRenderer::VULKAN_ALLOC_CALLBACKS);
		}
	}
}