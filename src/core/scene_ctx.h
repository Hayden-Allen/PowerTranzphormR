#pragma once
#include "geom/carve.h"
#include "geom/glu_tess.h"
#include "light.h"
#include "waypoint.h"

class sgnode;
class generated_mesh;
class generated_heightmap;
struct scene_material;
class smnode;
struct app_ctx;

struct scene_ctx_uniforms
{
	/*const mat<space::OBJECT, space::CLIP>& mvp;
	const tmat<space::OBJECT, space::CAMERA>& mv;*/
	const mat<space::WORLD, space::CLIP> vp;
	const tmat<space::WORLD, space::CAMERA>& view;
	const pmat<space::CAMERA, space::CLIP>& proj;
	/*const tmat<space::OBJECT, space::WORLD>& model;
	const tmat<space::OBJECT, space::WORLD>& normal;*/
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
	const carve_vert_attrs& get_vert_attrs() const;
	const attr_material_t& get_mtl_id_attr() const;
	void draw(const mgl::context& glctx, const scene_ctx_uniforms& mats);
	void update(app_ctx* const app);
	void clear(bool ready_for_default_material = true);
	void destroy();
	void save(std::ofstream& out, const std::string& out_fp);
	const std::string load(std::ifstream& in, const std::string& in_fp);
	void xport_sgnode(sgnode* const cur, std::vector<std::unordered_map<u32, mgl::static_retained_render_object>*>* const phorm_ros);
	void save_xport(haul::output_file* const out);
	void load_skybox(const std::string& folder, const std::string& phorm_base);
public:
	sgnode* get_sg_root();
	const sgnode* get_sg_root() const;
	void set_sg_root(sgnode* const new_root);
public:
	const std::unordered_map<u32, scene_material*>& get_materials();
	scene_material* create_default_material();
	u32 add_material(scene_material* const mtl);
	void erase_material(const u32 id); // WARNING: Call app_ctx->remove_material instead of this one
	u32 get_id_for_material(scene_material* const mat);
	scene_material* get_material(const GLuint id);
public:
	std::vector<light*>& get_lights();
	light* add_light();
	light* add_light(light* const l);
	void destroy_light(light* const l);
	void update_light(const light* const l);
public:
	std::vector<waypoint*>& get_waypoints();
	waypoint* add_waypoint();
	waypoint* add_waypoint(waypoint* const w);
	void destroy_waypoint(waypoint* const w);
public:
	const std::vector<smnode*>& get_static_meshes();
	smnode* const add_static_mesh();
	void add_static_mesh(smnode* const node);
	void destroy_static_mesh(smnode* const n);
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
	generated_mesh* generated_textured_heightmap_static(const GLuint mtl_id, const heightmap_options& options = {});
	void tesselate_external(const mesh_t* mesh, std::unordered_map<u32, std::vector<mesh_vertex>>& out_verts_for_mtl);
private:
	static inline u32 s_next_mtl_id = 1;
	constexpr static u32 s_num_lights = 128;
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
	std::unordered_map<u32, mgl::static_retained_render_object> m_sg_ros_for_mtl;
	std::unordered_map<smnode*, std::unordered_map<u32, mgl::static_retained_render_object>> m_sm_ros;
	std::vector<smnode*> m_static_meshes;
	std::vector<light*> m_lights;
	std::vector<waypoint*> m_waypoints;
	sgnode* m_sg_root = nullptr;
	mgl::static_uniform_buffer m_light_buffer;
	u32 m_num_visible_lights = 0;
	mgl::retained_skybox_rgb_u8* m_skybox = nullptr;
	std::string m_skybox_folder;
private:
	void m_build_light_buffer();
	void m_build_sg_vaos();
	void m_build_sm_vaos();
	std::unordered_map<u32, mgl::static_retained_render_object> m_build_sm_vaos_world(smnode* const node);
	void m_tesselate(const mesh_t* mesh, std::unordered_map<u32, std::vector<mesh_vertex>>& out_verts_for_mtl, std::unordered_map<u32, std::vector<u32>>& out_indices_for_mtl, const bool snap_norms);
	void m_draw_vaos(const mgl::context& glctx, const scene_ctx_uniforms& mats, const std::unordered_map<u32, mgl::static_retained_render_object>& ros, const tmat<space::OBJECT, space::WORLD>& model = tmat<space::OBJECT, space::WORLD>(), const tex_coord_t* offset = nullptr);
	void m_compute_norms_snap(std::vector<mesh_vertex>& input_verts, std::vector<u32>& indices, const bool snap_all, const f32 snap_angle);
	void m_compute_norms(std::vector<mesh_vertex>& input_verts, std::vector<u32>& indices);
};
