#pragma once

#include "camera.h"
#include "carve.h"
#include "glu_tess.h"
#include "scene_graph.h"

struct material_texture
{
	std::string name;
	mgl::texture2d_rgb_u8* tex;

	material_texture(const std::string& n, mgl::texture2d_rgb_u8* const t) :
		name(n),
		tex(t)
	{}
	material_texture(const material_texture& o) noexcept :
		name(o.name),
		tex(o.tex)
	{}
};

class preview_layer : public mgl::layer
{
public:
	preview_layer(const mgl::context* const mgl_context, const camera& cam);
	virtual ~preview_layer();
public:
	virtual void on_frame(const f32 dt) override;
	virtual void on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods) override;
	const mgl::framebuffer_u8& get_framebuffer() const
	{
		return m_fb;
	}
private:
	// TODO move
	void m_make_scene(carve::csg::CSG& csg, attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, mesh_t** out_mesh, std::unordered_map<GLuint, material_t>& out_mtls);
	void m_compute_csg(std::unordered_map<GLuint, material_t>& out_mtls, std::unordered_map<GLuint, std::vector<GLfloat>>& out_vtxs_for_mtl);
	void m_tesselate(mesh_t* in_scene, std::unordered_map<GLuint, std::vector<GLfloat>>& out_vtxs_for_mtl, attr_tex_coord_t tex_coord_attr, attr_material_t mtl_id_attr, const bool clear);
private:
	sgnode *m_sphere_node = nullptr, *m_tor_node = nullptr;
	scene_graph* m_sg = nullptr;
	const mgl::context* m_mgl_context;
	// mgl::shaders m_shaders;
	mgl::framebuffer_u8 m_fb;
	camera m_cam;
	// TODO move
	std::unordered_map<GLuint, material_t> m_mtls;
	std::unordered_map<GLuint, std::vector<GLfloat>> m_vtxs_for_mtl;
	std::unordered_map<GLuint, mgl::static_vertex_array> m_vaos_for_mtl;
	// std::unordered_map<GLuint, mgl::texture2d_rgb_u8> m_texs_for_mtl;
	std::unordered_map<GLuint, std::vector<material_texture>> m_texs_for_mtl;
	std::unordered_map<GLuint, mgl::shaders*> m_shaders_for_mtl;
	struct
	{
		carve::csg::CSG csg;
		attr_tex_coord_t tex_coord_attr;
		attr_material_t mtl_id_attr;
	} m_ctx;
};
