#pragma once
#include "pch.h"
#include "geom/generated_mesh.h"
#include "visibility_xportable.h"
#include "util/color.h"

class generated_mesh;
class scene_ctx;

class smnode : public visibility_xportable
{
public:
	smnode(generated_mesh* const gen);
	smnode(generated_mesh* const gen, const tmat<space::OBJECT, space::WORLD>& mat, const std::string& name);
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
	bool should_snap() const;
	bool should_snap_all() const;
	f32 get_snap_angle() const;
	const tex_coord_t* get_uv_offset() const;
	smnode* clone(scene_ctx* const scene) const;
public:
	void set_gen_dirty();
	void set_transform(const tmat<space::OBJECT, space::WORLD>& new_mat);
	void set_material(scene_ctx* const scene, const u32 mat);
	void set_vertices_color(const color_t& col, scene_ctx* const scene);
	void center_vertices_at_origin();
	void set_gen(generated_mesh* const gen);
	void set_should_snap(const bool snap);
	void set_should_snap_all(const bool all);
	void set_snap_angle(const f32 snap);
public:
	bool is_gen_dirty() const;
	bool is_static() const;
public:
	void recompute(scene_ctx* const scene);
	void make_static(scene_ctx* const scene);
	nlohmann::json save(scene_ctx* const scene) const;
	void xport(mgl::output_file& out) const override;
private:
	tmat<space::OBJECT, space::WORLD> m_mat;
	generated_mesh* m_gen;
	std::vector<point<space::OBJECT>> m_local_verts;
	bool m_should_snap = false, m_snap_all = false;
	f32 m_snap_angle = 7 * c::PI / 24;
private:
	void copy_local_verts();
	void copy_local_to_carve();
};