#pragma once

#include "imgui_window.h"

class preview_window : public imgui_window {
public:
	preview_window(const mgl::framebuffer_u8 &framebuffer);
	virtual ~preview_window();

	void handle_frame();

private:
	const mgl::framebuffer_u8& m_fb;
};
