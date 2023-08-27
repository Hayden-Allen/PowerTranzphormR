#pragma once
#include "pch.h"

class generated_mesh;
class scene_ctx;
struct app_ctx;

class sgnode
{
public:
	sgnode* parent;
	std::vector<sgnode*> children;
	std::vector<point<space::OBJECT>> src_verts;
	generated_mesh* gen;
	carve::csg::CSG::OP operation;
	std::string id, name;
	bool selected, dirty;
	tmat<space::OBJECT, space::PARENT> mat;
private:
	static inline u32 s_next_id = 0;
public:
	// leaf node
	sgnode(sgnode* p, generated_mesh* m, const std::string& n, const tmat<space::OBJECT, space::PARENT>& t = tmat<space::OBJECT, space::PARENT>());
	// non-leaf node
	sgnode(carve::csg::CSG& scene, sgnode* p, carve::csg::CSG::OP op, const tmat<space::OBJECT, space::PARENT>& t = tmat<space::OBJECT, space::PARENT>());
	sgnode(const nlohmann::json& obj);
	MGL_DCM(sgnode);
	virtual ~sgnode();
public:
	static u32 get_next_id();
	static void set_next_id(const u32 id);
public:
	s64 get_index() const;
	bool is_root() const;
	bool is_leaf() const;
	bool is_mesh() const;
	bool owns_mesh() const;
	void set_transform(const tmat<space::OBJECT, space::PARENT>& new_mat);
	void set_operation(const carve::csg::CSG::OP op);
	void set_gen_dirty();
	void set_dirty();
	tmat<space::OBJECT, space::WORLD> accumulate_mats();
	tmat<space::OBJECT, space::WORLD> accumulate_parent_mats();
public:
	void add_child(sgnode* const node, const s64 index = -1);
	s64 remove_child(sgnode* const node);
	sgnode* clone(app_ctx* const app, sgnode* const parent) const;
	void recompute(scene_ctx* const scene);
	void transform(const tmat<space::OBJECT, space::OBJECT>& m);
	nlohmann::json save() const;
private:
	sgnode();
private:
	void copy_verts();
	void transform_verts();
};