#pragma once

#include "camera.h"
#include "carve.h"
#include "glu_tess.h"
#include "scene_graph.h"

class preview_layer : public mgl::layer
{
public:
	preview_layer(const mgl::context& mgl_context, int fb_width, int fb_height);
	virtual ~preview_layer();
public:
	virtual void on_frame(const f32 dt) override;
	virtual void on_window_resize(const s32 width, const s32 height) override;
	virtual void on_mouse_button(const s32 button, const s32 action, const s32 mods) override;
	virtual void on_mouse_move(const f32 x, const f32 y, const f32 dx, const f32 dy) override;
	virtual void on_scroll(const f32 x, const f32 y) override;
	virtual void on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods) override;
	void set_exit_preview_callback(const std::function<void()> &callback);
	const mgl::framebuffer_u8& get_framebuffer();
private:
	void m_make_scene(carve::csg::CSG& csg, attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, mesh_t** out_mesh, std::unordered_map<GLuint, material_t>& out_mtls);
	void m_compute_csg(std::unordered_map<GLuint, material_t>& out_mtls, std::unordered_map<GLuint, std::vector<GLfloat>>& out_vtxs_for_mtl);
	void m_tesselate(mesh_t* in_scene, std::unordered_map<GLuint, std::vector<GLfloat>>& out_vtxs_for_mtl, attr_tex_coord_t tex_coord_attr, attr_material_t mtl_id_attr);
private:
	const mgl::context& m_mgl_context;
	u32 m_fb_width = 0, m_fb_height = 0;
	sgnode* m_sg = nullptr;
	mgl::shaders m_shaders;
	const point<space::WORLD> m_cam_pos;
	f32 m_ar = 0.0f;
	f32 m_mx = 0.0f, m_my = 0.0f;
	camera m_cam;
	tmat<space::OBJECT, space::WORLD> m_obj;
	std::unordered_map<GLuint, material_t> m_mtls;
	std::unordered_map<GLuint, std::vector<GLfloat>> m_vtxs_for_mtl;
	std::unordered_map<GLuint, mgl::static_vertex_array> m_vaos_for_mtl;
	std::unordered_map<GLuint, mgl::texture2d_rgb_u8> m_texs_for_mtl;
	mgl::framebuffer_u8 m_fb;
	bool m_keys[11] = { false };
	std::function<void()> m_exit_preview_callback;
};
