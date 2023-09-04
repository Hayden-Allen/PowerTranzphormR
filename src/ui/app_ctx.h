#pragma once
#include "core/scene_ctx.h"
#include "core/action_stack.h"
#include "shortcut_menu.h"

class scene_graph_window;
class materials_list_window;

enum class global_selection_type
{
	none = 0,
	sgnode,
	material,
	light
};

struct app_ctx
{
public:
	mgl::context mgl_ctx;
	scene_ctx scene;
	action_stack actions;
	mgl::framebuffer_u8 preview_fb;
	mgl::camera preview_cam;
	ImGuizmo::OPERATION gizmo_op = ImGuizmo::OPERATION::TRANSLATE;
	std::vector<shortcut_menu> shortcut_menus;
	sgnode* clipboard = nullptr;
	std::unordered_map<const sgnode*, sgnode*> frozen2unfrozen;
	mutable std::string loaded_filename;
	global_selection_type sel_type = global_selection_type::none;
public:
	app_ctx();
	~app_ctx();
public:
	void clear();
	bool save() const;
	void save(const std::string& fp) const;
	bool save_as() const;
	bool confirm_unsaved_changes();
	void load(const std::string& fp);
	void undo();
	void redo();
	bool has_unfrozen(const sgnode* const node) const;
	void set_sel_type(const global_selection_type t);
	void set_selected_sgnode(sgnode* const node);
	sgnode* get_selected_sgnode();
	sgnode* get_imgui_needs_select_unfocused_sgnode();
	void unset_imgui_needs_select_unfocused_sgnode();
	void set_selected_material(scene_material* const mtl);
	scene_material* get_selected_material();
	scene_material* get_imgui_needs_select_unfocused_mtl();
	void unset_imgui_needs_select_unfocused_mtl();
	void set_sg_window(scene_graph_window* const window);
	void set_mtls_window(materials_list_window* const window);
	std::vector<std::pair<u32, scene_material*>> get_sorted_materials();
	void remove_material(const u32 id);
	void set_material(sgnode* const node, const u32 id);
	void add_light();
	void set_selected_light(light* const l);
	light* get_selected_light();
public:
	void transform_action(sgnode* const t, const tmat<space::OBJECT, space::PARENT>& old_mat, const tmat<space::OBJECT, space::PARENT>& new_mat);
	void reparent_action(sgnode* const target, sgnode* const new_parent, const s64 new_index);
	void create_action(sgnode* const target, sgnode* const parent, const u32 skip_count = 0);
	void destroy_action(sgnode* const target);
	void destroy_selected_action();
	void create_cube_action();
	void create_sphere_action();
	void create_cylinder_action();
	void create_cone_action();
	void create_torus_action();
	void create_heightmap_action();
	void create_union_action();
	void create_subtract_action();
	void create_intersect_action();
	void freeze_action(sgnode* const target);
	void unfreeze_action(sgnode* const target);
	void rename_action(sgnode* const target, const std::string& new_name);
private:
	void create_operation_action(const carve::csg::CSG::OP op);
	template<typename FN>
	void create_shape_action(FN fn, const std::string& name);
	void create(sgnode* const node);
	void init_menus();
	void file_menu();
	void phorm_menu();
	void material_menu();
private:
	sgnode* m_selected_sgnode = nullptr;
	sgnode* m_imgui_needs_select_unfocused_sgnode = nullptr;
	scene_material* m_selected_mtl = nullptr;
	scene_material* m_imgui_needs_select_unfocused_mtl = nullptr;
	light* m_selected_light = nullptr;
	scene_graph_window* m_sg_window = nullptr;
	materials_list_window* m_mtls_window = nullptr;
};
