#pragma once
#include "pch.h"
#include "visibility_xportable.h"
#include "util/color.h"

class generated_mesh;
class generated_static_mesh;
class scene_ctx;
struct app_ctx;

class sgnode : public visibility_xportable
{
public:
	// nop node
	sgnode(sgnode* p, generated_mesh* m, const std::string& n, const tmat<space::OBJECT, space::PARENT>& t = tmat<space::OBJECT, space::PARENT>());
	// frozen node
	sgnode(sgnode* const parent, generated_static_mesh* const m, const std::string& name, const tmat<space::OBJECT, space::OBJECT>& t);
	// operation node
	sgnode(sgnode* p, carve::csg::CSG::OP op, const tmat<space::OBJECT, space::PARENT>& t = tmat<space::OBJECT, space::PARENT>());
	sgnode(const nlohmann::json& obj, scene_ctx* const scene);
	MGL_DCM(sgnode);
	virtual ~sgnode();
public:
	s64 get_index() const;
	sgnode* get_parent();
	const sgnode* get_parent() const;
	tmat<space::OBJECT, space::PARENT>& get_mat();
	const tmat<space::OBJECT, space::PARENT>& get_mat() const;
	generated_mesh* get_gen();
	const generated_mesh* get_gen() const;
	const std::vector<sgnode*>& get_children() const;
	const carve::csg::CSG::OP get_operation() const;
	bool is_root() const;
	bool is_operation() const;
	bool is_dirty() const;
	bool is_frozen() const;
	void set_frozen_vertices_color(const color_t& col, scene_ctx* const scene);
	void center_frozen_vertices_at_origin();
	void set_transform(const tmat<space::OBJECT, space::PARENT>& new_mat);
	void transform(const tmat<space::OBJECT, space::OBJECT>& m);
	void set_operation(const carve::csg::CSG::OP op);
	void set_dirty();
	void set_gen_dirty();
	tmat<space::OBJECT, space::WORLD> accumulate_mats() const;
	tmat<space::PARENT, space::WORLD> accumulate_parent_mats() const;
	void set_material(scene_ctx* const scene, u32 mtl_id);									   // WARNING: Always use app_ctx->set_material instead of this one
	void replace_material(scene_ctx* const scene, const u32 old_mtl_id, const u32 new_mtl_id); // WARNING: Always use app_ctx->remove_material instead of using this
	u32 get_material();
	std::vector<point<space::OBJECT>>& get_local_verts();
	void set_visibility(const bool v) override;
public:
	void add_child(sgnode* const node, const s64 index = -1);
	s64 remove_child(sgnode* const node);
	sgnode* clone(app_ctx* const app) const;
	sgnode* clone_self_and_insert(app_ctx* const app, sgnode* const parent) const;
	sgnode* freeze(scene_ctx* const scene) const;
	void recompute(scene_ctx* const scene);
	const generated_mesh* compute_xport(scene_ctx* const scene) const;
	nlohmann::json save(scene_ctx* const scene) const;
	void destroy(std::unordered_set<sgnode*>& freed);
	bool is_separate_xport() const;
private:
	sgnode* m_parent = nullptr;
	std::vector<sgnode*> m_children;
	generated_mesh* m_gen = nullptr;
	carve::csg::CSG::OP m_operation = carve::csg::CSG::OP::ALL;
	bool m_dirty = false, m_frozen = false;
	tmat<space::OBJECT, space::PARENT> m_mat;
	std::vector<point<space::OBJECT>> m_local_verts;
private:
	sgnode(const sgnode* const original);
private:
	void set_parent(sgnode* const parent);
	void copy_local_verts();
	void transform_verts();
	void set_dirty_up();
	void set_dirty_down();
	u32 subtree_count() const;
	void clone_self_and_insert(app_ctx* const app, sgnode* const parent, const u32 count) const;
};