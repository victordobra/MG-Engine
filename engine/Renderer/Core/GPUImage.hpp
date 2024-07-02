#pragma once

#include <Core.hpp>
#include "GPUImageEnums.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Vulkan/Core/VulkanImage.hpp"

namespace wfe {
	/// @brief An implementation of a GPU image.
	class GPUImage {
	public:

		/// @brief Creates a GPU image.
		/// @param renderer The renderer to create the image in.
		/// @param width The image's width.
		/// @param height The image's height.
		/// @param depth The image's depth.
		/// @param imageType The image's type.
		/// @param imageFormat The image's format.
		/// @param canMap True if the image can be mapped, otherwise false.
		GPUImage(Renderer* renderer, uint32_t width, uint32_t height, uint32_t depth, GPUImageType imageType, GPUImageFormat imageFormat, bool8_t canMap) : api(renderer->GetRendererBackendAPI()) {
			// Use the constructor based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				new(internalData) VulkanImage(renderer, width, height, depth, imageType, imageFormat, canMap);
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}

		GPUImage() = delete;
		GPUImage(const GPUImage&) = delete;
		GPUImage(GPUImage&&) noexcept = delete;

		GPUImage& operator=(const GPUImage&) = delete;
		GPUImage& operator=(GPUImage&&) = delete;

		/// @brief Gets the image's width.
		/// @return The image's width.
		uint32_t GetWidth() const {
			// Call the get width function for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				return ((const VulkanImage*)internalData)->GetWidth();
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Gets the image's height.
		/// @return The image's height.
		uint32_t GetHeight() const {
			// Call the get height function for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				return ((const VulkanImage*)internalData)->GetHeight();
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Gets the image's depth.
		/// @return The image's depth.
		uint32_t GetDepth() const {
			// Call the get depth function for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				return ((const VulkanImage*)internalData)->GetDepth();
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Gets the image's mapped memory.
		/// @return A pointer to the image's mapped memory, or nullptr if the image isn't mapped.
		void* GetMappedMemory() {
			// Call the get mapped memory function for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				return ((VulkanImage*)internalData)->GetMappedMemory();
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Gets the image's mapped memory.
		/// @return A const pointer to the image's mapped memory, or nullptr if the image isn't mapped.
		const void* GetMappedMemory() const {
			// Call the get mapped memory function for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				return ((const VulkanImage*)internalData)->GetMappedMemory();
			default:
				throw Exception("Invalid renderer API!");
			}
		}

		/// @brief Gets the internal image implementation data, which can be used based on the renderer's API.
		/// @return A void pointer to the internal implementation's class.
		void* GetInternalData() {
			return internalData;
		}

		/// @brief Destroys the GPU image.
		~GPUImage() {
			// Call the destructor for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanImage*)internalData)->~VulkanImage();
				break;
			}
		}
	private:
		Renderer::RendererBackendAPI api;
		char internalData[sizeof(VulkanImage)];
	};
}