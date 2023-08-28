#include "pch.h"
#include "app_ctx.h"
#include "geom/generated_mesh.h"

app_ctx::app_ctx() :
	mgl_ctx(1280, 720, "PowerTranzphormR", { .vsync = true, .clear = { .r = 0.25f, .g = 0.25f, .b = 0.25f } }),
	actions(&scene),
	preview_fb(1280, 720)
{
	f32 ar = static_cast<f32>(preview_fb.get_width()) / static_cast<f32>(preview_fb.get_height());
	preview_cam = mgl::camera({ 0, 0, 5 }, 0, 0, 108 / ar, ar, 0.1f, 1000.0f, 5.0f);
	init_menus();
}



void app_ctx::clear()
{
	loaded_filename = "";
	actions.clear();
	scene.clear();
}
bool app_ctx::save() const
{
	if (loaded_filename.size())
	{
		save(loaded_filename);
		return true;
	}
	else
	{
		return save_as();
	}
}
void app_ctx::save(const std::string& fp) const
{
	std::ofstream out(fp);
	assert(out.is_open());
	const auto& cp = preview_cam.get_pos();
	out << cp.x << " " << cp.y << " " << cp.z << "\n";
	actions.save(out, scene.get_sg_root());
	loaded_filename = fp;
}
bool app_ctx::save_as() const
{
	nfdchar_t* nfd_path = nullptr;
	nfdresult_t nfd_res = NFD_SaveDialog("phorm", nullptr, &nfd_path);
	if (nfd_res == NFD_OKAY)
	{
		// TODO probably not correct
		std::string path(nfd_path);
		if (!path.ends_with(".phorm"))
			path.append(".phorm");
		save(path);
		return true;
	}
	else
	{
		return false;
	}
}
bool app_ctx::confirm_unsaved_changes()
{
	if (!actions.get_modified())
	{
		return true; // Safe to continue if no unsaved changes
	}
	s32 res = MessageBox(glfwGetWin32Window(mgl_ctx.window), L"Do you want to save your changes?", L"PowerTranzphormR", MB_YESNOCANCEL | MB_ICONQUESTION);
	if (res == IDYES)
	{
		return save(); // Only continue if save was successful
	}
	else if (res == IDNO)
	{
		return true; // Continue (true) without saving
	}
	return false;
}
void app_ctx::load(const std::string& fp)
{
	clear();
	std::ifstream in(fp);
	assert(in.is_open());
	f32 x, y, z;
	in >> x >> y >> z;
	preview_cam.set_pos(point<space::WORLD>(x, y, z));
	scene.set_sg_root(actions.load(in));
	loaded_filename = fp;
}
void app_ctx::undo()
{
	const sgnode* const selected = scene.get_selected_node();
	assert(selected);
	const action* const a = actions.undo();
	if (a)
	{
		if (a->undo_conflict(selected))
			scene.set_selected_node(scene.get_sg_root());
	}
}
void app_ctx::redo()
{
	const sgnode* const selected = scene.get_selected_node();
	assert(selected);
	const action* const a = actions.redo();
	if (a)
	{
		if (a->redo_conflict(selected))
			scene.set_selected_node(scene.get_sg_root());
	}
}
bool app_ctx::is_node_frozen(sgnode* const node) const
{
	return frozen.contains(node);
}



