#include "pch.h"
#include "ui/shortcut_menus_layer.h"
#include "ui/imgui_layer.h"
#include "ui/preview_layer.h"
#include "ui/preview_window.h"
#include "ui/scene_graph_window.h"
#include "ui/materials_list_window.h"
#include "ui/properties_window.h"
#include "ui/app_ctx.h"
#include "core/scene_material.h"
#include "ui/vertex_editor_window.h"


// static sgnode* textured_cuboid_node(scene_ctx* const scene, GLuint mtl_id, const cuboid_options& options = {})
//{
//	generated_mesh* m = scene->generated_textured_cuboid(mtl_id, options);
//	return new sgnode(nullptr, m, "Box", options.transform);
// }
// static sgnode* textured_ellipsoid_node(scene_ctx* const scene, GLuint mtl_id, const ellipsoid_options& options = {})
//{
//	generated_mesh* m = scene->generated_textured_ellipsoid(mtl_id, options);
//	return new sgnode(nullptr, m, "Ellipsoid", options.transform);
// }
// static sgnode* textured_cylinder_node(scene_ctx* const scene, GLuint mtl_id, const cylinder_options& options = {})
//{
//	generated_mesh* m = scene->generated_textured_cylinder(mtl_id, options);
//	return new sgnode(nullptr, m, "Cylinder", options.transform);
// }
// static sgnode* textured_cone_node(scene_ctx* const scene, GLuint mtl_id, const cone_options& options = {})
//{
//	generated_mesh* m = scene->generated_textured_cone(mtl_id, options);
//	return new sgnode(nullptr, m, "Cone", options.transform);
// }
// static sgnode* textured_torus_node(scene_ctx* const scene, GLuint mtl_id, const torus_options& options = {})
//{
//	generated_mesh* m = scene->generated_textured_torus(mtl_id, options);
//	return new sgnode(nullptr, m, "Torus", options.transform);
// }

/*
void make_scene(scene_ctx* const out_scene)
{
	mgl::shaders* s1 = new mgl::shaders("src/glsl/csg.vert", "src/glsl/csg.frag");
	auto t1 = load_texture_rgb_u8("res/1.png");
	scene_material* mtl1 = new scene_material;
	mtl1->name = "Material 1";
	mtl1->shaders = s1;
	mtl1->texs = { { "u_tex", t1 } };
	out_scene->add_material(mtl1);

	auto& csg = out_scene->get_csg();

	sgnode* n1 = textured_cuboid_node(
		out_scene, 1,
		{});

	sgnode* n2 = textured_ellipsoid_node(
		out_scene, 1,
		{ .transform = tmat_util::translation<space::OBJECT, space::PARENT>(0.0f, 0.5f, 0.0f) });

	sgnode* na = new sgnode(csg, nullptr, carve::csg::CSG::A_MINUS_B);
	na->add_child(n1);
	na->add_child(n2);

	sgnode* sg = out_scene->get_sg_root();
	sg->add_child(na);

	mgl::shaders* s2 = new mgl::shaders("src/glsl/csg_hm.vert", "src/glsl/csg_hm.frag");
	mgl::retained_texture2d_rgb_u8* hm_tex = load_retained_texture_rgb_u8("res/hm.bmp");
	mgl::texture2d_rgb_u8* t3 = load_texture_rgb_u8("res/3.png");
	scene_material mtl3("mtl3", { { "u_tex", t3 }, { "u_heightmap", hm_tex } }, s2);
	out_scene->add_material(mtl3);
	for (s32 i = 0; i < 10; i++)
	{
		mesh_t* heightmap = textured_heightmap(tex_coord_attr, mtl_id_attr, 3, hm_tex,
			{
				.max_height = 10.f,
				.width_steps = 10,
				.depth_steps = 10,
				.transform = tmat_util::translation<space::OBJECT>(i, -2, 2),
			});
		out_scene->add_heightmap(heightmap);
	}
}
*/

int main(int argc, char** argv)
{
	app_ctx a_ctx;

	shortcut_menus_layer sl(&a_ctx);
	a_ctx.mgl_ctx.add_layer(&sl);

	imgui_layer il(&a_ctx);
	a_ctx.mgl_ctx.add_layer(&il);

	preview_layer pl(&a_ctx);
	pl.disable();
	pl.set_disable_callback([&]()
		{
			a_ctx.mgl_ctx.unlock_cursor();
			il.enable();
		});
	pl.set_enable_callback([&]()
		{
			a_ctx.mgl_ctx.lock_cursor();
			il.disable();
		});
	a_ctx.mgl_ctx.add_layer(&pl);

	preview_window preview(&a_ctx);
	preview.set_enable_callback([&]()
		{
			pl.enable();
		});
	il.add_window(&preview);

	materials_list_window ml_window(&a_ctx);
	il.add_window(&ml_window);
	a_ctx.set_mtls_window(&ml_window);

	scene_graph_window sg_window(&a_ctx);
	il.add_window(&sg_window);
	a_ctx.set_sg_window(&sg_window);

	properties_window prop_window(&a_ctx);
	il.add_window(&prop_window);

	vertex_editor_window vert_window(&a_ctx);
	il.add_window(&vert_window);


	while (true)
	{
		if (!a_ctx.mgl_ctx.is_running())
		{
			if (a_ctx.confirm_unsaved_changes())
			{
				break;
			}
			else
			{
				glfwSetWindowShouldClose(a_ctx.mgl_ctx.window, false);
			}
		}

		a_ctx.mgl_ctx.begin_frame();

		char buf[64] = { 0 };
		sprintf_s(buf, "PowerTranzphormR (%u FPS)", (u32)std::round(a_ctx.mgl_ctx.avg_fps));
		a_ctx.mgl_ctx.set_title(buf);

		a_ctx.mgl_ctx.update_layers();
		a_ctx.mgl_ctx.end_frame();
	}

	return 0;
}
