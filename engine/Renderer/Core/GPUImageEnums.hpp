#pragma once

#include <Core.hpp>

namespace wfe {
	/// @brief An enum describing all supported GPU image types.
	enum GPUImageType {
		/// @brief The type of a one-dimensional image.
		GPU_IMAGE_TYPE_1D,
		/// @brief The type of a two-dimensional image.
		GPU_IMAGE_TYPE_2D,
		/// @brief The type of a three-dimensional image.
		GPU_IMAGE_TYPE_3D
	};
	/// @brief An enum describing all supported GPU image formats.
	enum GPUImageFormat {
		/// @brief A one-component, 8-bit unsigned normalized format.
		GPU_IMAGE_FORMAT_R8_UNORM,
		/// @brief A one-component, 8-bit signed normalized format.
		GPU_IMAGE_FORMAT_R8_SNORM,
		/// @brief A one-component, 8-bit unsigned scaled integer format.
		GPU_IMAGE_FORMAT_R8_USCALED,
		/// @brief A one-component, 8-bit signed scaled integer format.
		GPU_IMAGE_FORMAT_R8_SSCALED,
		/// @brief A one-component, 8-bit unsigned integer format.
		GPU_IMAGE_FORMAT_R8_UINT,
		/// @brief A one-component, 8-bit signed integer format.
		GPU_IMAGE_FORMAT_R8_SINT,
		/// @brief A one-component, 8-bit unsigned normalized format stored with sRGB nonlinear encoding.
		GPU_IMAGE_FORMAT_R8_SRGB,
		/// @brief A two-component, 16-bit unsigned normalized format.
		GPU_IMAGE_FORMAT_R8G8_UNORM,
		/// @brief A two-component, 16-bit signed normalized format.
		GPU_IMAGE_FORMAT_R8G8_SNORM,
		/// @brief A two-component, 16-bit unsigned scaled integer format.
		GPU_IMAGE_FORMAT_R8G8_USCALED,
		/// @brief A two-component, 16-bit signed scaled integer format.
		GPU_IMAGE_FORMAT_R8G8_SSCALED,
		/// @brief A two-component, 16-bit unsigned integer format.
		GPU_IMAGE_FORMAT_R8G8_UINT,
		/// @brief A two-component, 16-bit signed integer format.
		GPU_IMAGE_FORMAT_R8G8_SINT,
		/// @brief A two-component, 16-bit unsigned normalized format stored with sRGB nonlinear encoding.
		GPU_IMAGE_FORMAT_R8G8_SRGB,
		/// @brief A three-component, 24-bit unsigned normalized format.
		GPU_IMAGE_FORMAT_R8G8B8_UNORM,
		/// @brief A three-component, 24-bit signed normalized format.
		GPU_IMAGE_FORMAT_R8G8B8_SNORM,
		/// @brief A three-component, 24-bit unsigned scaled integer format.
		GPU_IMAGE_FORMAT_R8G8B8_USCALED,
		/// @brief A three-component, 24-bit signed scaled integer format.
		GPU_IMAGE_FORMAT_R8G8B8_SSCALED,
		/// @brief A three-component, 24-bit unsigned integer format.
		GPU_IMAGE_FORMAT_R8G8B8_UINT,
		/// @brief A three-component, 24-bit signed integer format.
		GPU_IMAGE_FORMAT_R8G8B8_SINT,
		/// @brief A three-component, 24-bit unsigned normalized format stored with sRGB nonlinear encoding.
		GPU_IMAGE_FORMAT_R8G8B8_SRGB,
		/// @brief A four-component, 32-bit unsigned normalized format.
		GPU_IMAGE_FORMAT_R8G8B8A8_UNORM,
		/// @brief A four-component, 32-bit signed normalized format.
		GPU_IMAGE_FORMAT_R8G8B8A8_SNORM,
		/// @brief A four-component, 32-bit unsigned scaled integer format.
		GPU_IMAGE_FORMAT_R8G8B8A8_USCALED,
		/// @brief A four-component, 32-bit signed scaled integer format.
		GPU_IMAGE_FORMAT_R8G8B8A8_SSCALED,
		/// @brief A four-component, 32-bit unsigned integer format.
		GPU_IMAGE_FORMAT_R8G8B8A8_UINT,
		/// @brief A four-component, 32-bit signed integer format.
		GPU_IMAGE_FORMAT_R8G8B8A8_SINT,
		/// @brief A four-component, 32-bit unsigned normalized format stored with sRGB nonlinear encoding for the red, green and blue components and linear encoding for the alpha component.
		GPU_IMAGE_FORMAT_R8G8B8A8_SRGB,
		/// @brief A one-component, 16-bit unsigned normalized format.
		GPU_IMAGE_FORMAT_R16_UNORM,
		/// @brief A one-component, 16-bit signed normalized format.
		GPU_IMAGE_FORMAT_R16_SNORM,
		/// @brief A one-component, 16-bit unsigned scaled integer format.
		GPU_IMAGE_FORMAT_R16_USCALED,
		/// @brief A one-component, 16-bit signed scaled integer format.
		GPU_IMAGE_FORMAT_R16_SSCALED,
		/// @brief A one-component, 16-bit unsigned integer format.
		GPU_IMAGE_FORMAT_R16_UINT,
		/// @brief A one-component, 16-bit signed integer format.
		GPU_IMAGE_FORMAT_R16_SINT,
		/// @brief A one-component, 16-bit signed float format.
		GPU_IMAGE_FORMAT_R16_SFLOAT,
		/// @brief A two-component, 32-bit unsigned normalized format.
		GPU_IMAGE_FORMAT_R16G16_UNORM,
		/// @brief A two-component, 32-bit signed normalized format.
		GPU_IMAGE_FORMAT_R16G16_SNORM,
		/// @brief A two-component, 32-bit unsigned scaled integer format.
		GPU_IMAGE_FORMAT_R16G16_USCALED,
		/// @brief A two-component, 32-bit signed scaled integer format.
		GPU_IMAGE_FORMAT_R16G16_SSCALED,
		/// @brief A two-component, 32-bit unsigned integer format.
		GPU_IMAGE_FORMAT_R16G16_UINT,
		/// @brief A two-component, 32-bit signed integer format.
		GPU_IMAGE_FORMAT_R16G16_SINT,
		/// @brief A two-component, 32-bit signed float format.
		GPU_IMAGE_FORMAT_R16G16_SFLOAT,
		/// @brief A three-component, 48-bit unsigned normalized format.
		GPU_IMAGE_FORMAT_R16G16B16_UNORM,
		/// @brief A three-component, 48-bit signed normalized format.
		GPU_IMAGE_FORMAT_R16G16B16_SNORM,
		/// @brief A three-component, 48-bit unsigned scaled integer format.
		GPU_IMAGE_FORMAT_R16G16B16_USCALED,
		/// @brief A three-component, 48-bit signed scaled integer format.
		GPU_IMAGE_FORMAT_R16G16B16_SSCALED,
		/// @brief A three-component, 48-bit unsigned integer format.
		GPU_IMAGE_FORMAT_R16G16B16_UINT,
		/// @brief A three-component, 48-bit signed integer format.
		GPU_IMAGE_FORMAT_R16G16B16_SINT,
		/// @brief A three-component, 48-bit signed float format.
		GPU_IMAGE_FORMAT_R16G16B16_SFLOAT,
		/// @brief A four-component, 64-bit unsigned normalized format.
		GPU_IMAGE_FORMAT_R16G16B16A16_UNORM,
		/// @brief A four-component, 64-bit signed normalized format.
		GPU_IMAGE_FORMAT_R16G16B16A16_SNORM,
		/// @brief A four-component, 64-bit unsigned scaled integer format.
		GPU_IMAGE_FORMAT_R16G16B16A16_USCALED,
		/// @brief A four-component, 64-bit signed scaled integer format.
		GPU_IMAGE_FORMAT_R16G16B16A16_SSCALED,
		/// @brief A four-component, 64-bit unsigned integer format.
		GPU_IMAGE_FORMAT_R16G16B16A16_UINT,
		/// @brief A four-component, 64-bit signed integer format.
		GPU_IMAGE_FORMAT_R16G16B16A16_SINT,
		/// @brief A four-component, 64-bit signed float format.
		GPU_IMAGE_FORMAT_R16G16B16A16_SFLOAT,
		/// @brief A one-component, 32-bit unsigned integer format.
		GPU_IMAGE_FORMAT_R32_UINT,
		/// @brief A one-component, 32-bit signed integer format.
		GPU_IMAGE_FORMAT_R32_SINT,
		/// @brief A one-component, 32-bit signed float format.
		GPU_IMAGE_FORMAT_R32_SFLOAT,
		/// @brief A two-component, 64-bit unsigned integer format.
		GPU_IMAGE_FORMAT_R32G32_UINT,
		/// @brief A two-component, 64-bit signed integer format.
		GPU_IMAGE_FORMAT_R32G32_SINT,
		/// @brief A two-component, 64-bit signed float format.
		GPU_IMAGE_FORMAT_R32G32_SFLOAT,
		/// @brief A three-component, 96-bit unsigned integer format.
		GPU_IMAGE_FORMAT_R32G32B32_UINT,
		/// @brief A three-component, 96-bit signed integer format.
		GPU_IMAGE_FORMAT_R32G32B32_SINT,
		/// @brief A three-component, 96-bit signed float format.
		GPU_IMAGE_FORMAT_R32G32B32_SFLOAT,
		/// @brief A four-component, 128-bit unsigned integer format.
		GPU_IMAGE_FORMAT_R32G32B32A32_UINT,
		/// @brief A four-component, 128-bit signed integer format.
		GPU_IMAGE_FORMAT_R32G32B32A32_SINT,
		/// @brief A four-component, 128-bit signed float format.
		GPU_IMAGE_FORMAT_R32G32B32A32_SFLOAT
	};
}