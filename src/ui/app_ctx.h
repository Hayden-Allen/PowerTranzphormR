#pragma once
#include "scene_ctx.h"
#include "action_stack.h"
#include "shortcut_menu.h"

struct app_ctx
{
public:
	scene_ctx scene;
	action_stack actions;
	mgl::context mgl_ctx;
	mgl::framebuffer_u8 preview_fb;
	mgl::camera preview_cam;
	ImGuizmo::OPERATION gizmo_op = ImGuizmo::OPERATION::TRANSLATE;
	std::vector<shortcut_menu> shortcut_menus;
	sgnode* clipboard;
	bool clipboard_cut = false;
public:
	app_ctx();
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
	void save(const std::string& fp) const
	{
		std::ofstream out(fp);
		assert(out.is_open());
		actions.save(out, scene.get_sg_root());
	}
	void load(const std::string& fp)
	{
		std::ifstream in(fp);
		assert(in.is_open());
		scene.set_sg_root(actions.load(in));
	}
private:
	void create_operation_action(const carve::csg::CSG::OP op);
	template<typename FN>
	void create_shape_action(FN fn, const std::string& name);
	void create(sgnode* const node);
	void init_menus();
};