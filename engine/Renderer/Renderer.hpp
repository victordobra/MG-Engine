#pragma once

#include <Core.hpp>
#include "Platform/Window.hpp"

namespace wfe {
	/// @brief An abstraction for the renderer's backend API.
	class Renderer {
	public:
		/// @brief A class that represents an exception caused by an unsupported renderer backend API.
		class UnsupportedAPIException : public Exception {
		public:
			UnsupportedAPIException() = delete;
			/// @brief Copies the given exception.
			/// @param other The exception to copy.
			UnsupportedAPIException(const UnsupportedAPIException& other) = default;
			/// @brief Moves the given exception's contents.
			/// @param other The exception to move.
			UnsupportedAPIException(UnsupportedAPIException&& other) noexcept = default;
			/// @brief Creates an exception with the given formatted message.
			/// @param format The format to use for the message.
			explicit UnsupportedAPIException(const char_t* format, ...) : Exception("") {
				// Get the va list
				va_list args;
				va_start(args, format);

				// Format the message
				FormatStringArgs(str, MAX_MESSAGE_LEN, format, args);

				// End the va list
				va_end(args);
			}

			/// @brief Copies the given exception's contents into this exception.
			/// @param other The exception to copy.
			/// @return A reference to this exception.
			UnsupportedAPIException& operator=(const UnsupportedAPIException& other) = default;
			/// @brief Moves the given exception's contents into this exception.
			/// @param other The exception to move.
			/// @return A reference to this exception.
			UnsupportedAPIException& operator=(UnsupportedAPIException&& other) = default;

			/// @brief Destroys the exception.
			~UnsupportedAPIException() = default;
		};

		/// @brief All possible rendering backend APIs.
		typedef enum {
			/// @brief The renderer backend uses the Vulkan API.
			RENDERER_BACKEND_API_VULKAN,
			/// @brief The number of APIs implemented in rendering backends.
			RENDERER_BACKEND_API_COUNT
		} RendererBackendAPI;

	 	/// @brief The maximum number of frames in flight at any moment.
	 	static const size_t MAX_FRAMES_IN_FLIGHT = 2;

		/// @brief Creates a renderer using the most optimal available API.
		/// @param window The window the renderer will display to.
		/// @param debugEnabled True if debugging should be enabled, otherwise false.
		/// @param logger The logger to use for general messages.
		Renderer(Window* window, bool8_t debugEnabled, Logger* logger);
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;

		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		/// @brief Gets the renderer backend's API.
		/// @return The enum value representing the renderer backend's API.
		RendererBackendAPI GetRendererBackendAPI() const {
			return rendererBackendAPI;
		}
		/// @brief Gets the renderer's backend inplementation.
		/// @return A void pointer that can be cast to a pointer to the appropriate renderer backend's class.
		void* GetRendererBackend() {
			return rendererBackend;
		}
		/// @brief Gets the renderer's backend inplementation.
		/// @return A const void pointer that can be cast to a const pointer to the appropriate renderer backend's class.
		const void* GetRendererBackend() const {
			return rendererBackend;
		}

		/// @brief Destroys the renderer.
		~Renderer();
	private:
		RendererBackendAPI rendererBackendAPI;
		void* rendererBackend;
	};
}