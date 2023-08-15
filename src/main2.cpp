#include "pch.h"
#include "imgui_layer.h"
#include "preview_layer.h"
#include "preview_window.h"

void make_scene(scene_ctx &out_scene) {
	//
	// FIXME
	//
	/*
	sgmaterial mtl1;
	materials.insert(std::make_pair(1, mtl1));
	sgmaterial mtl2;
	materials.insert(std::make_pair(2, mtl2));

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

	mesh_t* cone = textured_cone(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.radius = .5f,
			.height = 1.f,
			.transform = tmat_util::translation<space::OBJECT>(0.f, 1.5f, 0),
		});
	sgnode* n3 = new sgnode(nullptr, cone);
	sgnode* n4 = new sgnode(csg, nullptr, carve::csg::CSG::UNION, { n2, n3 });

	mesh_t* tor = textured_torus(tex_coord_attr, mtl_id_attr, 2,
		{
			.tube_radius = .5f,
			// .num_center_steps = 64,
			// .num_tube_steps = 64,
			.transform = tmat_util::translation<space::OBJECT>(1.f, c::EPSILON, 0),
		});
	m_tor_node = new sgnode(nullptr, tor);
	sgnode* n6 = new sgnode(csg, nullptr, carve::csg::CSG::A_MINUS_B, { n4, m_tor_node });

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

	mesh_t* sphere2 = textured_ellipsoid(
		tex_coord_attr, mtl_id_attr, 1,
		{
			.transform = tmat_util::translation<space::OBJECT>(-1.5f + c::EPSILON, -1.f, 0),
		});
	m_sphere_node = new sgnode(nullptr, sphere2);
	// std::vector<sgnode*> asdf = { na, m_sphere_node };
	sgnode* sg = new sgnode(csg, nullptr, carve::csg::CSG::A_MINUS_B, { na, m_sphere_node });

	sg->recompute(csg);



	mgl::retained_texture2d_rgb_u8* hm_tex = load_retained_texture_rgb_u8("res/hm.bmp");
	std::vector<mesh_t*> asdf;
	for (s32 i = 0; i < 10; i++)
	{
		mesh_t* heightmap = textured_heightmap(tex_coord_attr, mtl_id_attr, 2, hm_tex,
			{
				.max_height = 10.f,
				.width_steps = 10,
				.depth_steps = 10,
				.transform = tmat_util::translation<space::OBJECT>(i, -.25f, 0.f),
			});
		asdf.push_back(heightmap);
	}

	m_sg = new scene_graph(sg, asdf);
	// *out_mesh = m_sg->mesh;
	*/
}

int main(int argc, char** argv)
{
	mgl::context c(1280, 720, "PowerTranzphormR",
		{ .vsync = true,
			.clear = { .b = 1.f } });

	scene_ctx scene;
	make_scene(scene);

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
