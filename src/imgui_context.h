#pragma once

#include "imgui_window.h"

class imgui_context : public mgl::layer {
public:
	imgui_context(const mgl::context& mgl_context);
	virtual ~imgui_context();

	void on_frame(const f32 dt);

	void add_window(imgui_window* window);
	void remove_window(imgui_window* window);

	void on_window_resize(const s32 width, const s32 height);
	void on_mouse_button(const s32 button, const s32 action, const s32 mods);
	void on_mouse_move(const f32 x, const f32 y, const f32 dx, const f32 dy);
	void on_scroll(const f32 x, const f32 y);
	void on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods);

private:
	const mgl::context& m_mgl_context;
	std::vector<imgui_window*> m_windows;
};
