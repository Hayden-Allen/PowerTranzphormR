#pragma once

#include "imgui_window.h"

template<typename FB>
class preview_window : public imgui_window
{
public:
	preview_window(const FB& framebuffer);
	virtual ~preview_window();
public:
	virtual void handle_frame() override;
private:
	const FB& m_fb;
};

typedef preview_window<mgl::framebuffer_u8> preview_window_u8;
typedef preview_window<mgl::framebuffer_f32> preview_window_f32;

#include "preview_window.cpp"