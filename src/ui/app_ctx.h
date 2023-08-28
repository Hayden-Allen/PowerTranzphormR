#pragma once
#include "scene_ctx.h"
#include "action_stack.h"
#include "shortcut_menu.h"

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
	std::unordered_map<sgnode*, sgnode*> frozen;
	bool clipboard_cut = false;
	mutable std::string loaded_filename;
public:
	app_ctx();
public:
	void clear();
	bool save() const;
	void save(const std::string& fp) const;
	bool save_as() const;
	bool confirm_unsaved_changes();
	void load(const std::string& fp);
	void undo();
	void redo();
	bool is_node_frozen(sgnode* const node) const;
public:
	void transform_action(sgnode* const t, const tmat<space::OBJECT, space::PARENT>& old_mat, const tmat<space::OBJECT, space::PARENT>& new_mat);
	void reparent_action(sgnode* const target, sgnode* const new_parent, const s64 new_index);
	void create_action(sgnode* const target, sgnode* const parent);
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
private:
	void create_operation_action(const carve::csg::CSG::OP op);
	template<typename FN>
	void create_shape_action(FN fn, const std::string& name);
	void create(sgnode* const node);
	void init_menus();
	void file_menu();
	void edit_menu();
	void phorm_menu();
};
