#include "pch.h"
#include "preview_layer.h"

using namespace mgl;
using namespace hats;

preview_layer::preview_layer(const mgl::context& mgl_context, int fb_width, int fb_height) :
	mgl::layer(),
	m_mgl_context(mgl_context),
	m_shaders(shaders::from_files("src/glsl/csg.vert", "src/glsl/csg.frag")),
	m_cam_pos(0, 0, 5),
	m_fb_width(fb_width),
	m_fb_height(fb_height),
	m_ar((f32)m_fb_width / (f32)m_fb_height),
	m_cam(m_cam_pos, 0, 0, 108 / m_ar, m_ar, 0.1f, 1000.0f, 5.0f),
	m_fb(m_fb_width, m_fb_height)
{
	m_compute_csg(m_mtls, m_vtxs_for_mtl);

	for (auto it = m_vtxs_for_mtl.begin(); it != m_vtxs_for_mtl.end(); ++it)
	{
		static_vertex_array vao(it->second.data(), (u32)it->second.size(), { 3, 2 });
		m_vaos_for_mtl.emplace(it->first, std::move(vao));
	}

	stbi_set_flip_vertically_on_load(true);
	for (GLuint i = 1; i <= 2; ++i)
	{
		int tex_w = -1, tex_h = -1, tex_c = -1;
		stbi_uc* tex_data = stbi_load(
			(std::string("res/") + std::to_string(i) + ".png").c_str(), &tex_w,
			&tex_h, &tex_c, 3);
		texture2d_rgb_u8 tex(GL_RGB, tex_w, tex_h, tex_data);
		stbi_image_free(tex_data);
		m_texs_for_mtl.emplace(i, std::move(tex));
	}
}

preview_layer::~preview_layer()
{
	//
}

void preview_layer::on_frame(const f32 dt)
{
	const f32 tdx = 1.f * m_keys[8] - m_keys[7];
	const f32 tdy = 1.f * m_keys[9] - m_keys[10];
	if (tdx != 0 || tdy != 0)
	{
		m_tor_node->transform(m_ctx.csg, carve::math::Matrix::TRANS(tdx * m_mgl_context.time.delta, tdy * m_mgl_context.time.delta, 0));
		// sphere_node->transform(ctx.csg, carve::math::Matrix::TRANS(tdx * c.time.delta, 0, 0));
		m_tesselate(m_sg->mesh, m_vtxs_for_mtl, m_ctx.tex_coord_attr, m_ctx.mtl_id_attr);
		m_vaos_for_mtl.clear();
		for (auto it = m_vtxs_for_mtl.begin(); it != m_vtxs_for_mtl.end(); ++it)
		{
			static_vertex_array vao(it->second.data(), (u32)it->second.size(), { 3, 2 });
			m_vaos_for_mtl.emplace(it->first, std::move(vao));
		}
	}

	const direction<space::CAMERA> move_dir(m_keys[3] - m_keys[1], m_keys[4] - m_keys[6], m_keys[2] - m_keys[0]);
	m_cam.move(m_mgl_context.time.delta, move_dir * (m_keys[5] ? 2.f : 1.f), m_mx, m_my);
	m_mx = 0.0f;
	m_my = 0.0f;

	const mat<space::OBJECT, space::CLIP>& mvp = m_cam.get_view_proj() * m_obj;

	m_fb.bind();
	glViewport(0, 0, m_fb.get_width(), m_fb.get_height());
	glEnable(GL_DEPTH_TEST);
	m_mgl_context.clear();
	for (auto it = m_vaos_for_mtl.begin(); it != m_vaos_for_mtl.end(); ++it)
	{
		const texture2d_rgb_u8& tex = m_texs_for_mtl[it->first];
		tex.bind(0);
		const material_t mat = m_mtls[it->first];
		m_shaders.bind();
		m_shaders.uniform_1i("u_tex", 0);
		m_shaders.uniform_3f("u_col", mat.r, mat.g, mat.b);
		m_shaders.uniform_mat4("u_mvp", mvp.e);
		m_mgl_context.draw(it->second, m_shaders);
	}
	m_fb.unbind();
}

void preview_layer::on_window_resize(const s32 width, const s32 height)
{
	//
}

void preview_layer::on_mouse_button(const s32 button, const s32 action, const s32 mods)
{
	//
}

