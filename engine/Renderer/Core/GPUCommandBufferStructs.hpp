#pragma once

#include <Core.hpp>

namespace wfe {
	/// @brief The submission level of a GPU command buffer.
	enum GPUCommandBufferLevel {
		/// @brief The command buffer level which will be directly submitted for execution.
		GPU_COMMAND_BUFFER_LEVEL_PRIMARY,
		/// @brief The command buffer level which will be executed by another command buffer.
		GPU_COMMAND_BUFFER_LEVEL_SECONDARY
	};
	/// @brief The type of a GPU command buffer.
	enum GPUCommandBufferType {
		/// @brief The command buffer type that can contain graphics and transfer commands.
		GPU_COMMAND_BUFFER_TYPE_GRAPHICS,
		/// @brief The command buffer type that can contain compute and transfer commands.
		GPU_COMMAND_BUFFER_TYPE_COMPUTE,
		/// @brief The command buffer type that can contain transfer commands.
		GPU_COMMAND_BUFFER_TYPE_TRANSFER
	};
	/// @brief The flags corresponding to all command pipeline stages.
	enum GPUPipelineStageFlags {
		/// @brief The beginning of the command pipeline.
		GPU_PIPELINE_STAGE_PIPELINE_START = 0x0001,
		/// @brief The pipeline stage in which indirect draw commands are processed.
		GPU_PIPELINE_STAGE_DRAW_INDIRECT = 0x0002,
		/// @brief The pipeline stage in which the vertex and index buffers are consumed.
		GPU_PIPELINE_STAGE_VERTEX_INPUT = 0x0004,
		/// @brief The vertex shader pipeline stage.
		GPU_PIPELINE_STAGE_VERTEX_SHADER = 0x0008,
		/// @brief The fragment shader pipeline stage.
		GPU_PIPELINE_STAGE_FRAGMENT_SHADER = 0x0010,
		/// @brief The pipeline stage in which the final color values are outputted.
		GPU_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT = 0x0020,
		/// @brief The compute shader execution pipeline stage.
		GPU_PIPELINE_STAGE_COMPUTE_SHADER = 0x0040,
		/// @brief The pipeline stage including all transfer commands.
		GPU_PIPELINE_STAGE_TRANSFER = 0x0080,
		/// @brief The end of the command pipeline.
		GPU_PIPELINE_STAGE_PIPELINE_END = 0x0100,
		/// @brief The execution of all graphics pipeline stages.
		GPU_PIPELINE_STAGE_ALL_GRAPHICS = 0x0200,
		/// @brief The execution of all supported commands.
		GPU_PIPELINE_STAGE_ALL_COMMANDS = 0x0400
	};
	/// @brief A type which contains one or more flags corresponding to command pipeline stages.
	typedef uint32_t GPUPipelineStage;

	/// @brief An union containing the clear values for a color image.
	union GPUColorImageClearValue {
		/// @brief The color clear values used with a floating point format.
		float32_t clearFloat[4];
		/// @brief The color clear values used with a signed integer format.
		int32_t clearInt[4];
		/// @brief The color clear values used with an unsigned integer or sRGB format.
		uint32_t clearUint[4];
	};

	/// @brief A struct describing the source and destination regions of a buffer copy operation.
	struct GPUBufferCopyRegion {
		/// @brief The copy region's offset in the source buffer.
		uint64_t srcOffset;
		/// @brief The copy region's offset in the destination buffer.
		uint64_t dstOffset;
		/// @brief The copy region's size.
		uint64_t size;
	};
	/// @brief A struct describing the source and destination regions of an image copy operation.
	struct GPUImageCopyRegion {
		/// @brief The copy region's offset in the source image.
		struct {
			/// @brief The X coordinate of the copy region's offset in the source image.
			uint32_t x;
			/// @brief The Y coordinate of the copy region's offset in the source image.
			uint32_t y;
			/// @brief The Z coordinate of the copy region's offset in the source image.
			uint32_t z;
		} srcOffset;
		/// @brief The copy region's offset in the destination image.
		struct {
			/// @brief The X coordinate of the copy region's offset in the destination image.
			uint32_t x;
			/// @brief The Y coordinate of the copy region's offset in the destination image.
			uint32_t y;
			/// @brief The Z coordinate of the copy region's offset in the destination image.
			uint32_t z;
		} dstOffset;
		/// @brief The copy region's size.
		struct {
			/// @brief The copy region's width.
			uint32_t width;
			/// @brief The copy region's height.
			uint32_t height;
			/// @brief The copy region's depth.
			uint32_t depth;
		} size;
	};
	/// @brief A struct describing the source and destination regions of a buffer to image or image to buffer copy operation.
	struct GPUBufferImageCopyRegion {
		/// @brief The copy region's offset in the buffer.
		uint64_t bufferOffset;
		/// @brief The copy region's offset in the image.
		struct {
			/// @brief The X coordinate of the copy region's offset in the image.
			uint32_t x;
			/// @brief The Y coordinate of the copy region's offset in the image.
			uint32_t y;
			/// @brief The Z coordinate of the copy region's offset in the image.
			uint32_t z;
		} imageOffset;
		/// @brief The copy region's size.
		struct {
			/// @brief The copy region's width.
			uint32_t width;
			/// @brief The copy region's height.
			uint32_t height;
			/// @brief The copy region's depth.
			uint32_t depth;
		} size;
	};
}