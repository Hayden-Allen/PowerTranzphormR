#pragma once
#include "imgui_window.h"
#include "shortcut_menu.h"

struct app_ctx;

class imgui_layer : public mgl::layer
{
public:
	imgui_layer(app_ctx* const a_ctx);
	virtual ~imgui_layer();
public:
	virtual bool on_frame(const f32 dt) override;
	virtual bool on_mouse_button(const s32 button, const s32 action, const s32 mods) override;
	virtual bool on_mouse_move(const f32 x, const f32 y, const f32 dx, const f32 dy) override;
	virtual bool on_scroll(const f32 x, const f32 y) override;
	virtual bool on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods) override;
	virtual bool on_char(const u32 codepoint) override;
	void add_window(imgui_window* window);
	void remove_window(imgui_window* window);
private:
	app_ctx* const m_app_ctx = nullptr;
	std::vector<imgui_window*> m_windows;
	std::function<void()> m_exit_callback;
	f32 m_prev_x_scale = -1.0f, m_prev_y_scale = -1.0f;
private:
	void draw_menus();
	void draw_menu(const shortcut_menu& menu);
	void draw_menu_item(const shortcut_menu_item& item);
};