#pragma once

#include <mingl/mingl.h>
#include "scene_ctx.h"
#include "action_stack.h"
#include "shortcut_menu.h"

struct app_ctx {
	scene_ctx scene;
	action_stack actions;
	mgl::context mgl_ctx;
	mgl::framebuffer_u8 preview_fb;
	mgl::camera preview_cam;
	ImGuizmo::OPERATION gizmo_op = ImGuizmo::OPERATION::TRANSLATE;
	std::vector<shortcut_menu> shortcut_menus;
	app_ctx() :
		actions(&scene),
		mgl_ctx(1280, 720, "PowerTranzphormR", { .vsync = true, .clear = { .b = 1.f } }),
		preview_fb(1280, 720)
	{
		f32 ar = static_cast<f32>(preview_fb.get_width()) / static_cast<f32>(preview_fb.get_height());
		preview_cam = mgl::camera({ 0, 0, 5 }, 0, 0, 108 / ar, ar, 0.1f, 1000.0f, 5.0f);
		m_init_menus();
	}
	void transform_action(sgnode* const t, const tmat<space::OBJECT, space::WORLD>& old_mat, const tmat<space::OBJECT, space::WORLD>& new_mat)
	{
		actions.transform(t, old_mat, new_mat);
	}
	void reparent_action(sgnode* const target, sgnode* const old_parent, sgnode* const new_parent)
	{
		actions.reparent(target, old_parent, new_parent);
	}
	void create_action(sgnode* const target, sgnode* const parent)
	{
		actions.create(target, parent);
	}
	void destroy_action(sgnode* const target)
	{
		actions.destroy(target);
	}
	void undo()
	{
		actions.undo();
	}
	void redo()
	{
		actions.redo();
	}
private:
	void m_init_menus()
	{
		shortcut_menu file_menu;
		file_menu.name = "File";
		shortcut_menu_item file_new = {
			"New Phonky Phorm",
			[]()
			{
				std::cout << "MENU: NEW\n";
			},
			"Ctrl+N",
			GLFW_KEY_N,
			GLFW_MOD_CONTROL,
		};
		shortcut_menu_item file_open = {
			"Open Phonky Phorm",
			[]()
			{
				std::cout << "MENU: OPEN\n";
			},
			"Ctrl+O",
			GLFW_KEY_O,
			GLFW_MOD_CONTROL,
		};
		shortcut_menu_item file_save = {
			"Save Phonky Phorm",
			[]()
			{
				std::cout << "MENU: SAVE\n";
			},
			"Ctrl+S",
			GLFW_KEY_S,
			GLFW_MOD_CONTROL,
		};
		shortcut_menu_item file_save_as = {
			"Save Phonky Phorm As...",
			[]()
			{
				std::cout << "MENU: SAVE AS\n";
			},
			"Ctrl+Shift+S",
			GLFW_KEY_S,
			GLFW_MOD_CONTROL | GLFW_MOD_SHIFT,
		};
		file_menu.groups.push_back({ file_new, file_open, file_save, file_save_as });
		shortcut_menus.push_back(file_menu);

		shortcut_menu edit_menu;
		edit_menu.name = "Edit";
		shortcut_menu_item edit_delete = {
			"Delete Node",
			[&]()
			{
				sgnode* const selected = scene.get_selected_node();
				if (selected && selected->parent)
				{
					destroy_action(selected);
					scene.set_selected_node(nullptr);
				}
			},
			"Delete",
			GLFW_KEY_DELETE,
			0,
		};
		edit_menu.groups.push_back({ edit_delete });
		shortcut_menu_item edit_undo = {
			"Undo Tranzphormation",
			[&]()
			{
				undo();
			},
			"Ctrl+Z",
			GLFW_KEY_Z,
			GLFW_MOD_CONTROL,
		};
		shortcut_menu_item edit_redo = {
			"Redo Tranzphormation",
			[&]()
			{
				redo();
			},
			"Ctrl+Y",
			GLFW_KEY_Y,
			GLFW_MOD_CONTROL,
		};
		edit_menu.groups.push_back({ edit_undo, edit_redo });
		shortcut_menu_item edit_translate = {
			"Gizmo: Translate",
			[&]()
			{
				if (!mgl_ctx.is_cursor_locked())
				{
					gizmo_op = ImGuizmo::OPERATION::TRANSLATE;
				}
			},
			"T",
			GLFW_KEY_T,
			0,
		};
		shortcut_menu_item edit_rotate = {
			"Gizmo: Rotate",
			[&]()
			{
				if (!mgl_ctx.is_cursor_locked())
				{
					gizmo_op = ImGuizmo::OPERATION::ROTATE;
				}
			},
			"R",
			GLFW_KEY_R,
			0,
		};
		shortcut_menu_item edit_scale = {
			"Gizmo: Scale",
			[&]()
			{
				if (!mgl_ctx.is_cursor_locked())
				{
					gizmo_op = ImGuizmo::OPERATION::SCALE;
				}
			},
			"S",
			GLFW_KEY_S,
			0,
		};
		edit_menu.groups.push_back({ edit_translate, edit_rotate, edit_scale });
		shortcut_menus.push_back(edit_menu);
	}
};
