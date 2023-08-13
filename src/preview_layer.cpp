#include "pch.h"
#include "preview_layer.h"

using namespace mgl;
using namespace hats;

preview_layer::preview_layer(const mgl::context* const mgl_context, const camera& cam) :
	m_mgl_context(mgl_context),
	m_shaders(shaders::from_files("src/glsl/csg.vert", "src/glsl/csg.frag")),
	m_fb(mgl_context->get_width(), mgl_context->get_height()),
	m_cam(cam)
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
	delete m_sg;
}



void preview_layer::on_frame(const f32 dt)
{
	const f32 tdx = 1.f * get_key(GLFW_KEY_RIGHT) - get_key(GLFW_KEY_LEFT);
	const f32 tdy = 1.f * get_key(GLFW_KEY_UP) - get_key(GLFW_KEY_DOWN);
	if (tdx != 0 || tdy != 0)
	{
		const f32 dt = m_mgl_context->time.delta;
		m_tor_node->transform(m_ctx.csg, carve::math::Matrix::TRANS(tdx * dt, tdy * dt, 0));
		// sphere_node->transform(ctx.csg, carve::math::Matrix::TRANS(tdx * c.time.delta, 0, 0));
		m_tesselate(m_sg->mesh, m_vtxs_for_mtl, m_ctx.tex_coord_attr, m_ctx.mtl_id_attr);
		m_vaos_for_mtl.clear();
		for (auto it = m_vtxs_for_mtl.begin(); it != m_vtxs_for_mtl.end(); ++it)
		{
			static_vertex_array vao(it->second.data(), (u32)it->second.size(), { 3, 2 });
			m_vaos_for_mtl.emplace(it->first, std::move(vao));
		}
	}

	const direction<space::CAMERA> move_dir(
		get_key(GLFW_KEY_D) - get_key(GLFW_KEY_A),
		get_key(GLFW_KEY_SPACE) - get_key(GLFW_KEY_LEFT_CONTROL),
		get_key(GLFW_KEY_S) - get_key(GLFW_KEY_W));
	const auto& mouse_delta = get_mouse().delta;
	m_cam.move(dt, move_dir * (get_key(GLFW_KEY_LEFT_SHIFT) ? .2f : 1.f), mouse_delta.x, mouse_delta.y);


	const tmat<space::OBJECT, space::WORLD> obj;
	const mat<space::OBJECT, space::CLIP>& mvp = m_cam.get_view_proj() * obj;

	m_fb.bind();
	glEnable(GL_DEPTH_TEST);
	m_mgl_context->clear();
	for (auto it = m_vaos_for_mtl.begin(); it != m_vaos_for_mtl.end(); ++it)
	{
		const texture2d_rgb_u8& tex = m_texs_for_mtl[it->first];
		tex.bind(0);
		const material_t mat = m_mtls[it->first];
		m_shaders.bind();
		m_shaders.uniform_1i("u_tex", 0);
		m_shaders.uniform_3f("u_col", mat.r, mat.g, mat.b);
		m_shaders.uniform_mat4("u_mvp", mvp.e);
		m_mgl_context->draw(it->second, m_shaders);
	}
	m_fb.unbind();
}

void preview_layer::on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods)
{
	if (key == GLFW_KEY_ESCAPE)
		disable();
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

	sgnode* n2 = new sgnode(csg, nullptr, carve::csg::CSG::A_MINUS_B, { n1, n0 });
	n0->parent = n1->parent = n2;

	mesh_t* cone = textured_cone(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.radius = .5f,
			.height = 1.f,
			.transform = tmat_util::translation<space::OBJECT>(0.f, 1.5f, 0),
		});
	sgnode* n3 = new sgnode(nullptr, cone);
	sgnode* n4 = new sgnode(csg, nullptr, carve::csg::CSG::UNION, { n2, n3 });
	n2->parent = n3->parent = n4;

	mesh_t* tor = textured_torus(tex_coord_attr, mtl_id_attr, 2,
		{
			.tube_radius = .5f,
			// .num_center_steps = 64,
			// .num_tube_steps = 64,
			.transform = tmat_util::translation<space::OBJECT>(1.f, c::EPSILON, 0),
		});
	m_tor_node = new sgnode(nullptr, tor);
	sgnode* n6 = new sgnode(csg, nullptr, carve::csg::CSG::A_MINUS_B, { n4, m_tor_node });
	n4->parent = m_tor_node->parent = n6;

	mesh_t* tor2 = textured_torus(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.transform = tmat_util::translation<space::OBJECT>(3.f, 0, 0),
		});
	sgnode* n7 = new sgnode(nullptr, tor2);

	mesh_t* sphere = textured_ellipsoid(
		tex_coord_attr, mtl_id_attr, 2,
		{
			.transform = tmat_util::translation<space::OBJECT>(-3.f, 0, 0),
		});
	sgnode* n9 = new sgnode(nullptr, sphere);
	sgnode* na = new sgnode(csg, nullptr, carve::csg::CSG::UNION, { n6, n7, n9 });
	n6->parent = n7->parent = n9->parent = na;

	mesh_t* sphere2 = textured_ellipsoid(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.transform = tmat_util::translation<space::OBJECT>(-1.5f + c::EPSILON, -1.f, 0),
		});
	m_sphere_node = new sgnode(nullptr, sphere2);
	m_sg = new sgnode(csg, nullptr, carve::csg::CSG::A_MINUS_B, { na, m_sphere_node });
	na->parent = m_sphere_node->parent = m_sg;

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
