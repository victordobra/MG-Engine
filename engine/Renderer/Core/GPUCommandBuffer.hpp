#pragma once

#include <Core.hpp>
#include "GPUCommandBufferStructs.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Vulkan/Core/VulkanCommandBuffer.hpp"
#include "GPUBuffer.hpp"
#include "GPUImage.hpp"
#include "GPUFence.hpp"
#include "GPUSemaphore.hpp"

namespace wfe {
	/// @brief An implementation of a GPU command buffer.
	class GPUCommandBuffer {
	public:
		/// @brief Creates a GPU command buffer.
		/// @param renderer The renderer to create the command buffer in.
		/// @param level The command buffer's submission level.
		/// @param type The command buffer's type.
		GPUCommandBuffer(Renderer* renderer, GPUCommandBufferLevel level, GPUCommandBufferType type) : api(renderer->GetRendererBackendAPI()) {
			// Use the constructor based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				new(internalData) VulkanCommandBuffer(renderer, level, type);
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}

		GPUCommandBuffer() = delete;
		GPUCommandBuffer(const GPUCommandBuffer&) = delete;
		GPUCommandBuffer(GPUCommandBuffer&&) noexcept = delete;

		GPUCommandBuffer& operator=(const GPUCommandBuffer&) = delete;
		GPUCommandBuffer& operator=(GPUCommandBuffer&&) noexcept = delete;

		/// @brief Gets the command buffer's level.
		/// @return The command buffer's level.
		GPUCommandBufferLevel GetLevel() const {
			// Call the get level function based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				return ((const VulkanCommandBuffer*)internalData)->GetLevel();
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Gets the command buffer's type.
		/// @return The command buffer's type.
		GPUCommandBufferType GetType() const {
			// Call the get type function based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				return ((const VulkanCommandBuffer*)internalData)->GetType();
			default:
				throw Exception("Invalid renderer API!");
			}
		}

