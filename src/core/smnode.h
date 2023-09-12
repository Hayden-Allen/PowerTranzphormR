#pragma once
#include "pch.h"
#include "geom/generated_mesh.h"
#include "xportable.h"

class generated_mesh;
class scene_ctx;

class smnode : public xportable
{
public:
	smnode(generated_mesh* const gen);
	smnode(const nlohmann::json& obj, scene_ctx* const scene);
	virtual ~smnode();
public:
	std::vector<point<space::OBJECT>>& get_local_verts();
	const std::vector<point<space::OBJECT>>& get_local_verts() const;
	const mesh_t* get_mesh() const;
	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const;
	u32 get_material() const;
	tmat<space::OBJECT, space::WORLD>& get_mat();
	const tmat<space::OBJECT, space::WORLD>& get_mat() const;
public:
	void set_gen_dirty();
	void set_transform(const tmat<space::OBJECT, space::WORLD>& new_mat);
	void set_material(scene_ctx* const scene, const u32 mat);
	void set_gen(generated_mesh* const gen);
public:
	bool is_gen_dirty() const;
	bool is_static() const;
public:
	void recompute(scene_ctx* const scene);
	void make_static(scene_ctx* const scene);
	nlohmann::json save(scene_ctx* const scene) const;
private:
	tmat<space::OBJECT, space::WORLD> m_mat;
	generated_mesh* m_gen;
	std::vector<point<space::OBJECT>> m_local_verts;
private:
	void copy_local_verts();
	void copy_local_to_carve();
};