#pragma once
#include "imgui_menu.h"
#include "imgui_window.h"
#include "action_stack.h"

class imgui_layer : public mgl::layer
{
public:
	imgui_layer(const mgl::context* const mgl_context, scene_ctx* const scene);
	virtual ~imgui_layer();
public:
	virtual void on_frame(const f32 dt) override;
	virtual void on_mouse_button(const s32 button, const s32 action, const s32 mods) override;
	virtual void on_mouse_move(const f32 x, const f32 y, const f32 dx, const f32 dy) override;
	virtual void on_scroll(const f32 x, const f32 y) override;
	virtual void on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods) override;
	void add_window(imgui_window* window);
	void remove_window(imgui_window* window);
	void transform_action(sgnode* const t, const tmat<space::OBJECT, space::WORLD>& old_mat, const tmat<space::OBJECT, space::WORLD>& new_mat)
	{
		m_actions.transform(t, old_mat, new_mat);
	}
	void reparent_action(sgnode* const target, sgnode* const old_parent, sgnode* const new_parent)
	{
		m_actions.reparent(target, old_parent, new_parent);
	}
	void create_action(sgnode* const target, sgnode* const parent)
	{
		m_actions.create(target, parent);
	}
	void destroy_action(sgnode* const target)
	{
		m_actions.destroy(target);
	}
	void undo()
	{
		m_actions.undo();
	}
	void redo()
	{
		m_actions.redo();
	}
private:
	const mgl::context* m_mgl_context;
	std::vector<imgui_window*> m_windows;
	std::vector<imgui_menu> m_menus;
	std::function<void()> m_exit_callback;
	scene_ctx* m_scene;
	action_stack m_actions;
private:
	void draw_menus();
	void draw_menu(const imgui_menu& menu);
	void init_menus();
	void handle_menu_keys(const s32 key, const s32 mods);
};