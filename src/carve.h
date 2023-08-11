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
struct material_t
{
	GLfloat r = 1.0f, g = 1.0f, b = 1.0f;
};

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
	float width = 2.f, height = 2.f, depth = 2.f;
	tmat<space::OBJECT, space::OBJECT> transform = tmat<space::OBJECT, space::OBJECT>();
};
struct cylinder_options
{
	float top_radius = 1.f, bottom_radius = 1.f, height = 2.f;
	uint32_t num_steps = 16;
	tmat<space::OBJECT, space::OBJECT> transform = tmat<space::OBJECT, space::OBJECT>();
};
struct cone_options
{
	float radius = 2.f, height = 2.f;
	uint32_t num_steps = 16;
	tmat<space::OBJECT, space::OBJECT> transform = tmat<space::OBJECT, space::OBJECT>();
};
struct torus_options
{
	float center_radius = 1.f, tube_radius = .5f;
	uint32_t num_center_steps = 8, num_tube_steps = 8;
	// carve::math::Matrix transform = carve::math::Matrix::IDENT();
	tmat<space::OBJECT, space::OBJECT> transform = tmat<space::OBJECT, space::OBJECT>();
};
struct ellipsoid_options
{
	float radius_x = 1.f, radius_y = 1.f, radius_z = 1.f;
	uint32_t num_horizontal_steps = 12, num_vertical_steps = 6;
	tmat<space::OBJECT, space::OBJECT> transform = tmat<space::OBJECT, space::OBJECT>();
};

mesh_t* textured_cuboid(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cuboid_options& options = {});
mesh_t* textured_cylinder(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cylinder_options& options = {});
mesh_t* textured_cone(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cone_options& options = {});
mesh_t* textured_torus(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const torus_options& options = {});
mesh_t* textured_ellipsoid(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const ellipsoid_options& options = {});
