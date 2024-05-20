#include "Program.hpp"
#include "ProjectInfo.hpp"
#include <unistd.h>

namespace wfe {
	// Window close callback
	static void* WindowCloseEventCallback(void* args, void* userData) {
		// Close the current program
		Program* program = (Program*)userData;
		program->Close(0);

		return nullptr;
	}

	// Public functions
	Program::Program(int32_t argc, char_t** args) : running(1), returnCode(0) {
		// Create the logger
		logger = NewObject<Logger>("log.txt", false);

		// Create the job manager
		jobManager = NewObject<JobManager>();

		// Create the window
		Window::WindowInfo windowInfo {
			.name = WFE_PROJECT_NAME,
			.posX = 64,
			.posY = 64,
			.width = 512,
			.height = 512,
			.isMaximized = true,
			.isMinimized = false
		};
		window = NewObject<Window>(windowInfo);

		// Create the renderer
		renderer = NewObject<Renderer>(window, true, logger);

		// Create the asset manager and load the main asset dir
		assetManager = NewObject<AssetManager>("assets/", this);
		assetManager->LoadAssets("main/");

		// Add the window close event callback
		window->GetCloseEvent().AddListener(Event::Listener(WindowCloseEventCallback, this));
	}

	int32_t Program::Run() {
		// Keep the update loop running until the running bool is reset
		while(running) {
			// Poll the window's events
			window->PollEvents();

			sleep(0);
		}

		return returnCode;
	}
	void Program::Close(int32_t returnCode) {
		// Set the running bool and the return code
		running = 0;
		returnCode = 0;
	}

	Program::~Program() {
		// Remove the window close event callback
		window->GetCloseEvent().RemoveListener(Event::Listener(WindowCloseEventCallback, this));

		// Destroy all child objects
		DestroyObject(assetManager);
		DestroyObject(renderer);
		DestroyObject(window);
		DestroyObject(jobManager);
		DestroyObject(logger);
	}
}