void preview_layer::on_mouse_move(const f32 x, const f32 y, const f32 dx, const f32 dy)
{
	m_mx = dx;
	m_my = dy;
}

void preview_layer::on_scroll(const f32 x, const f32 y)
{
	//
}

void preview_layer::on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods)
{
	constexpr u32 NUM_KEYS = 11;

	if (key == GLFW_KEY_ESCAPE)
	{
		m_mx = 0.0f;
		m_my = 0.0f;
		for (int i = 0; i < NUM_KEYS; ++i)
		{
			m_keys[i] = false;
		}
		m_exit_preview_callback();
		return;
	}

	constexpr static u32 keycodes[NUM_KEYS] = { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN };
	for (int i = 0; i < NUM_KEYS; ++i)
	{
		if (key == keycodes[i])
		{
			if (action == GLFW_PRESS)
			{
				m_keys[i] = true;
			}
			else if (action == GLFW_RELEASE)
			{
				m_keys[i] = false;
			}
		}
	}
}

void preview_layer::set_exit_preview_callback(const std::function<void()>& callback)
{
	m_exit_preview_callback = callback;
}

const framebuffer_u8& preview_layer::get_framebuffer()
{
	return m_fb;
}

void preview_layer::m_make_scene(carve::csg::CSG& csg, attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, mesh_t** out_mesh, std::unordered_map<GLuint, material_t>& out_mtls)
{
	material_t mtl1;
	out_mtls.insert(std::make_pair(1, mtl1));
	material_t mtl2;
	out_mtls.insert(std::make_pair(2, mtl2));


	mesh_t* cyl = textured_cylinder(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.top_radius = .5f,
			.bottom_radius = .5f,
			.transform = tmat_util::translation<space::OBJECT>(0, .5f, 0) *
						 tmat_util::scale<space::OBJECT>(1.5f, 1.f, 1.5f),
		});
	sgnode* n0 = new sgnode(nullptr, cyl);

	mesh_t* box_b = textured_cuboid(
		tex_coord_attr, mtl_id_attr, 2,
		{
			.width = 3.f,
			.transform = tmat_util::translation<space::OBJECT>(0, -1.f, 0),
		});
	sgnode* n1 = new sgnode(nullptr, box_b);

	// out_mesh = csg.compute(cyl, box_b, carve::csg::CSG::B_MINUS_A, nullptr, carve::csg::CSG::CLASSIFY_EDGE);
	sgnode* n2 = new sgnode(csg, nullptr, carve::csg::CSG::B_MINUS_A, n0, n1);
	n0->parent = n1->parent = n2;

	mesh_t* cone = textured_cone(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.radius = .5f,
			.height = 1.f,
			.transform = tmat_util::translation<space::OBJECT>(0.f, 1.5f, 0),
		});
	// out_mesh = csg.compute(out_mesh, cone, carve::csg::CSG::UNION, nullptr, carve::csg::CSG::CLASSIFY_EDGE);
	sgnode* n3 = new sgnode(nullptr, cone);
	sgnode* n4 = new sgnode(csg, nullptr, carve::csg::CSG::UNION, n2, n3);
	n2->parent = n3->parent = n4;

	mesh_t* tor = textured_torus(tex_coord_attr, mtl_id_attr, 2,
		{
			.tube_radius = .5f,
			// .num_center_steps = 64,
			// .num_tube_steps = 64,
			.transform = tmat_util::translation<space::OBJECT>(1.f, c::EPSILON, 0),
		});
	// out_mesh = csg.compute(out_mesh, tor, carve::csg::CSG::A_MINUS_B, nullptr, carve::csg::CSG::CLASSIFY_EDGE);
	m_tor_node = new sgnode(nullptr, tor);
	sgnode* n6 = new sgnode(csg, nullptr, carve::csg::CSG::A_MINUS_B, n4, m_tor_node);
	n4->parent = m_tor_node->parent = n6;

	mesh_t* tor2 = textured_torus(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.transform = tmat_util::translation<space::OBJECT>(3.f, 0, 0),
		});
	// out_mesh = csg.compute(out_mesh, tor2, carve::csg::CSG::UNION, nullptr, carve::csg::CSG::CLASSIFY_EDGE);
	sgnode* n7 = new sgnode(nullptr, tor2);
	sgnode* n8 = new sgnode(csg, nullptr, carve::csg::CSG::UNION, n6, n7);
	n6->parent = n7->parent = n8;

	mesh_t* sphere = textured_ellipsoid(
		tex_coord_attr, mtl_id_attr, 2,
		{
			.transform = tmat_util::translation<space::OBJECT>(-3.f, 0, 0),
		});
	// out_mesh = csg.compute(out_mesh, sphere, carve::csg::CSG::UNION, nullptr, carve::csg::CSG::CLASSIFY_EDGE);
	sgnode* n9 = new sgnode(nullptr, sphere);
	sgnode* na = new sgnode(csg, nullptr, carve::csg::CSG::UNION, n8, n9);
	n8->parent = n9->parent = na;

	mesh_t* sphere2 = textured_ellipsoid(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.transform = tmat_util::translation<space::OBJECT>(-1.5f + c::EPSILON, -1.f, 0),
		});
	// out_mesh = csg.compute(out_mesh, sphere2, carve::csg::CSG::A_MINUS_B, nullptr, carve::csg::CSG::CLASSIFY_EDGE);
	m_sphere_node = new sgnode(nullptr, sphere2);
	m_sg = new sgnode(csg, nullptr, carve::csg::CSG::A_MINUS_B, na, m_sphere_node);
	na->parent = m_sphere_node->parent = m_sg;

	// sg->operation = carve::csg::CSG::OP::UNION;
	// sg->recompute(csg);

	/*delete sphere;
	delete sphere2;
	delete tor;
	delete tor2;
	delete cone;
	delete cyl;
	delete box_b;
	delete box_a;*/
	*out_mesh = m_sg->mesh;
}

