#pragma once
#include "geom/carve.h"
#include "geom/glu_tess.h"

class sgnode;
class generated_mesh;

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
	const sgnode* get_sg_root() const { return m_sg_root; }
	void set_sg_root(sgnode* const new_root);
	sgnode* get_selected_node() { return m_selected_node; }
	void set_selected_node(sgnode* node) { m_selected_node = node; }
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
	mesh_t* create_textured_cuboid(GLuint mtl_id, const cuboid_options& options = {})
	{
		return textured_cuboid(m_tex_coord_attr, m_mtl_id_attr, mtl_id, options);
	}
	mesh_t* create_textured_ellipsoid(GLuint mtl_id, const ellipsoid_options& options = {})
	{
		return textured_ellipsoid(m_tex_coord_attr, m_mtl_id_attr, mtl_id, options);
	}
	mesh_t* create_textured_cylinder(GLuint mtl_id, const cylinder_options& options = {})
	{
		return textured_cylinder(m_tex_coord_attr, m_mtl_id_attr, mtl_id, options);
	}
	mesh_t* create_textured_cone(GLuint mtl_id, const cone_options& options = {})
	{
		return textured_cone(m_tex_coord_attr, m_mtl_id_attr, mtl_id, options);
	}
	mesh_t* create_textured_torus(GLuint mtl_id, const torus_options& options = {})
	{
		return textured_torus(m_tex_coord_attr, m_mtl_id_attr, mtl_id, options);
	}
	mesh_t* create_textured_heightmap(GLuint mtl_id, const mgl::retained_texture2d_rgb_u8* const map, const heightmap_options& options = {})
	{
		return textured_heightmap(m_tex_coord_attr, m_mtl_id_attr, mtl_id, map, options);
	}
	generated_mesh* generated_textured_cuboid(GLuint mtl_id, const cuboid_options& options = {});
	generated_mesh* generated_textured_ellipsoid(GLuint mtl_id, const ellipsoid_options& options = {});
	generated_mesh* generated_textured_cylinder(GLuint mtl_id, const cylinder_options& options = {});
	generated_mesh* generated_textured_cone(GLuint mtl_id, const cone_options& options = {});
	generated_mesh* generated_textured_torus(GLuint mtl_id, const torus_options& options = {});
	generated_mesh* generated_textured_heightmap(GLuint mtl_id, const mgl::retained_texture2d_rgb_u8* const map, const heightmap_options& options = {});
private:
	static inline u32 s_next_mtl_id = 1;
	constexpr static u32 s_vert_size = 8;
	// constexpr static f32 s_snap_angle = c::PI / 2;
	// constexpr static f32 s_snap_angle = c::PI / 3;
	// constexpr static f32 s_snap_angle = c::PI / 4;
public:
	static inline f32 s_snap_angle = 7 * c::PI / 24;
	static inline bool s_snap_all = false;
private:
	carve::csg::CSG m_csg;
	attr_tex_coord_t m_tex_coord_attr;
	attr_material_t m_mtl_id_attr;
	std::unordered_map<u32, scene_material> m_mtls;
	std::unordered_map<u32, mgl::static_vertex_array> m_sg_vaos_for_mtl, m_hm_vaos_for_mtl;
	std::vector<mesh_t*> m_hms;
	sgnode* m_sg_root = nullptr;
	sgnode* m_selected_node = nullptr;
	bool m_hms_dirty = false;
private:
	void m_build_sg_vaos();
	void m_tesselate(const mesh_t* mesh, std::unordered_map<u32, std::vector<mesh_vertex>>& out_verts_for_mtl);
	void m_draw_vaos(const mgl::context& glctx, const scene_ctx_uniforms& mats, const std::unordered_map<u32, mgl::static_vertex_array>& vaos);
};
