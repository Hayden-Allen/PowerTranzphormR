#include "pch.h"
#include "app_ctx.h"
#include "geom/generated_mesh.h"
#include "sgnode.h"
#include "scene_material.h"
#include "action.h"
#include "scene_graph_window.h"

app_ctx::app_ctx() :
	mgl_ctx(1280, 720, "PowerTranzphormR", { .vsync = true, .clear = { .r = 0.25f, .g = 0.25f, .b = 0.25f } }),
	actions(&scene, this),
	preview_fb(1280, 720)
{
	NFD_Init(); // Should happen after GLFW initialized
	f32 ar = static_cast<f32>(preview_fb.get_width()) / static_cast<f32>(preview_fb.get_height());
	preview_cam = mgl::camera({ 0, 0, 5 }, 0, 0, 108 / ar, ar, 0.1f, 1000.0f, 5.0f);
	init_menus();
}



void app_ctx::clear()
{
	loaded_filename = "";
	actions.clear();
	scene.clear();
	NFD_Quit(); // Should happen before GLFW destroyed
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
	loaded_filename = fp;

	actions.save(out, scene.get_sg_root());

	out << frozen2unfrozen.size();
	for (const auto& pair : frozen2unfrozen)
	{
		out << pair.first->get_id() << " " << pair.second->get_id() << "\n";
	}
}
bool app_ctx::save_as() const
{
	nfdchar_t* nfd_path = nullptr;
	nfdfilteritem_t nfd_filters[1] = { { "PowerTranzphormR Scene", "phorm" } };
	nfdresult_t nfd_res = NFD_SaveDialog(&nfd_path, nfd_filters, 1, nullptr, nullptr);
	if (nfd_res == NFD_OKAY)
	{
		save(std::string(nfd_path));
		NFD_FreePath(nfd_path);
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
	loaded_filename = fp;

	const auto& nodes = actions.load(in);
	// should always be the id of the root
	assert(nodes.contains("sgn0"));
	scene.set_sg_root(nodes.at("sgn0"));

	u64 f2u_count;
	in >> f2u_count;
	frozen2unfrozen.reserve(f2u_count);
	for (u64 i = 0; i < f2u_count; i++)
	{
		std::string fid = "", uid = "";
		in >> fid >> uid;
		assert(nodes.contains(fid) && nodes.contains(uid));
		frozen2unfrozen.insert({ nodes.at(fid), nodes.at(uid) });
	}
}
void app_ctx::undo()
{
	const sgnode* const selected = get_selected_sgnode();
	const action* const a = actions.undo();
	if (a)
	{
		if (selected && a->undo_conflict(selected))
			set_selected_sgnode(nullptr);
	}
}
void app_ctx::redo()
{
	const sgnode* const selected = get_selected_sgnode();
	const action* const a = actions.redo();
	if (a)
	{
		if (selected && a->redo_conflict(selected))
			set_selected_sgnode(nullptr);
	}
}
bool app_ctx::has_unfrozen(const sgnode* const node) const
{
	return frozen2unfrozen.contains(node);
}
void app_ctx::set_selected_sgnode(sgnode* const node)
{
	if (m_selected_sgnode != node)
	{
		m_selected_sgnode = node;
		m_imgui_needs_select_unfocused_sgnode = m_selected_sgnode;
	}
}
sgnode* app_ctx::get_selected_sgnode()
{
	return sel_type == global_selection_type::sgnode ? m_selected_sgnode : nullptr;
}
sgnode* app_ctx::get_imgui_needs_select_unfocused_sgnode()
{
	return m_imgui_needs_select_unfocused_sgnode;
}
void app_ctx::unset_imgui_needs_select_unfocused_sgnode()
{
	m_imgui_needs_select_unfocused_sgnode = nullptr;
}
void app_ctx::set_selected_material(scene_material* const mtl)
{
	if (m_selected_mtl != mtl)
	{
		m_selected_mtl = mtl;
		m_imgui_needs_select_unfocused_mtl = m_selected_mtl;
	}
}
scene_material* app_ctx::get_selected_material()
{
	return sel_type == global_selection_type::material ? m_selected_mtl : nullptr;
}
scene_material* app_ctx::get_imgui_needs_select_unfocused_mtl()
{
	return m_imgui_needs_select_unfocused_mtl;
}
void app_ctx::unset_imgui_needs_select_unfocused_mtl()
{
	m_imgui_needs_select_unfocused_mtl = nullptr;
}
void app_ctx::set_sg_window(scene_graph_window* const window)
{
	assert(!m_sg_window);
	m_sg_window = window;
}
std::vector<std::pair<u32, scene_material*>> app_ctx::get_sorted_materials()
{
	const auto& unordered_mtls = scene.get_materials();
	std::vector<std::pair<u32, scene_material*>> sorted_mtls;
	for (const auto& i : unordered_mtls)
	{
		if (i.first != 0)
		{
			sorted_mtls.emplace_back(std::make_pair(i.first, i.second));
		}
	}
	std::sort(sorted_mtls.begin(), sorted_mtls.end(), [](const auto& a, const auto& b)
		{
			return a.second->name < b.second->name;
		});
	sorted_mtls.emplace(sorted_mtls.begin(), std::make_pair(0, unordered_mtls.at(0)));
	return sorted_mtls;
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
	sgnode* const selected = get_selected_sgnode();
	assert(selected && selected->get_parent());
	actions.destroy(selected);
	set_selected_sgnode(nullptr);
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
	sgnode* const selected = get_selected_sgnode();
	asset(selected);
	mesh_t* m = textured_heightmap(scene.get_tex_coord_attr(), scene.get_mtl_attr(), 1);
	sgnode* n = new sgnode(nullptr, m, "Heightmap");
	create_action(n, selected);
	set_selected_sgnode(n);
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
	frozen2unfrozen.insert({ new_node, target });
}
void app_ctx::unfreeze_action(sgnode* const target)
{
	const auto& it = frozen2unfrozen.find(target);
	assert(it != frozen2unfrozen.end());
	actions.unfreeze(target, it->second);
	frozen2unfrozen.erase(it);
}
void app_ctx::rename_action(sgnode* const target, const std::string& new_name)
{
	actions.rename(target, new_name);
}



void app_ctx::create_operation_action(const carve::csg::CSG::OP op)
{
	create(new sgnode(nullptr, op));
}
template<typename FN>
void app_ctx::create_shape_action(FN fn, const std::string& name)
{
	generated_mesh* gen = (scene.*fn)(0, {});
	create(new sgnode(nullptr, gen, name));
}
void app_ctx::create(sgnode* const node)
{
	sgnode* const selected = get_selected_sgnode();
	assert(selected);
	if (selected->is_operation())
	{
		actions.create(node, selected);
	}
	else
	{
		assert(!selected->is_root());
		actions.create(node, selected->get_parent());
	}
	set_selected_sgnode(node);
}
void app_ctx::init_menus()
{
	file_menu();
	phorm_menu();
	material_menu();
}
void app_ctx::file_menu()
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
			nfdfilteritem_t nfd_filters[1] = { { "PowerTranzphormR Scene", "phorm" } };
			nfdresult_t nfd_res = NFD_OpenDialog(&nfd_path, nfd_filters, 1, nullptr);
			if (nfd_res == NFD_OKAY)
			{
				load(std::string(nfd_path));
				NFD_FreePath(nfd_path);
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
}
void app_ctx::phorm_menu()
{
	shortcut_menu_item phorm_create_cube = {
		"Cube",
		[&]()
		{
			create_cube_action();
		},
		[&]()
		{
			return true;
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
			return true;
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
			return true;
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
			return true;
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
			return true;
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
			return true;
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
			return true;
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
			return true;
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
			return get_selected_sgnode();
		},
		"",
		0,
		0,
		{
			{ phorm_create_cube, phorm_create_sphere, phorm_create_cylinder, phorm_create_cone, phorm_create_torus },
			{ phorm_create_union, phorm_create_subtract, phorm_create_intersect },
		}
	};

	shortcut_menu_item phorm_undo = {
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
	shortcut_menu_item phorm_redo = {
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

	shortcut_menu_item phorm_cut = {
		"Cut",
		[&]()
		{
			sgnode* const selected = get_selected_sgnode();
			clipboard = selected->clone(this);
			destroy_action(selected);
		},
		[&]()
		{
			sgnode* const selected = get_selected_sgnode();
			return selected && !selected->is_root();
		},
		"Ctrl+X",
		GLFW_KEY_X,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_copy = {
		"Copy",
		[&]()
		{
			sgnode* const selected = get_selected_sgnode();
			clipboard = selected->clone(this);
		},
		[&]()
		{
			return get_selected_sgnode();
		},
		"Ctrl+C",
		GLFW_KEY_C,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_paste = {
		"Paste",
		[&]()
		{
			sgnode* const selected = get_selected_sgnode();
			if (selected->is_operation())
			{
				sgnode* const clone = clipboard->clone_self_and_insert(this, selected);
				set_selected_sgnode(clone);
			}
			else
			{
				assert(selected->get_parent());
				sgnode* const clone = clipboard->clone_self_and_insert(this, selected->get_parent());
				set_selected_sgnode(clone);
			}
		},
		[&]()
		{
			sgnode* const selected = get_selected_sgnode();
			return clipboard && selected;
		},
		"Ctrl+V",
		GLFW_KEY_V,
		GLFW_MOD_CONTROL,
	};

	shortcut_menu_item phorm_move_up = {
		"Move Up",
		[&]()
		{
			sgnode* const selected = get_selected_sgnode();
			const s64 i = selected->get_index();
			reparent_action(selected, selected->get_parent(), i - 1);
		},
		[&]()
		{
			const sgnode* const selected = get_selected_sgnode();
			return selected && !selected->is_root() && selected->get_index() > 0;
		},
		"Ctrl+Up",
		GLFW_KEY_UP,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_move_down = {
		"Move Down",
		[&]()
		{
			sgnode* const selected = get_selected_sgnode();
			assert(selected);
			const s64 i = selected->get_index();
			reparent_action(selected, selected->get_parent(), i + 1);
		},
		[&]()
		{
			const sgnode* const selected = get_selected_sgnode();
			return selected && !selected->is_root() && selected->get_index() < static_cast<s64>(selected->get_parent()->get_children().size() - 1);
		},
		"Ctrl+Down",
		GLFW_KEY_DOWN,
		GLFW_MOD_CONTROL,
	};

	shortcut_menu_item phorm_phreeze = {
		"Phreeze!",
		[&]()
		{
			freeze_action(get_selected_sgnode());
		},
		[&]()
		{
			const sgnode* const node = get_selected_sgnode();
			return node && !node->is_root() && node->get_gen()->mesh && !node->is_frozen();
		},
		"Ctrl+P",
		GLFW_KEY_P,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item phorm_unphreeze = {
		"Unphreeze!",
		[&]()
		{
			unfreeze_action(get_selected_sgnode());
		},
		[&]()
		{
			const sgnode* const node = get_selected_sgnode();
			return node && !node->is_root() && node->get_gen()->mesh && has_unfrozen(node);
		},
		"Ctrl+P",
		GLFW_KEY_P,
		GLFW_MOD_CONTROL,
	};

	shortcut_menu_item phorm_rename = {
		"Rename",
		[&]()
		{
			assert(m_sg_window);
			assert(!m_sg_window->get_renaming());
			m_sg_window->set_renaming(get_selected_sgnode());
		},
		[&]()
		{
			return get_selected_sgnode();
		},
		"Ctrl+R",
		GLFW_KEY_R,
		GLFW_MOD_CONTROL,
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
			sgnode* const selected = get_selected_sgnode();
			return selected && !selected->is_root();
		},
		"Delete",
		GLFW_KEY_DELETE,
		0,
	};

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

	shortcut_menu phorm_menu;
	phorm_menu.name = "Phorm";
	phorm_menu.groups.push_back({ phorm_create });
	phorm_menu.groups.push_back({ phorm_undo, phorm_redo });
	phorm_menu.groups.push_back({ phorm_cut, phorm_copy, phorm_paste });
	phorm_menu.groups.push_back({ phorm_move_up, phorm_move_down });
	phorm_menu.groups.push_back({ phorm_phreeze, phorm_unphreeze });
	phorm_menu.groups.push_back({ phorm_rename, phorm_destroy });
	phorm_menu.groups.push_back({ gizmodes });
	shortcut_menus.push_back(phorm_menu);
}
void app_ctx::material_menu()
{
	shortcut_menu_item material_create = {
		"Create",
		[&]()
		{
			scene_material* const mtl = scene.create_default_material();
			scene.add_material(mtl);
			set_selected_material(mtl);
		},
		[&]()
		{
			return true;
		},
		"Ctrl+Shift+=",
		GLFW_KEY_EQUAL,
		GLFW_MOD_CONTROL | GLFW_MOD_SHIFT,
	};
	shortcut_menu_item material_rename = {
		"Rename",
		[&]()
		{
			//
			// TODO
			//
		},
		[&]()
		{
			return get_selected_material();
		},
		"Ctrl+R",
		GLFW_KEY_R,
		GLFW_MOD_CONTROL,
	};
	shortcut_menu_item material_destroy = {
		"Destroy",
		[&]()
		{
			scene_material* const mtl = get_selected_material();
			scene.remove_material(scene.get_id_for_material(mtl));
			set_selected_material(nullptr);
		},
		[&]()
		{
			return get_selected_material();
		},
		"Delete",
		GLFW_KEY_DELETE,
		0,
	};

	shortcut_menu material_menu;
	material_menu.name = "Material";
	material_menu.groups.push_back({ material_create, material_rename, material_destroy });
	shortcut_menus.push_back(material_menu);
}
