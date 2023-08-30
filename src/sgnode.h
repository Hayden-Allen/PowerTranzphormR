#pragma once
#include "pch.h"

class generated_mesh;
class scene_ctx;
struct app_ctx;

class sgnode
{
public:
	// nop node
	sgnode(sgnode* p, generated_mesh* m, const std::string& n, const tmat<space::OBJECT, space::PARENT>& t = tmat<space::OBJECT, space::PARENT>());
	// operation node
	sgnode(sgnode* p, carve::csg::CSG::OP op, const tmat<space::OBJECT, space::PARENT>& t = tmat<space::OBJECT, space::PARENT>());
	sgnode(const nlohmann::json& obj, scene_ctx* const scene);
	MGL_DCM(sgnode);
	virtual ~sgnode();
public:
	static u32 get_next_id();
	static void set_next_id(const u32 id);
public:
	s64 get_index() const;
	sgnode* get_parent();
	const sgnode* get_parent() const;
	const std::string& get_id() const;
	const std::string& get_name() const;
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
	void set_transform(const tmat<space::OBJECT, space::PARENT>& new_mat);
	void transform(const tmat<space::OBJECT, space::OBJECT>& m);
	void set_operation(const carve::csg::CSG::OP op);
	void set_dirty();
	void set_gen_dirty();
	void set_name(const std::string& n);
	tmat<space::OBJECT, space::WORLD> accumulate_mats() const;
	tmat<space::PARENT, space::WORLD> accumulate_parent_mats() const;
	void remove_material(u32 mtl_id);
public:
	void add_child(sgnode* const node, const s64 index = -1);
	s64 remove_child(sgnode* const node);
	sgnode* clone(app_ctx* const app) const;
	sgnode* clone_self_and_insert(app_ctx* const app, sgnode* const parent) const;
	sgnode* freeze(scene_ctx* const scene) const;
	void recompute(scene_ctx* const scene);
	nlohmann::json save(scene_ctx* const scene) const;
	void destroy(std::unordered_set<sgnode*>& freed);
private:
	static inline u32 s_next_id = 0;
private:
	sgnode* m_parent = nullptr;
	std::vector<sgnode*> m_children;
	generated_mesh* m_gen = nullptr;
	carve::csg::CSG::OP m_operation = carve::csg::CSG::OP::ALL;
	std::string m_id, m_name;
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
};