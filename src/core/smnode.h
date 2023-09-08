#pragma once
#include "pch.h"
#include "geom/generated_mesh.h"

class generated_mesh;
class scene_ctx;

class smnode
{
public:
	smnode(generated_mesh* const gen);
	virtual ~smnode();
public:
	void set_transform(const tmat<space::OBJECT, space::WORLD>& new_mat);
	void set_dirty();
	void set_gen_dirty();
	bool is_dirty() const;
	void recompute(scene_ctx* const scene);
	std::vector<point<space::OBJECT>>& get_local_verts();
	const std::vector<point<space::OBJECT>>& get_local_verts() const;
	const mesh_t* get_mesh() const;
	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const;
	u32 get_material() const;
	const tmat<space::OBJECT, space::WORLD>& get_mat() const;
private:
	tmat<space::OBJECT, space::WORLD> m_mat;
	generated_mesh* m_gen;
	std::vector<point<space::OBJECT>> m_local_verts;
	bool m_dirty = false;
private:
	void copy_local_verts();
	void transform_verts();
};