void app_ctx::transform_action(sgnode* const t, const tmat<space::OBJECT, space::PARENT>& old_mat, const tmat<space::OBJECT, space::PARENT>& new_mat)
{
	actions.transform(t, old_mat, new_mat);
}
void app_ctx::reparent_action(sgnode* const target, sgnode* const new_parent, const s64 new_index)
{
	actions.reparent(target, new_parent, new_index);
}
void app_ctx::create_action(sgnode* const target, sgnode* const parent)
{
	actions.create(target, parent);
}
void app_ctx::destroy_action(sgnode* const target)
{
	actions.destroy(target);
}
void app_ctx::destroy_selected_action()
{
	sgnode* const selected = scene.get_selected_node();
	assert(selected);
	if (selected->parent)
	{
		actions.destroy(selected);
		scene.set_selected_node(scene.get_sg_root());
	}
}
void app_ctx::create_cube_action()
{
	create_shape_action(&scene_ctx::generated_textured_cuboid, "Cube");
}
void app_ctx::create_sphere_action()
{
	create_shape_action(&scene_ctx::generated_textured_ellipsoid, "Sphere");
}
void app_ctx::create_cylinder_action()
{
	create_shape_action(&scene_ctx::generated_textured_cylinder, "Cylinder");
}
void app_ctx::create_cone_action()
{
	create_shape_action(&scene_ctx::generated_textured_cone, "Cone");
}
void app_ctx::create_torus_action()
{
	create_shape_action(&scene_ctx::generated_textured_torus, "Torus");
}
void app_ctx::create_heightmap_action()
{
	assert(false);
	// FIXME: Heightmaps are not sgnodes
	/*
	sgnode* const selected = scene.get_selected_node();
	if (selected)
	{
		mesh_t* m = textured_heightmap(scene.get_tex_coord_attr(), scene.get_mtl_attr(), 1);
		sgnode* n = new sgnode(nullptr, m, "Heightmap");
		create_action(n, selected);
		scene.set_selected_node(n);
	}
	*/
}
void app_ctx::create_union_action()
{
	create_operation_action(carve::csg::CSG::OP::UNION);
}
void app_ctx::create_subtract_action()
{
	create_operation_action(carve::csg::CSG::OP::A_MINUS_B);
}
void app_ctx::create_intersect_action()
{
	create_operation_action(carve::csg::CSG::OP::INTERSECTION);
}
void app_ctx::freeze_action(sgnode* const target)
{
	assert(!target->is_root());
	sgnode* const new_node = actions.freeze(target);
	frozen.insert({ new_node, target });
	scene.set_selected_node(new_node);
}
void app_ctx::unfreeze_action(sgnode* const target)
{
	const auto& it = frozen.find(target);
	assert(it != frozen.end());
	actions.unfreeze(target, it->second);
	scene.set_selected_node(it->second);
	frozen.erase(it);
}