		/// @brief Begins recording the command buffer.
		void BeginRecording() {
			// Call the begin recording function based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanCommandBuffer*)internalData)->BeginRecording();
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Ends recording the command buffer.
		void EndRecording() {
			// Call the end recording function based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanCommandBuffer*)internalData)->EndRecording();
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Resets the command buffer.
		void Reset() {
			// Call the reset function based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanCommandBuffer*)internalData)->Reset();
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}

		/// @brief Records a command which clears the given color image.
		/// @param image The color image to clear.
		/// @param clearValue A union containing the clear values used on the image.
		void CmdClearColorImage(GPUImage& image, GPUColorImageClearValue clearValue) {
			// Call the clear color image command record function based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanCommandBuffer*)internalData)->CmdClearColorImage(image, clearValue);
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Records a command which clears the given depth/stencil image.
		/// @param image The depth/stencil image to clear.
		/// @param depthValue The depth clear value used on the image (if a depth component exists).
		/// @param stencilValue The stencil clear value used on the iamge (if a stencil component exists).
		void CmdClearDepthStencilImage(GPUImage& image, float32_t depthValue, uint32_t stencilValue) {
			// Call the clear depth stencil image command record function based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanCommandBuffer*)internalData)->CmdClearDepthStencilImage(image, depthValue, stencilValue);
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Records a command which fills the given buffer.
		/// @param buffer The buffer to fill.
		/// @param offset The offset into the buffer at which to start filling. Must be a multiple of 4.
		/// @param size The number of bytes to fill. Must me a multiple of 4.
		/// @param data The 4-byte word repeated across the filled region.
		void CmdFillBuffer(GPUBuffer& buffer, uint64_t offset, uint64_t size, uint32_t data) {
			// Call the fill buffer command record function based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanCommandBuffer*)internalData)->CmdFillBuffer(buffer, offset, size, data);
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Records a command which updates the given buffer. Only recommended for small updates.
		/// @param buffer The buffer to update.
		/// @param offset The offset into the buffer at which to start updating. Must be a multiple of 4.
		/// @param size The updated area's size. Must be a multiple of 4.
		/// @param data The source data for the buffer update.
		void CmdUpdateBuffer(GPUBuffer& buffer, uint64_t offset, uint64_t size, void* data) {
			// Call the update buffer command record function based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanCommandBuffer*)internalData)->CmdUpdateBuffer(buffer, offset, size, data);
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Records a command which copies data from one buffer to another.
		/// @param srcBuffer The source buffer.
		/// @param dstBuffer The destination buffer.
		/// @param regionCount The number of copy regions.
		/// @param regions A pointer to the array of copy regions.
		void CmdCopyBuffer(GPUBuffer& srcBuffer, GPUBuffer& dstBuffer, size_t regionCount, const GPUBufferCopyRegion* regions) {
			// Call the copy buffer command record function based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanCommandBuffer*)internalData)->CmdCopyBuffer(srcBuffer, dstBuffer, regionCount, regions);
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Records a command which copies data from one image to another.
		/// @param srcImage The source image.
		/// @param dstImage The destination image.
		/// @param regionCount The number of copy regions.
		/// @param regions A pointer to the array of copy regions.
		void CmdCopyImage(GPUImage& srcImage, GPUImage& dstImage, size_t regionCount, const GPUImageCopyRegion* regions) {
			// Call the copy image command record function based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanCommandBuffer*)internalData)->CmdCopyImage(srcImage, dstImage, regionCount, regions);
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Records a command which copies data from a buffer to an image.
		/// @param buffer The source buffer.
		/// @param image The destination image.
		/// @param regionCount The number of copy regions.
		/// @param regions A pointer to the array of copy regions. 
		void CmdCopyBufferToImage(GPUBuffer& buffer, GPUImage& image, size_t regionCount, const GPUBufferImageCopyRegion* regions) {
			// Call the copy buffer to image command record function based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanCommandBuffer*)internalData)->CmdCopyBufferToImage(buffer, image, regionCount, regions);
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}
		/// @brief Records a command which copies data from an image to a buffer.
		/// @param image The source image.
		/// @param buffer The destination buffer.
		/// @param regionCount The number of copy regions.
		/// @param regions A pointer to the array of copy regions. 
		void CmdCopyImageToBuffer(GPUImage& image, GPUBuffer& buffer, size_t regionCount, const GPUBufferImageCopyRegion* regions) {
			// Call the copy image to buffer command record function based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanCommandBuffer*)internalData)->CmdCopyImageToBuffer(image, buffer, regionCount, regions);
				break;
			default:
				throw Exception("Invalid renderer API!");
			}
		}

		/// @brief Records a command which runs one or more secondary command buffers.
		/// @param commandBufferCount The number of command buffers to run.
		/// @param commandBuffers A pointer to the array of command buffers.
		void CmdRunCommandBuffers(size_t commandBufferCount, GPUCommandBuffer* commandBuffers) {
			// Call the run command buffers command record function based on the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanCommandBuffer*)internalData)->CmdRunCommandBuffers(commandBufferCount, commandBuffers);
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

		/// @brief Destroys the GPU command buffer.
		~GPUCommandBuffer() {
			// Call the destructor for the renderer's API
			switch(api) {
			case Renderer::RENDERER_BACKEND_API_VULKAN:
				((VulkanCommandBuffer*)internalData)->~VulkanCommandBuffer();
				break;
			}
		}
	private:
		char internalData[sizeof(VulkanCommandBuffer)];
		Renderer::RendererBackendAPI api;
	};
	/// @brief A struct which holds information about a GPU command buffer submit.
	struct GPUCommandBufferSubmitInfo {
		/// @brief A vector containing the semaphores to wait for before executing the command buffers.
		vector<GPUSemaphore*> waitSemaphores;
		/// @brief A vector containing the pipeline stages at which each corresponding semaphore wait will occur.
		vector<GPUPipelineStage> waitStages;
		/// @brief A vector containing the command buffers to execute.
		vector<GPUCommandBuffer*> commandBuffers;
		/// @brief A vector containing the semaphores to signal when all command buffers have completed execution.
		vector<GPUSemaphore*> signalSemaphores;
	};
}