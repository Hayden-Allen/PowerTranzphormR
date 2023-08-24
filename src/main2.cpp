#include "pch.h"
#include "ui/imgui_layer.h"
#include "ui/preview_layer.h"
#include "ui/preview_window.h"
#include "ui/scene_graph_window.h"

sgnode* textured_cuboid_node(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cuboid_options& options = {})
{
	mesh_t* m = textured_cuboid(tex_coord_attr, mtl_id_attr, mtl_id, options);
	return new sgnode(nullptr, m, options.transform);
}
sgnode* textured_cylinder_node(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cylinder_options& options = {})
{
	mesh_t* m = textured_cylinder(tex_coord_attr, mtl_id_attr, mtl_id, options);
	return new sgnode(nullptr, m, options.transform);
}
sgnode* textured_cone_node(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cone_options& options = {})
{
	mesh_t* m = textured_cone(tex_coord_attr, mtl_id_attr, mtl_id, options);
	return new sgnode(nullptr, m, options.transform);
}
sgnode* textured_torus_node(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const torus_options& options = {})
{
	mesh_t* m = textured_torus(tex_coord_attr, mtl_id_attr, mtl_id, options);
	return new sgnode(nullptr, m, options.transform);
}
sgnode* textured_ellipsoid_node(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const ellipsoid_options& options = {})
{
	mesh_t* m = textured_ellipsoid(tex_coord_attr, mtl_id_attr, mtl_id, options);
	return new sgnode(nullptr, m, options.transform);
}

