#pragma once

#include "Platform/Window.hpp"
#include "Renderer/Renderer.hpp"
#include "Assets/Asset.hpp"

#include <Core.hpp>

namespace wfe {
	class AssetManager;

	/// @brief A class containing an abstraction for the program and its components.
	class Program {
	public:
		/// @brief Creates the program and its components.
		/// @param argc The number for console arguments given. Defaulted to 0.
		/// @param args The console arguments given, or nullpre if none are present.
		Program(int32_t argc = 0, char_t** args = nullptr);
		Program(const Program&) = delete;
		Program(Program&&) noexcept = delete;

		Program& operator=(const Program&) = delete;
		Program& operator=(Program&&) = delete;

		/// @brief Runs the program.
		/// @return The program's return code.
		int32_t Run();
		/// @brief Closes the program, causing it to stop running after the current update loop.
		/// @param returnCode The program's return code.
		void Close(int32_t returnCode);

		/// @brief Gets the program's logger.
		/// @return A pointer to the program's logger.
		Logger* GetLogger() {
			return logger;
		}
		/// @brief Gets the program's logger.
		/// @return A const pointer to the program's logger.
		const Logger* GetLogger() const {
			return logger;
		}
		/// @brief Gets the program's job manager.
		/// @return A pointer to the program's job manager.
		JobManager* GetJobManager() {
			return jobManager;
		}
		/// @brief Gets the program's job manager.
		/// @return A const pointer to the program's job manager.
		const JobManager* GetJobManager() const {
			return jobManager;
		}
		/// @brief Gets the program's window.
		/// @return A pointer to the program's window.
		Window* GetWindow() {
			return window;
		}
		/// @brief Gets the program's window.
		/// @return A pointer to the program's window.
		const Window* GetWindow() const {
			return window;
		}
		/// @brief Gets the program's renderer.
		/// @return A pointer to the program's renderer.
		Renderer* GetRenderer() {
			return renderer;
		}
		/// @brief Gets the program's renderer.
		/// @return A const pointer to the program's renderer.
		const Renderer* GetRenderer() const {
			return renderer;
		}
		/// @brief Gets the program's asset manager.
		/// @return A pointer to the program's asset manager.
		AssetManager* GetAssetManager() {
			return assetManager;
		}
		/// @brief Gets the program's asset manager.
		/// @return A const pointer to the program's asset manager.
		const AssetManager* GetAssetManager() const {
			return assetManager;
		}

		/// @brief Destroys the program and its components.
		~Program();
	private:
		atomic_int32_t running;
		atomic_int32_t returnCode;

		Logger* logger;
		JobManager* jobManager;
		Window* window;
		Renderer* renderer;
		AssetManager* assetManager;
	};
}