void preview_layer::m_tesselate(mesh_t* in_scene, std::unordered_map<GLuint, std::vector<GLfloat>>& out_vtxs_for_mtl, attr_tex_coord_t tex_coord_attr, attr_material_t mtl_id_attr)
{
	for (auto& pair : out_vtxs_for_mtl)
		pair.second.clear();
	GLUtesselator* tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_BEGIN, (GLUTessCallback)tess_callback_begin);
	gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (GLUTessCallback)tess_callback_vertex_data);
	gluTessCallback(tess, GLU_TESS_EDGE_FLAG, (GLUTessCallback)tess_callback_edge_flag); // Edge flag forces only triangles
	gluTessCallback(tess, GLU_TESS_END, (GLUTessCallback)tess_callback_end);
	for (mesh_t::face_iter i = in_scene->faceBegin(); i != in_scene->faceEnd(); ++i)
	{
		mesh_t::face_t* f = *i;
		GLuint mtl_id = mtl_id_attr.getAttribute(f, 0);

		std::vector<tess_vtx> vtxs;
		for (mesh_t::face_t::edge_iter_t e = f->begin(); e != f->end(); ++e)
		{
			auto t = tex_coord_attr.getAttribute(f, e.idx());
			tess_vtx v;
			v.x = e->vert->v.x;
			v.y = e->vert->v.y;
			v.z = e->vert->v.z;
			v.u = t.u;
			v.v = t.v;
			v.target = &out_vtxs_for_mtl[mtl_id];
			vtxs.emplace_back(v);
		}

		gluTessBeginPolygon(tess, nullptr);
		gluTessBeginContour(tess);
		for (const tess_vtx& v : vtxs)
			gluTessVertex(tess, (GLdouble*)&v, (GLvoid*)&v);
		gluTessEndContour(tess);
		gluTessEndPolygon(tess);
	}
	gluDeleteTess(tess);
}

void preview_layer::m_compute_csg(
	std::unordered_map<GLuint, material_t>& out_mtls,
	std::unordered_map<GLuint, std::vector<GLfloat>>& out_vtxs_for_mtl)
{
	out_mtls.clear();
	out_vtxs_for_mtl.clear();

	m_ctx.tex_coord_attr.installHooks(m_ctx.csg);
	m_ctx.mtl_id_attr.installHooks(m_ctx.csg);

	mesh_t* in_scene = nullptr;
	m_make_scene(m_ctx.csg, m_ctx.tex_coord_attr, m_ctx.mtl_id_attr, &in_scene, out_mtls);
	for (auto it = out_mtls.begin(); it != out_mtls.end(); ++it)
		out_vtxs_for_mtl.insert(
			std::make_pair(it->first, std::vector<GLfloat>()));

	m_tesselate(in_scene, out_vtxs_for_mtl, m_ctx.tex_coord_attr, m_ctx.mtl_id_attr);
}