void app_ctx::create_operation_action(const carve::csg::CSG::OP op)
{
	create(new sgnode(scene.get_csg(), nullptr, op));
}
template<typename FN>
void app_ctx::create_shape_action(FN fn, const std::string& name)
{
	generated_mesh* gen = (scene.*fn)(0, {});
	create(new sgnode(nullptr, gen, name));
}
void app_ctx::create(sgnode* const node)
{
	sgnode* const selected = scene.get_selected_node();
	assert(selected);
	if (selected)
	{
		actions.create(node, selected);
	}
}
void app_ctx::init_menus()
{
	shortcut_menu file_menu;
	file_menu.name = "File";
	shortcut_menu_item file_new = {
		"New",
		[&]()
		{
			if (!confirm_unsaved_changes())
			{
				return;
			}
			clear();
		},
		[]()
		{
			return true;
		},
		"Ctrl+N",
		GLFW_KEY_N,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item file_open = {
		"Open",
		[&]()
		{
			if (!confirm_unsaved_changes())
			{
				return;
			}
			nfdchar_t* nfd_path = nullptr;
			nfdresult_t nfd_res = NFD_OpenDialog("phorm", nullptr, &nfd_path);
			if (nfd_res == NFD_OKAY)
			{
				load(std::string(nfd_path));
			}
		},
		[]()
		{
			return true;
		},
		"Ctrl+O",
		GLFW_KEY_O,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item file_save = {
		"Save",
		[&]()
		{
			save();
		},
		[]()
		{
			return true;
		},
		"Ctrl+S",
		GLFW_KEY_S,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item file_save_as = {
		"Save As...",
		[&]()
		{
			save_as();
		},
		[]()
		{
			return true;
		},
		"Ctrl+Shift+S",
		GLFW_KEY_S,
		GLFW_MOD_CONTROL | GLFW_MOD_SHIFT,
	};
	file_menu.groups.push_back({ file_new, file_open, file_save, file_save_as });
	shortcut_menus.push_back(file_menu);

	shortcut_menu edit_menu;
	edit_menu.name = "Edit";
	shortcut_menu_item edit_undo = {
		"Undo",
		[&]()
		{
			undo();
		},
		[&]()
		{
			return actions.can_undo();
		},
		"Ctrl+Z",
		GLFW_KEY_Z,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item edit_redo = {
		"Redo",
		[&]()
		{
			redo();
		},
		[&]()
		{
			return actions.can_redo();
		},
		"Ctrl+Y",
		GLFW_KEY_Y,
		GLFW_MOD_CONTROL,
	};
	edit_menu.groups.push_back({ edit_undo, edit_redo });
	shortcut_menu_item edit_cut = {
		"Cut",
		[&]()
		{
			clipboard_cut = true;
			sgnode* const selected = scene.get_selected_node();
			clipboard = selected->clone(this, nullptr, false);
		},
		[&]()
		{
			sgnode* const selected = scene.get_selected_node();
			return selected->parent;
		},
		"Ctrl+X",
		GLFW_KEY_X,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item edit_copy = {
		"Copy",
		[&]()
		{
			sgnode* const selected = scene.get_selected_node();
			clipboard = selected->clone(this, nullptr, false);
		},
		[&]()
		{
			return true;
		},
		"Ctrl+C",
		GLFW_KEY_C,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item edit_paste = {
		"Paste",
		[&]()
		{
			sgnode* const selected = scene.get_selected_node();
			// selected->add_child(clipboard->clone());
			sgnode* const clone = clipboard->clone(this, selected);
			// create_action(clone, selected);
			if (clipboard_cut)
			{
				destroy_action(clipboard);
				clipboard_cut = false;
			}
		},
		[&]()
		{
			sgnode* const selected = scene.get_selected_node();
			return clipboard && !selected->is_mesh();
		},
		"Ctrl+V",
		GLFW_KEY_V,
		GLFW_MOD_CONTROL,
	};
	edit_menu.groups.push_back({ edit_cut, edit_copy, edit_paste });
	shortcut_menu_item edit_move_up = {
		"Move Up",
		[&]()
		{
			sgnode* const selected = scene.get_selected_node();
			assert(selected);
			const s64 i = selected->get_index();
			reparent_action(selected, selected->parent, i - 1);
		},
		[&]()
		{
			const sgnode* const selected = scene.get_selected_node();
			assert(selected);
			const s64 i = selected->get_index();
			return selected->parent && i > 0;
		},
		"Ctrl+Up",
		GLFW_KEY_UP,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item edit_move_down = {
		"Move Down",
		[&]()
		{
			sgnode* const selected = scene.get_selected_node();
			assert(selected);
			const s64 i = selected->get_index();
			reparent_action(selected, selected->parent, i + 1);
		},
		[&]()
		{
			const sgnode* const selected = scene.get_selected_node();
			assert(selected);
			const s64 i = selected->get_index();
			return selected->parent && i < static_cast<s64>(selected->parent->children.size() - 1);
		},
		"Ctrl+Down",
		GLFW_KEY_DOWN,
		GLFW_MOD_CONTROL,
	};
	edit_menu.groups.push_back({ edit_move_up, edit_move_down });
	shortcut_menu_item gizmodes = {
		"Gizmodes",
		[]() {
		},
		[&]()
		{
			return !mgl_ctx.is_cursor_locked();
		},
		"",
		0,
		0,
	};
	shortcut_menu_item gizmo_translate = {
		"Translate",
		[&]()
		{
			gizmo_op = ImGuizmo::OPERATION::TRANSLATE;
		},
		[&]()
		{
			return true;
		},
		"T",
		GLFW_KEY_T,
		0,
	};
	shortcut_menu_item gizmo_rotate = {
		"Rotate",
		[&]()
		{
			gizmo_op = ImGuizmo::OPERATION::ROTATE;
		},
		[&]()
		{
			return true;
		},
		"R",
		GLFW_KEY_R,
		0,
	};
	shortcut_menu_item gizmo_scale = {
		"Scale",
		[&]()
		{
			gizmo_op = ImGuizmo::OPERATION::SCALE;
		},
		[&]()
		{
			return true;
		},
		"E",
		GLFW_KEY_E,
		0,
	};
	gizmodes.groups.push_back({ gizmo_translate, gizmo_rotate, gizmo_scale });
	edit_menu.groups.push_back({ gizmodes });
	shortcut_menus.push_back(edit_menu);

	shortcut_menu phorm_menu;
	phorm_menu.name = "Phorm";
	shortcut_menu_item phorm_create_cube = {
		"Cube",
		[&]()
		{
			create_cube_action();
		},
		[&]()
		{
			const sgnode* const selected = scene.get_selected_node();
			return selected && !selected->is_mesh();
		},
		"Ctrl+1",
		GLFW_KEY_1,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create_sphere = {
		"Sphere",
		[&]()
		{
			create_sphere_action();
		},
		[&]()
		{
			const sgnode* const selected = scene.get_selected_node();
			return selected && !selected->is_mesh();
		},
		"Ctrl+2",
		GLFW_KEY_2,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create_cylinder = {
		"Cylinder",
		[&]()
		{
			create_cylinder_action();
		},
		[&]()
		{
			const sgnode* const selected = scene.get_selected_node();
			return selected && !selected->is_mesh();
		},
		"Ctrl+3",
		GLFW_KEY_3,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create_cone = {
		"Cone",
		[&]()
		{
			create_cone_action();
		},
		[&]()
		{
			const sgnode* const selected = scene.get_selected_node();
			return selected && !selected->is_mesh();
		},
		"Ctrl+4",
		GLFW_KEY_4,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create_torus = {
		"Torus",
		[&]()
		{
			create_torus_action();
		},
		[&]()
		{
			const sgnode* const selected = scene.get_selected_node();
			return selected && !selected->is_mesh();
		},
		"Ctrl+5",
		GLFW_KEY_5,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create_union = {
		"Union",
		[&]()
		{
			create_union_action();
		},
		[&]()
		{
			const sgnode* const selected = scene.get_selected_node();
			return selected && !selected->is_mesh();
		},
		"Ctrl+=",
		GLFW_KEY_EQUAL,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create_subtract = {
		"Subtract",
		[&]()
		{
			create_subtract_action();
		},
		[&]()
		{
			const sgnode* const selected = scene.get_selected_node();
			return selected && !selected->is_mesh();
		},
		"Ctrl+-",
		GLFW_KEY_MINUS,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create_intersect = {
		"Intersect",
		[&]()
		{
			create_intersect_action();
		},
		[&]()
		{
			const sgnode* const selected = scene.get_selected_node();
			return selected && !selected->is_mesh();
		},
		"Ctrl+8",
		GLFW_KEY_8,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_create = {
		"Create",
		[&]() {},
		[&]()
		{
			const sgnode* const selected = scene.get_selected_node();
			return selected && !selected->is_mesh();
		},
		"",
		0,
		0,
		{
			{ phorm_create_cube, phorm_create_sphere, phorm_create_cylinder, phorm_create_cone, phorm_create_torus },
			{ phorm_create_union, phorm_create_subtract, phorm_create_intersect },
		}
	};
	shortcut_menu_item phorm_destroy = {
		"Destroy",
		[&]()
		{
			if (!mgl_ctx.is_cursor_locked())
			{
				destroy_selected_action();
			}
		},
		[&]()
		{
			return scene.get_selected_node();
		},
		"Delete",
		GLFW_KEY_DELETE,
		0,
	};
	phorm_menu.groups.push_back({ phorm_create, phorm_destroy });
	shortcut_menus.push_back(phorm_menu);
}
