#include "pch.h"
#include "camera.h"
#include "carve.h"
#include "glu_tess.h"

using namespace mgl;
using namespace hats;

static void make_scene(carve::csg::CSG& csg, attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, mesh_t*& out_mesh, std::unordered_map<GLuint, material_t>& out_mtls) {
	if (out_mesh) {
		delete out_mesh;
	}
	out_mtls.clear();

	material_t mtl1;
	out_mtls.insert(std::make_pair(1, mtl1));
	material_t mtl2;
	out_mtls.insert(std::make_pair(2, mtl2));

	mesh_t* cyl = textured_cylinder(tex_coord_attr, mtl_id_attr, 1,
		{
			.top_radius = .5f,
			.bottom_radius = .5f,
			.transform = tmat_util::scale<space::OBJECT>(1.f, 2.f, 1.f)
		}
	);
	out_mesh = cyl;
	mesh_t* cube = textured_cuboid(tex_coord_attr, mtl_id_attr, 2);
	out_mesh = csg.compute(cube, cyl, carve::csg::CSG::A_MINUS_B, nullptr, carve::csg::CSG::CLASSIFY_EDGE);

	delete cyl;
	delete cube;
}

static void tesselate(mesh_t* in_scene, std::unordered_map<GLuint, std::vector<GLfloat>>& out_vtxs_for_mtl, attr_tex_coord_t tex_coord_attr, attr_material_t mtl_id_attr)
{
	GLUtesselator* tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_BEGIN, (GLUTessCallback)tess_callback_begin);
	gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (GLUTessCallback)tess_callback_vertex_data);
	gluTessCallback(tess, GLU_TESS_EDGE_FLAG, (GLUTessCallback)tess_callback_edge_flag); // Edge flag forces only triangles
	gluTessCallback(tess, GLU_TESS_END, (GLUTessCallback)tess_callback_end);
	for (mesh_t::face_iter i = in_scene->faceBegin(); i != in_scene->faceEnd(); ++i) {
		mesh_t::face_t* f = *i;
		GLuint mtl_id = mtl_id_attr.getAttribute(f, 0);

		std::vector<tess_vtx> vtxs;
		for (mesh_t::face_t::edge_iter_t e = f->begin(); e != f->end(); ++e) {
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
		for (const tess_vtx& v : vtxs) {
			gluTessVertex(tess, (GLdouble*)&v, (GLvoid*)&v);
		}
		gluTessEndContour(tess);
		gluTessEndPolygon(tess);
	}
	gluDeleteTess(tess);
}

static void compute_csg(std::unordered_map<GLuint, material_t>& out_mtls, std::unordered_map<GLuint, std::vector<GLfloat>>& out_vtxs_for_mtl) {
	out_mtls.clear();
	out_vtxs_for_mtl.clear();

	carve::csg::CSG csg;
	attr_tex_coord_t tex_coord_attr;
	tex_coord_attr.installHooks(csg);
	attr_material_t mtl_id_attr;
	mtl_id_attr.installHooks(csg);

	mesh_t* in_scene = nullptr;
	make_scene(csg, tex_coord_attr, mtl_id_attr, in_scene, out_mtls);
	for (auto it = out_mtls.begin(); it != out_mtls.end(); ++it) {
		out_vtxs_for_mtl.insert(std::make_pair(it->first, std::vector<GLfloat>()));
	}

	tesselate(in_scene, out_vtxs_for_mtl, tex_coord_attr, mtl_id_attr);

	delete in_scene;
}

