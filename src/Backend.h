#ifndef RMLUI_BACKENDS_BACKEND_H
#define RMLUI_BACKENDS_BACKEND_H

// #include "RmlUi_Platform_Win32.h"
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/RenderInterface.h>
// #include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/Types.h>

#include "RmlUi/Core/SystemInterface.h"
#include "renderer/Renderer.h"
#include <RmlUi_Platform_Win32.h>

struct BackendData {
	// SystemInterface_Win32 system_interface;
	RenderInterface_GD render_interface;

    GLuint rmluiFbo;
    GLuint gdFbo;

    // geode::Ref<cocos2d::CCGLProgram> shader;

	bool context_dimensions_dirty = true;
	Rml::Vector2i window_dimensions;
	bool running = true;
};

// this global is probably bad if i want more than 1 window maybe idk
// or if i wanted threads
static Rml::UniquePtr<BackendData> data;


namespace Backend {

// Initializes the backend, including the custom system and render interfaces, and opens a window for rendering the RmlUi context.
void Initialize(float, float);
// Closes the window and release all resources owned by the backend, including the system and render interfaces.
void Shutdown();

// Returns a pointer to the custom system interface which should be provided to RmlUi.
// Rml::SystemInterface* GetSystemInterface();
// Returns a pointer to the custom render interface which should be provided to RmlUi.
RenderInterface_GD* GetRenderInterface();

// Polls and processes events from the current platform, and applies any relevant events to the provided RmlUi context and the key down callback.
// @return False to indicate that the application should be closed.
// bool ProcessEvents(Rml::Context* context, KeyDownCallback key_down_callback = nullptr, bool power_save = false);
// Request application closure during the next event processing call.
// void RequestExit();

// Prepares the render state to accept rendering commands from RmlUi, call before rendering the RmlUi context.
void BeginFrame();
// Presents the rendered frame to the screen, call after rendering the RmlUi context.
void PresentFrame();

} // namespace Backend

#endif
