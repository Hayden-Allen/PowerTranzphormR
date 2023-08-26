#pragma once
#include "pch.h"


struct tex_coord_t
{
	f32 u, v;
	tex_coord_t() :
		u(0.0f), v(0.0f) {}
	tex_coord_t(const f32 s, const f32 t) :
		u(s), v(t) {}
};
static tex_coord_t operator*(const f64 s, const tex_coord_t& t)
{
	return tex_coord_t(t.u * (float)s, t.v * (float)s);
}
static tex_coord_t& operator+=(tex_coord_t& t1, const tex_coord_t& t2)
{
	t1.u += t2.u;
	t1.v += t2.v;
	return t1;
}

typedef carve::mesh::MeshSet<3> mesh_t;
typedef mesh_t::vertex_t vertex_t;
typedef mesh_t::face_t face_t;
typedef carve::interpolate::FaceVertexAttr<tex_coord_t> attr_tex_coord_t;
typedef carve::interpolate::FaceAttr<GLuint> attr_material_t;

template<space SPACE>
static carve::geom3d::Vector hats2carve(const point<SPACE>& p)
{
	return carve::geom::VECTOR(p.x, p.y, p.z);
}

struct cuboid_options
{
	f32 width = 2.f, height = 2.f, depth = 2.f;
	tmat<space::OBJECT, space::PARENT> transform;
};
struct ellipsoid_options
{
	f32 radius_x = 1.f, radius_y = 1.f, radius_z = 1.f;
	u32 num_horizontal_steps = 12, num_vertical_steps = 6;
	tmat<space::OBJECT, space::PARENT> transform;
};
struct cylinder_options
{
	f32 top_radius = 1.f, bottom_radius = 1.f, height = 2.f;
	u32 num_steps = 16;
	tmat<space::OBJECT, space::PARENT> transform;
};
struct cone_options
{
	f32 radius = 2.f, height = 2.f;
	u32 num_steps = 16;
	tmat<space::OBJECT, space::PARENT> transform;
};
struct torus_options
{
	f32 center_radius = 1.f, tube_radius = .5f;
	u32 num_center_steps = 12, num_tube_steps = 12;
	tmat<space::OBJECT, space::PARENT> transform;
};
struct heightmap_options
{
	f32 width = 1.f, max_height = 1.f, depth = 1.f;
	u32 width_steps = 0, depth_steps = 0;
	tmat<space::OBJECT, space::PARENT> transform;
};

mesh_t* textured_cuboid(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cuboid_options& options = {});
mesh_t* textured_cylinder(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cylinder_options& options = {});
mesh_t* textured_cone(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cone_options& options = {});
mesh_t* textured_torus(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const torus_options& options = {});
mesh_t* textured_ellipsoid(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const ellipsoid_options& options = {});
mesh_t* textured_heightmap(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const mgl::retained_texture2d_rgb_u8* const map, const heightmap_options& options = {});