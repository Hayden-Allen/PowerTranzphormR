#pragma once
#include "geom/carve.h"
#include "geom/glu_tess.h"

class sgnode;
class generated_mesh;
struct scene_material;

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
	// TODO temporary
	static inline f32 s_snap_angle = 7 * c::PI / 24;
	static inline bool s_snap_all = false;
public:
	scene_ctx();
	~scene_ctx();
public:
	carve::csg::CSG& get_csg();
	carve_vert_attrs& get_vert_attrs();
	attr_material_t& get_mtl_id_attr();
	void draw(const mgl::context& glctx, const scene_ctx_uniforms& mats);
	void update();
	void clear(bool ready_for_default_material = true);
	void destroy();
	void save(std::ofstream& out) const;
	void load(std::ifstream& in);
public:
	sgnode* get_sg_root();
	const sgnode* get_sg_root() const;
	void set_sg_root(sgnode* const new_root);
public:
	const std::unordered_map<u32, scene_material*>& get_materials();
	scene_material* create_default_material();
	u32 add_material(scene_material* mtl);
	void erase_material(const u32 id); // WARNING: Call app_ctx->remove_material instead of this one
	u32 get_id_for_material(scene_material* mat);
	scene_material* get_material(GLuint id);
public:
	mesh_t* create_textured_cuboid(const GLuint mtl_id, const cuboid_options& options = {});
	mesh_t* create_textured_ellipsoid(const GLuint mtl_id, const ellipsoid_options& options = {});
	mesh_t* create_textured_cylinder(const GLuint mtl_id, const cylinder_options& options = {});
	mesh_t* create_textured_cone(const GLuint mtl_id, const cone_options& options = {});
	mesh_t* create_textured_torus(const GLuint mtl_id, const torus_options& options = {});
	mesh_t* create_textured_heightmap(const GLuint mtl_id, const heightmap_options& options = {});
	generated_mesh* generated_textured_cuboid(const GLuint mtl_id, const cuboid_options& options = {});
	generated_mesh* generated_textured_ellipsoid(const GLuint mtl_id, const ellipsoid_options& options = {});
	generated_mesh* generated_textured_cylinder(const GLuint mtl_id, const cylinder_options& options = {});
	generated_mesh* generated_textured_cone(const GLuint mtl_id, const cone_options& options = {});
	generated_mesh* generated_textured_torus(const GLuint mtl_id, const torus_options& options = {});
	generated_mesh* generated_textured_heightmap(const GLuint mtl_id, const heightmap_options& options = {});
private:
	static inline u32 s_next_mtl_id = 1;
private:
	constexpr static std::vector<u32> get_vert_layout()
	{
		return {
			3,			// x, y, z
			3,			// nx, ny, nz
			4, 4, 4, 4, // uv0-uv3
			4,			// weights
			4,			// rgba
		};
	}
private:
	carve::csg::CSG m_csg;
	attr_material_t m_mtl_id_attr;
	carve_vert_attrs m_vert_attrs;
	std::unordered_map<u32, scene_material*> m_mtls;
	// std::unordered_map<u32, mgl::static_vertex_array> m_sg_vaos_for_mtl, m_hm_vaos_for_mtl;
	std::unordered_map<u32, mgl::static_render_object> m_sg_vaos_for_mtl, m_hm_vaos_for_mtl;
	std::vector<mesh_t*> m_hms;
	sgnode* m_sg_root = nullptr;
	sgnode* m_selected_node = nullptr;
	scene_material* m_selected_mtl = nullptr;
	bool m_hms_dirty = false;
private:
	void m_build_sg_vaos();
	void m_tesselate(const mesh_t* mesh, std::unordered_map<u32, std::vector<mesh_vertex>>& out_verts_for_mtl, std::unordered_map<u32, std::vector<u32>>& out_indices_for_mtl);
	void m_draw_vaos(const mgl::context& glctx, const scene_ctx_uniforms& mats, const std::unordered_map<u32, mgl::static_render_object>& ros);
};