void make_scene(scene_ctx* const out_scene)
{
	mgl::shaders* s1 = new mgl::shaders("src/glsl/csg.vert", "src/glsl/csg.frag");
	auto t1 = load_texture_rgb_u8("res/1.png");
	auto t2 = load_texture_rgb_u8("res/2.png");
	scene_material mtl1("mtl1", { { "u_tex", t1 } }, s1);
	out_scene->add_material(mtl1);
	scene_material mtl2("mtl2", { { "u_tex", t2 } }, s1);
	out_scene->add_material(mtl2);

	auto& tex_coord_attr = out_scene->get_tex_coord_attr();
	auto& mtl_id_attr = out_scene->get_mtl_attr();
	auto& csg = out_scene->get_csg();

	/*mesh_t* cyl = textured_cylinder(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.top_radius = .5f,
			.bottom_radius = .5f,
			.transform = tmat_util::translation<space::OBJECT, space::WORLD>(0, .5f, 0) *
						 tmat_util::scale<space::OBJECT>(1.5f, 1.f, 1.5f),
		});
	sgnode* n0 = new sgnode(nullptr, cyl);*/
	sgnode* n0 = textured_cylinder_node(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.top_radius = .5f,
			.bottom_radius = .5f,
			.transform = tmat_util::translation<space::OBJECT, space::WORLD>(0, .5f, 0) *
						 tmat_util::scale<space::OBJECT>(1.5f, 1.f, 1.5f),
		});
	n0->name = "Cylinder";

	/*mesh_t* box_b = textured_cuboid(
		tex_coord_attr, mtl_id_attr, 2,
		{
			.width = 3.f,
			.transform = tmat_util::translation<space::OBJECT, space::WORLD>(0, -1.f, 0),
		});
	sgnode* n1 = new sgnode(nullptr, box_b);*/
	sgnode* n1 = textured_cuboid_node(
		tex_coord_attr, mtl_id_attr, 2,
		{
			.width = 3.f,
			.transform = tmat_util::translation<space::OBJECT, space::WORLD>(0, -1.f, 0),
		});
	n1->name = "Box";

	sgnode* n2 = new sgnode(csg, nullptr, carve::csg::CSG::A_MINUS_B, { n1, n0 });

	/*mesh_t* cone = textured_cone(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.radius = .5f,
			.height = 1.f,
			.num_steps = 8,
			.transform = tmat_util::translation<space::OBJECT, space::WORLD>(0.f, 1.5f, 0),
		});
	sgnode* cone_node = new sgnode(nullptr, cone);*/
	sgnode* cone_node = textured_cone_node(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.radius = .5f,
			.height = 1.f,
			.num_steps = 8,
			.transform = tmat_util::translation<space::OBJECT, space::WORLD>(0.f, 1.5f, 0),
		});
	cone_node->name = "Cone";
	sgnode* n4 = new sgnode(csg, nullptr, carve::csg::CSG::UNION, { n2, cone_node });

	/*mesh_t* tor2 = textured_torus(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.transform = tmat_util::translation<space::OBJECT, space::WORLD>(3.f, 0, 0),
		});
	sgnode* n7 = new sgnode(nullptr, tor2);*/
	sgnode* n7 = textured_torus_node(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.transform = tmat_util::translation<space::OBJECT, space::WORLD>(3.f, 0, 0),
		});
	n7->name = "Torus";

	/*mesh_t* sphere = textured_ellipsoid(
		tex_coord_attr, mtl_id_attr, 2,
		{
			.transform = tmat_util::translation<space::OBJECT, space::WORLD>(-3.f, 0, 0),
		});
	sgnode* n9 = new sgnode(nullptr, sphere);*/
	sgnode* n9 = textured_ellipsoid_node(
		tex_coord_attr, mtl_id_attr, 2,
		{
			.transform = tmat_util::translation<space::OBJECT, space::WORLD>(-3.f, 0, 0),
		});
	n9->name = "Sphere";
	/*mesh_t* cyl2 = textured_cylinder(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.top_radius = .5f,
			.bottom_radius = .5f,
			.num_steps = 8,
			.transform = tmat_util::translation<space::OBJECT, space::WORLD>(-1.5f, 1.5f, 0),
		});
	sgnode* cyl2_node = new sgnode(nullptr, cyl2);*/
	sgnode* cyl2_node = textured_cylinder_node(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.top_radius = .5f,
			.bottom_radius = .5f,
			.num_steps = 8,
			.transform = tmat_util::translation<space::OBJECT, space::WORLD>(-1.5f, 1.5f, 0),
		});
	cyl2_node->name = "Cylinder";
	sgnode* na = new sgnode(csg, nullptr, carve::csg::CSG::UNION, { n4, n7, n9, cyl2_node });

	/*mesh_t* sphere2 = textured_ellipsoid(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.transform = tmat_util::translation<space::OBJECT, space::WORLD>(-1.75f + c::EPSILON, -1.f, 0),
		});
	sgnode* sphere_node = new sgnode(nullptr, sphere2);*/
	sgnode* sphere_node = textured_ellipsoid_node(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.transform = tmat_util::translation<space::OBJECT, space::WORLD>(-1.75f + c::EPSILON, -1.f, 0),
		});
	sphere_node->name = "Sphere";
	// mesh_t* tor = textured_torus(
	//	tex_coord_attr, mtl_id_attr, 2,
	//	{
	//		.tube_radius = .5f,
	//		// .num_center_steps = 64,
	//		// .num_tube_steps = 64,
	//		.transform = tmat_util::translation<space::OBJECT, space::WORLD>(1.f - c::EPSILON, c::EPSILON, 0),
	//	});
	// sgnode* tor_node = new sgnode(nullptr, tor);
	sgnode* tor_node = textured_torus_node(
		tex_coord_attr, mtl_id_attr, 2,
		{
			.tube_radius = .5f,
			// .num_center_steps = 64,
			// .num_tube_steps = 64,
			.transform = tmat_util::translation<space::OBJECT, space::WORLD>(1.f - c::EPSILON, c::EPSILON, 0),
		});
	tor_node->name = "Torus";

	sgnode* sg = out_scene->get_sg_root();
	sg->set_operation(carve::csg::CSG::A_MINUS_B);
	sg->add_child(na);
	sg->add_child(sphere_node);
	sg->add_child(tor_node);

	/*mgl::shaders* s2 = new mgl::shaders("src/glsl/csg_hm.vert", "src/glsl/csg_hm.frag");
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
	}*/
}

int main(int argc, char** argv)
{
	mgl::context c(1280, 720, "PowerTranzphormR",
		{ .vsync = true,
			.clear = { .b = 1.f } });

	scene_ctx scene;
	make_scene(&scene);

	imgui_layer il(&c);
	c.add_layer(&il);

	const f32 ar = c.get_aspect_ratio();
	preview_layer pl(&c, &scene);
	pl.disable();
	pl.set_disable_callback([&]()
		{
			c.unlock_cursor();
			il.enable();
		});
	pl.set_enable_callback([&]()
		{
			c.lock_cursor();
			il.disable();
		});
	c.add_layer(&pl);

	preview_window preview(c, &pl);
	il.add_window(&preview);

	scene_graph_window sg_window(&scene);
	il.add_window(&sg_window);

	while (c.is_running())
	{
		c.begin_frame();

		char buf[64] = { 0 };
		sprintf_s(buf, "PowerTranzphormR (%u FPS)", (u32)std::round(c.avg_fps));
		c.set_title(buf);

		c.update_layers();
		c.end_frame();
	}

	return 0;
}
