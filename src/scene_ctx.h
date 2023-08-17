#pragma once

#include "carve.h"
#include "glu_tess.h"
#include "scene_graph.h"

struct scene_material
{
	std::string name;
	std::vector<std::pair<std::string, mgl::texture2d_rgb_u8*>> texs;
	mgl::shaders* shaders;

	scene_material() :
		name(""),
		shaders(nullptr)
	{}
	scene_material(const std::string& n, const std::vector<std::pair<std::string, mgl::texture2d_rgb_u8*>>& t, mgl::shaders* const s) :
		name(n),
		texs(t),
		shaders(s)
	{}
	scene_material(const scene_material& o) noexcept :
		name(o.name),
		texs(o.texs),
		shaders(o.shaders)
	{}
};

struct scene_ctx_uniforms
{
	const mat<space::OBJECT, space::CLIP>& mvp;
	const tmat<space::OBJECT, space::CAMERA>& mv;
	const tmat<space::OBJECT, space::WORLD>& model;
	const tmat<space::OBJECT, space::WORLD>& normal;
	const point<space::WORLD>& cam_pos;
};

class scene_ctx
{
public:
	scene_ctx();
	~scene_ctx();
public:
	sgnode* get_sg_root() { return m_sg_root; }
	u32 add_heightmap(mesh_t* hm);
	void remove_heightmap(const u32 id);
	u32 add_material(const scene_material& mtl);
	void remove_material(const u32 id);
	void update();
	void draw(const mgl::context& glctx, const scene_ctx_uniforms& mats);
	// TODO probably remove (change textured_* interface)
	attr_tex_coord_t& get_tex_coord_attr()
	{
		return m_tex_coord_attr;
	}
	attr_material_t& get_mtl_attr()
	{
		return m_mtl_id_attr;
	}
	carve::csg::CSG& get_csg()
	{
		return m_csg;
	}
	void set_sg_root(sgnode* n)
	{
		m_sg_root = n;
		// need to build initial vaos when root node assigned
		m_build_sg_vaos();
	}
private:
	static inline u32 s_next_mtl_id = 1;
	constexpr static u32 s_vert_size = 8;
	constexpr static f32 s_snap_angle = c::PI / 2;
private:
	carve::csg::CSG m_csg;
	attr_tex_coord_t m_tex_coord_attr;
	attr_material_t m_mtl_id_attr;
	std::unordered_map<u32, scene_material> m_mtls;
	std::unordered_map<u32, mgl::static_vertex_array> m_sg_vaos_for_mtl, m_hm_vaos_for_mtl;
	std::vector<mesh_t*> m_hms;
	sgnode* m_sg_root = nullptr;
	bool m_hms_dirty = false;
private:
	void m_build_sg_vaos();
	void m_tesselate(const mesh_t* mesh, std::unordered_map<u32, std::vector<mesh_vertex>>& out_verts_for_mtl);
	void m_draw_vaos(const mgl::context& glctx, const scene_ctx_uniforms& mats, const std::unordered_map<u32, mgl::static_vertex_array>& vaos);
};
