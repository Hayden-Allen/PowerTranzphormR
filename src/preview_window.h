#pragma once

#include "imgui_window.h"

template<typename FB>
class preview_window : public imgui_window
{
public:
	preview_window(const mgl::context &mgl_context, const FB& framebuffer);
	virtual ~preview_window();
	void set_enter_preview_callback(const std::function<void()>& callback);
public:
	virtual void handle_frame() override;
private:
	const mgl::context& m_mgl_context;
	const FB& m_fb;
	std::function<void()> m_enter_preview_callback;
};

typedef preview_window<mgl::framebuffer_u8> preview_window_u8;
typedef preview_window<mgl::framebuffer_f32> preview_window_f32;

#include "preview_window.cpp"