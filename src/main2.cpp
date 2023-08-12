#include "pch.h"
#include "camera.h"
#include "carve.h"
#include "glu_tess.h"
#include "imgui_context.h"
#include "preview_window.h"
#include "scene_graph.h"

using namespace mgl;
using namespace hats;

static sgnode* sg = nullptr;
static void make_scene(carve::csg::CSG& csg, attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, mesh_t** out_mesh, std::unordered_map<GLuint, material_t>& out_mtls)
{
	material_t mtl1;
	out_mtls.insert(std::make_pair(1, mtl1));
	material_t mtl2;
	out_mtls.insert(std::make_pair(2, mtl2));


	mesh_t* cyl = textured_cylinder(
		tex_coord_attr, mtl_id_attr, 1,
		{ .top_radius = .5f,
		  .bottom_radius = .5f,
		  .transform = tmat_util::translation<space::OBJECT>(0, .5f, 0) *
					   tmat_util::scale<space::OBJECT>(1.5f, 1.f, 1.5f) }
	);
	sgnode n0(nullptr, cyl);

	mesh_t* box_b = textured_cuboid(
		tex_coord_attr, mtl_id_attr, 2,
		{ .width = 3.f,
		  .transform = tmat_util::translation<space::OBJECT>(0, -1.f, 0) }
	);
	sgnode n1(nullptr, box_b);

	// out_mesh = csg.compute(cyl, box_b, carve::csg::CSG::B_MINUS_A, nullptr, carve::csg::CSG::CLASSIFY_EDGE);
	sgnode n2(csg, nullptr, carve::csg::CSG::B_MINUS_A, &n0, &n1);
	n0.parent = n1.parent = &n2;

	mesh_t* cone = textured_cone(
		tex_coord_attr, mtl_id_attr, 1,
		{ .radius = .5f,
		  .height = 1.f,
		  .transform = tmat_util::translation<space::OBJECT>(0.f, 1.5f, 0) }
	);
	// out_mesh = csg.compute(out_mesh, cone, carve::csg::CSG::UNION, nullptr, carve::csg::CSG::CLASSIFY_EDGE);
	sgnode n3(nullptr, cone);
	sgnode n4(csg, nullptr, carve::csg::CSG::UNION, &n2, &n3);
	n2.parent = n3.parent = &n4;

	mesh_t* tor = textured_torus(
		tex_coord_attr, mtl_id_attr, 2,
		{ .tube_radius = .5f,
		  .num_tube_steps = 8,
		  .transform = tmat_util::translation<space::OBJECT>(1.f, 0, 0) }
	);
	// out_mesh = csg.compute(out_mesh, tor, carve::csg::CSG::A_MINUS_B, nullptr, carve::csg::CSG::CLASSIFY_EDGE);
	sgnode n5(nullptr, tor);
	sgnode n6(csg, nullptr, carve::csg::CSG::A_MINUS_B, &n4, &n5);
	n4.parent = n5.parent = &n6;

	mesh_t* tor2 = textured_torus(
		tex_coord_attr, mtl_id_attr, 1,
		{ .transform = tmat_util::translation<space::OBJECT>(3.f, 0, 0) }
	);
	// out_mesh = csg.compute(out_mesh, tor2, carve::csg::CSG::UNION, nullptr, carve::csg::CSG::CLASSIFY_EDGE);
	sgnode n7(nullptr, tor2);
	sgnode n8(csg, nullptr, carve::csg::CSG::UNION, &n6, &n7);
	n6.parent = n7.parent = &n8;

	mesh_t* sphere = textured_ellipsoid(
		tex_coord_attr, mtl_id_attr, 2,
		{ .transform = tmat_util::translation<space::OBJECT>(-3.f, 0, 0) }
	);
	// out_mesh = csg.compute(out_mesh, sphere, carve::csg::CSG::UNION, nullptr, carve::csg::CSG::CLASSIFY_EDGE);
	sgnode n9(nullptr, sphere);
	sgnode na(csg, nullptr, carve::csg::CSG::UNION, &n8, &n9);
	n8.parent = n9.parent = &na;

	mesh_t* sphere2 = textured_ellipsoid(
		tex_coord_attr, mtl_id_attr, 1,
		{ .transform = tmat_util::translation<space::OBJECT>(-1.5f, -1.f, 0) }
	);
	// out_mesh = csg.compute(out_mesh, sphere2, carve::csg::CSG::A_MINUS_B, nullptr, carve::csg::CSG::CLASSIFY_EDGE);
	sgnode nb(nullptr, sphere2);
	sg = new sgnode(csg, nullptr, carve::csg::CSG::A_MINUS_B, &na, &nb);
	na.parent = nb.parent = sg;

	/*delete sphere;
	delete sphere2;
	delete tor;
	delete tor2;
	delete cone;
	delete cyl;
	delete box_b;
	delete box_a;*/
	*out_mesh = sg->mesh;
}

