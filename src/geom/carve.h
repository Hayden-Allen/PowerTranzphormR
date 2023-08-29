#pragma once
#include "pch.h"

class scene_ctx;

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
mesh_t* carve_clone(const mesh_t* const mesh, scene_ctx* const scene);

struct primitive_options
{
	tmat<space::OBJECT, space::PARENT> transform;
	f32 u_scale = 1.f, v_scale = 1.f;
};
struct cuboid_options : public primitive_options
{};
struct ellipsoid_options : public primitive_options
{
	u32 num_horizontal_steps = 12, num_vertical_steps = 6;
};
struct cylinder_options : public primitive_options
{
	f32 top_radius = .5f, bottom_radius = .5f;
	u32 num_steps = 16;
};
struct cone_options : public primitive_options
{
	u32 num_steps = 16;
};
struct torus_options : public primitive_options
{
	f32 center_radius = 1.f / 3.f, tube_radius = .5f - 1.f / 3.f;
	u32 num_center_steps = 12, num_tube_steps = 12;
};
struct heightmap_options : public primitive_options
{
	u32 width_steps = 0, depth_steps = 0;
};

mesh_t* textured_cuboid(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cuboid_options& options = {});
mesh_t* textured_cylinder(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cylinder_options& options = {});
mesh_t* textured_cone(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const cone_options& options = {});
mesh_t* textured_torus(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const torus_options& options = {});
mesh_t* textured_ellipsoid(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const ellipsoid_options& options = {});
mesh_t* textured_heightmap(attr_tex_coord_t& tex_coord_attr, attr_material_t& mtl_id_attr, GLuint mtl_id, const mgl::retained_texture2d_rgb_u8* const map, const heightmap_options& options = {});