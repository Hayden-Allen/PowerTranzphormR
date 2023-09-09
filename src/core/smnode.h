#pragma once
#include "pch.h"
#include "geom/generated_mesh.h"

class generated_mesh;
class scene_ctx;

class smnode
{
public:
	smnode(generated_mesh* const gen);
	smnode(const nlohmann::json& obj, scene_ctx* const scene);
	virtual ~smnode();
public:
	static u32 get_next_id();
	static void set_next_id(const u32 id);
	static void reset_next_id();
public:
	std::vector<point<space::OBJECT>>& get_local_verts();
	const std::vector<point<space::OBJECT>>& get_local_verts() const;
	const mesh_t* get_mesh() const;
	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const;
	u32 get_material() const;
	tmat<space::OBJECT, space::WORLD>& get_mat();
	const tmat<space::OBJECT, space::WORLD>& get_mat() const;
	const std::string& get_id() const;
	const std::string& get_name() const;
public:
	void set_name(const std::string& name);
	void set_dirty();
	void set_gen_dirty();
	void set_transform(const tmat<space::OBJECT, space::WORLD>& new_mat);
	void set_material(scene_ctx* const scene, const u32 mat);
public:
	bool is_dirty() const;
	bool is_static() const;
public:
	void recompute(scene_ctx* const scene);
	void make_static(scene_ctx* const scene);
	nlohmann::json save(scene_ctx* const scene) const;
private:
	constexpr static inline u32 s_first_id = 0;
	static inline u32 s_next_id = s_first_id;
private:
	tmat<space::OBJECT, space::WORLD> m_mat;
	generated_mesh* m_gen;
	std::vector<point<space::OBJECT>> m_local_verts;
	bool m_dirty = false;
	std::string m_id;
	std::string m_name = "Heightmap";
	u32 m_material;
private:
	void copy_local_verts();
	void transform_verts();
};