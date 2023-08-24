#pragma once
#include "imgui_menu.h"
#include "imgui_window.h"
#include "app_ctx.h"

class imgui_layer : public mgl::layer
{
public:
	imgui_layer(app_ctx* const a_ctx);
	virtual ~imgui_layer();
public:
	virtual void on_frame(const f32 dt) override;
	virtual void on_mouse_button(const s32 button, const s32 action, const s32 mods) override;
	virtual void on_mouse_move(const f32 x, const f32 y, const f32 dx, const f32 dy) override;
	virtual void on_scroll(const f32 x, const f32 y) override;
	virtual void on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods) override;
	void add_window(imgui_window* window);
	void remove_window(imgui_window* window);
private:
	app_ctx* const m_app_ctx = nullptr;
	std::vector<imgui_window*> m_windows;
	std::vector<imgui_menu> m_menus;
	std::function<void()> m_exit_callback;
private:
	void draw_menus();
	void draw_menu(const imgui_menu& menu);
	void init_menus();
	void handle_menu_keys(const s32 key, const s32 mods);
};