int main(int argc, char** argv)
{
	context c(1920, 1080, "PowerTranzphormR", true);
	c.set_clear_color(0, 0, 1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	ImGui_ImplGlfw_InitForOpenGL(c.window, true);
	ImGui_ImplOpenGL3_Init("#version 430 core");

	const shaders& shaders = shaders::from_files("src/glsl/csg.vert", "src/glsl/csg.frag");

	const point<space::WORLD> cam_pos(0, 0, 5);
	const direction<space::WORLD>& cam_dir = -direction_util::k_hat<space::WORLD>();
	const f32 ar = c.get_aspect_ratio();
	camera cam(cam_pos, cam_dir, 108 / ar, ar, .01f, 1000.f, 5.f);
	tmat<space::OBJECT, space::WORLD> obj;

	std::unordered_map<GLuint, material_t> mtls;
	std::unordered_map<GLuint, std::vector<GLfloat>> vtxs_for_mtl;
	compute_csg(mtls, vtxs_for_mtl);

	std::unordered_map<GLuint, static_vertex_array> vaos_for_mtl;
	for (auto it = vtxs_for_mtl.begin(); it != vtxs_for_mtl.end(); ++it) {
		static_vertex_array vao(it->second.data(), (u32)it->second.size(), { 3, 2 });
		vaos_for_mtl.emplace(it->first, std::move(vao));
	}

	stbi_set_flip_vertically_on_load(true);
	std::unordered_map<GLuint, texture2d_rgb_u8> texs_for_mtl;
	for (GLuint i = 1; i <= 2; ++i) {
		int tex_w = -1, tex_h = -1, tex_c = -1;
		stbi_uc* tex_data = stbi_load((std::string("res/") + std::to_string(i) + ".png").c_str(), &tex_w, &tex_h, &tex_c, 3);
		texture2d_rgb_u8 tex(GL_RGB, tex_w, tex_h, tex_data);
		stbi_image_free(tex_data);
		texs_for_mtl.emplace(i, std::move(tex));
	}


	/*bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);*/
	constexpr u32 keycodes[7] = { GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL };
	bool keys[7] = { false };
	while (c.is_running())
	{
		c.begin_frame();
		if (c.get_key(GLFW_KEY_ESCAPE))
			break;
		for (int i = 0; i < 7; i++)
			keys[i] = c.get_key(keycodes[i]);

		// printf("\n\nAAAA\n\n");
		// printf("%d | %d-%d %d-%d %d-%d\n", keys[6], keys[3], keys[1], keys[4], keys[5], keys[2], keys[0]);
		const direction<space::WORLD> move_dir(keys[3] - keys[1], keys[4] - keys[5], keys[2] - keys[0]);
		// move_dir.print();
		// printf("%f\n", spd);
		// cam.get_pos().print();
		/*constexpr f32 speeds[2] = { 1.f, 2.f };
		cam.move(c.time.delta, move_dir * speeds[keys[6]]);*/
		cam.move(c.time.delta, move_dir * (keys[6] ? 2.f : 1.f));
		// printf("\n\nBBBB\n\n");
		const mat<space::OBJECT, space::CLIP>& mvp = cam.get_view_proj() * obj;

		for (auto it = vaos_for_mtl.begin(); it != vaos_for_mtl.end(); ++it) {
			const texture2d_rgb_u8& tex = texs_for_mtl[it->first];
			tex.bind(0);
			const material_t mat = mtls[it->first];
			shaders.bind();
			shaders.uniform_1i("u_tex", 0);
			shaders.uniform_3f("u_col", mat.r, mat.g, mat.b);
			shaders.uniform_mat4("u_mvp", mvp.e);
			c.draw(it->second, shaders);
		}

		/*ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		if (ImGui::Begin("DOCKSPACE", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus))
		{
			ImGuiID dockspace_id = ImGui::GetID("DOCKSPACE");
			ImGui::DockSpace(dockspace_id, ImVec2(0.f, 0.f), ImGuiDockNodeFlags_None);
			if (show_demo_window)
				ImGui::ShowDemoWindow(&show_demo_window);
			ImGui::End();
		}

		ImGui::PopStyleVar(2);

		ImGui::Render();
		glDisable(GL_DEPTH_TEST);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
		glEnable(GL_DEPTH_TEST);*/

		c.end_frame();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	return 0;
}