static void tesselate(mesh_t* in_scene, std::unordered_map<GLuint, std::vector<GLfloat>>& out_vtxs_for_mtl, attr_tex_coord_t tex_coord_attr, attr_material_t mtl_id_attr)
{
	GLUtesselator* tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_BEGIN, (GLUTessCallback)tess_callback_begin);
	gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (GLUTessCallback)tess_callback_vertex_data);
	gluTessCallback(
		tess, GLU_TESS_EDGE_FLAG,
		(GLUTessCallback)
			tess_callback_edge_flag
	); // Edge flag forces only triangles
	gluTessCallback(tess, GLU_TESS_END, (GLUTessCallback)tess_callback_end);
	for (mesh_t::face_iter i = in_scene->faceBegin(); i != in_scene->faceEnd();
		 ++i)
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

static void compute_csg(
	std::unordered_map<GLuint, material_t>& out_mtls,
	std::unordered_map<GLuint, std::vector<GLfloat>>& out_vtxs_for_mtl
)
{
	out_mtls.clear();
	out_vtxs_for_mtl.clear();

	carve::csg::CSG csg;
	attr_tex_coord_t tex_coord_attr;
	tex_coord_attr.installHooks(csg);
	attr_material_t mtl_id_attr;
	mtl_id_attr.installHooks(csg);

	mesh_t* in_scene = nullptr;
	make_scene(csg, tex_coord_attr, mtl_id_attr, &in_scene, out_mtls);
	for (auto it = out_mtls.begin(); it != out_mtls.end(); ++it)
		out_vtxs_for_mtl.insert(
			std::make_pair(it->first, std::vector<GLfloat>())
		);

	tesselate(in_scene, out_vtxs_for_mtl, tex_coord_attr, mtl_id_attr);

	delete in_scene;
}

const std::vector<imgui_menu> construct_app_menus()
{
	std::vector<imgui_menu> result;

	imgui_menu file_menu;
	file_menu.name = "File";
	imgui_menu_item file_new = {
		"New Phonky Phorm",
		[]()
		{ std::cout << "MENU: NEW\n"; },
		"Ctrl+N",
		{ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_N }
	};
	imgui_menu_item file_open = {
		"Open Phonky Phorm",
		[]()
		{ std::cout << "MENU: OPEN\n"; },
		"Ctrl+O",
		{ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_O }
	};
	imgui_menu_item file_save = {
		"Save Phonky Phorm",
		[]()
		{ std::cout << "MENU: SAVE\n"; },
		"Ctrl+S",
		{ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_S }
	};
	imgui_menu_item file_save_as = {
		"Save Phonky Phorm As...",
		[]()
		{ std::cout << "MENU: SAVE AS\n"; },
		"Ctrl+Shift+S",
		{ GLFW_KEY_LEFT_CONTROL, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_S }
	};
	file_menu.groups.push_back({ file_new, file_open, file_save, file_save_as });
	result.push_back(file_menu);

	return result;
}

int main(int argc, char** argv)
{
	context c(1280, 720, "PowerTranzphormR", true);
	c.set_clear_color(0, 0, 1);
	imgui_context ic(c);
	ic.set_menus(construct_app_menus());
	c.add_layer(&ic);

	const shaders& shaders =
		shaders::from_files("src/glsl/csg.vert", "src/glsl/csg.frag");

	const point<space::WORLD> cam_pos(0, 0, 5);
	const f32 ar = c.get_aspect_ratio();
	camera cam(cam_pos, 0, 0, 108 / ar, ar, .01f, 1000.f, 5.f);
	tmat<space::OBJECT, space::WORLD> obj;

	std::unordered_map<GLuint, material_t> mtls;
	std::unordered_map<GLuint, std::vector<GLfloat>> vtxs_for_mtl;
	compute_csg(mtls, vtxs_for_mtl);

	std::unordered_map<GLuint, static_vertex_array> vaos_for_mtl;
	for (auto it = vtxs_for_mtl.begin(); it != vtxs_for_mtl.end(); ++it)
	{
		static_vertex_array vao(it->second.data(), (u32)it->second.size(), { 3, 2 });
		vaos_for_mtl.emplace(it->first, std::move(vao));
	}

	stbi_set_flip_vertically_on_load(true);
	std::unordered_map<GLuint, texture2d_rgb_u8> texs_for_mtl;
	for (GLuint i = 1; i <= 2; ++i)
	{
		int tex_w = -1, tex_h = -1, tex_c = -1;
		stbi_uc* tex_data = stbi_load(
			(std::string("res/") + std::to_string(i) + ".png").c_str(), &tex_w,
			&tex_h, &tex_c, 3
		);
		texture2d_rgb_u8 tex(GL_RGB, tex_w, tex_h, tex_data);
		stbi_image_free(tex_data);
		texs_for_mtl.emplace(i, std::move(tex));
	}

	u32 lastw = c.width, lasth = c.height;
	framebuffer_u8 framebuffer(c.width, c.height);
	preview_window preview(framebuffer);
	ic.add_window(&preview);

	constexpr u32 keycodes[11] = { GLFW_KEY_W,
								   GLFW_KEY_A,
								   GLFW_KEY_S,
								   GLFW_KEY_D,
								   GLFW_KEY_SPACE,
								   GLFW_KEY_LEFT_SHIFT,
								   GLFW_KEY_LEFT_CONTROL,
								   GLFW_KEY_LEFT,
								   GLFW_KEY_RIGHT,
								   GLFW_KEY_UP,
								   GLFW_KEY_DOWN };
	bool keys[11] = { false };

	constexpr u32 NSAMPLES = 128;
	f32 fps_samples[NSAMPLES] = { 0.f };
	s32 cur_sample = 0;
	f32 avg_fps = 0.f;

	while (c.is_running())
	{
		c.begin_frame();
		if (c.get_key(GLFW_KEY_ESCAPE))
			break;
		for (int i = 0; i < 11; i++)
			keys[i] = c.get_key(keycodes[i]);

		const f32 cur_fps = 1.f / c.time.delta;
		avg_fps -= fps_samples[cur_sample] / NSAMPLES;
		fps_samples[cur_sample] = cur_fps;
		avg_fps += fps_samples[cur_sample] / NSAMPLES;
		cur_sample = (cur_sample + 1) % NSAMPLES;
		std::string window_title = "PowerTranzphormR (" +
								   std::to_string((u32)std::round(avg_fps)) +
								   " FPS)";
		glfwSetWindowTitle(c.window, window_title.c_str());

		// const direction<space::CAMERA> move_dir(keys[3] - keys[1], keys[4] -
		// keys[5], keys[2] - keys[0]);
		const direction<space::CAMERA> move_dir(keys[3] - keys[1], 0, keys[2] - keys[0]);
		cam.move(c.time.delta, move_dir * (keys[6] ? 2.f : 1.f), c.mouse.delta.x, c.mouse.delta.y);
		const mat<space::OBJECT, space::CLIP>& mvp = cam.get_view_proj() * obj;

		// TODO better way to handle this?
		if (c.width != lastw || c.height != lasth)
		{
			lastw = c.width;
			lasth = c.height;
			framebuffer = framebuffer_u8(lastw, lasth);
		}
		// RENDER SCENE
		framebuffer.bind();
		glEnable(GL_DEPTH_TEST);
		c.clear();
		for (auto it = vaos_for_mtl.begin(); it != vaos_for_mtl.end(); ++it)
		{
			const texture2d_rgb_u8& tex = texs_for_mtl[it->first];
			tex.bind(0);
			const material_t mat = mtls[it->first];
			shaders.bind();
			shaders.uniform_1i("u_tex", 0);
			shaders.uniform_3f("u_col", mat.r, mat.g, mat.b);
			shaders.uniform_mat4("u_mvp", mvp.e);
			c.draw(it->second, shaders);
		}
		framebuffer.unbind();

		// RENDER UI
		c.handle_layers_for_frame();

		c.end_frame();
	}

	return 0;
}
