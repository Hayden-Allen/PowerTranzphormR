#pragma once
#include "pch.h"
#include "util/color.h"

class scene_ctx;

struct tex_coord_t
{
	f32 u, v, uo, vo;
	tex_coord_t() :
		u(0.0f), v(0.0f), uo(0.f), vo(0.f)
	{}
	tex_coord_t(const f32 s, const f32 t) :
		u(s), v(t), uo(0.f), vo(0.f)
	{}
	tex_coord_t(const f32 s, const f32 t, const f32 r, const f32 q) :
		u(s), v(t), uo(r), vo(q)
	{}
};
static tex_coord_t operator*(const f64 s, const tex_coord_t& t)
{
	return tex_coord_t(t.u * (f32)s, t.v * (f32)s, t.uo * (f32)s, t.vo * (f32)s);
}
static tex_coord_t& operator+=(tex_coord_t& t1, const tex_coord_t& t2)
{
	t1.u += t2.u;
	t1.v += t2.v;
	t1.uo += t2.uo;
	t1.vo += t2.vo;
	return t1;
}

typedef carve::mesh::MeshSet<3> mesh_t;
typedef mesh_t::vertex_t vertex_t;
typedef mesh_t::face_t face_t;
typedef carve::interpolate::FaceVertexAttr<tex_coord_t> attr_tex_coord_t;
typedef carve::interpolate::FaceVertexAttr<f64> attr_tex_weight_t;
typedef carve::interpolate::FaceVertexAttr<color_t> attr_tex_color_t;
typedef carve::interpolate::FaceAttr<GLuint> attr_material_t;

template<space SPACE>
static carve::geom3d::Vector hats2carve(const point<SPACE>& p)
{
	return carve::geom::VECTOR(p.x, p.y, p.z);
}
mesh_t* carve_clone(const mesh_t* const mesh, scene_ctx* const scene, const tmat<space::WORLD, space::OBJECT>& inv_mat = tmat<space::WORLD, space::OBJECT>());

struct primitive_options
{
	tmat<space::OBJECT, space::PARENT> transform;
	f32 u0 = 1.f, v0 = 1.f, uo0 = 0.f, vo0 = 0.f;
	f32 u1 = 1.f, v1 = 1.f, uo1 = 0.f, vo1 = 0.f;
	f32 u2 = 1.f, v2 = 1.f, uo2 = 0.f, vo2 = 0.f;
	f32 u3 = 1.f, v3 = 1.f, uo3 = 0.f, vo3 = 0.f;
	f32 w0 = 1.f, w1 = 0.f, w2 = 0.f, w3 = 0.f;
	f32 r = 0.f, g = 0.f, b = 0.f, a = 0.f;
};
struct cuboid_options : public primitive_options
{};
struct ellipsoid_options : public primitive_options
{
	u32 num_horizontal_steps = 12, num_vertical_steps = 6;
	f32 noise = 0.f;
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
	u32 width_steps = 2, depth_steps = 2;
};

struct carve_vert_attrs
{
	attr_tex_coord_t uv0, uv1, uv2, uv3;
	attr_tex_weight_t w0, w1, w2, w3;
	attr_tex_color_t color;

	void install_hooks(carve::csg::CSG& csg)
	{
		uv0.installHooks(csg);
		uv1.installHooks(csg);
		uv2.installHooks(csg);
		uv3.installHooks(csg);
		w0.installHooks(csg);
		w1.installHooks(csg);
		w2.installHooks(csg);
		w3.installHooks(csg);
		color.installHooks(csg);
	}
	/*void set_attribute(const face_t* const face, const u32 i, const tex_coord_t& _uv0, const tex_coord_t& _uv1, const tex_coord_t& _uv2, const tex_coord_t& _uv3, const f64 _w0, const f64 _w1, const f64 _w2, const f64 _w3, const color_t& c)
	{
		uv0.setAttribute(face, i, _uv0);
		uv1.setAttribute(face, i, _uv1);
		uv2.setAttribute(face, i, _uv2);
		uv3.setAttribute(face, i, _uv3);
		w0.setAttribute(face, i, _w0);
		w1.setAttribute(face, i, _w1);
		w2.setAttribute(face, i, _w2);
		w3.setAttribute(face, i, _w3);
		color.setAttribute(face, i, c);
	}*/
	void set_attribute(const face_t* const face, const u32 i, const primitive_options& opts, const tex_coord_t& mask)
	{
		uv0.setAttribute(face, i, tex_coord_t(mask.u * opts.u0, mask.v * opts.v0, opts.uo0, opts.vo0));
		uv1.setAttribute(face, i, tex_coord_t(mask.u * opts.u1, mask.v * opts.v1, opts.uo1, opts.vo1));
		uv2.setAttribute(face, i, tex_coord_t(mask.u * opts.u2, mask.v * opts.v2, opts.uo2, opts.vo2));
		uv3.setAttribute(face, i, tex_coord_t(mask.u * opts.u3, mask.v * opts.v3, opts.uo3, opts.vo3));
		w0.setAttribute(face, i, opts.w0);
		w1.setAttribute(face, i, opts.w1);
		w2.setAttribute(face, i, opts.w2);
		w3.setAttribute(face, i, opts.w3);
		color.setAttribute(face, i, color_t(opts.r, opts.g, opts.b, opts.a));
	}
};

mesh_t* textured_cuboid(carve_vert_attrs& vert_attrs, attr_material_t& mtl_id_attr, const GLuint mtl_id, const cuboid_options& options = {});
mesh_t* textured_cylinder(carve_vert_attrs& vert_attrs, attr_material_t& mtl_id_attr, const GLuint mtl_id, const cylinder_options& options = {});
mesh_t* textured_cone(carve_vert_attrs& vert_attrs, attr_material_t& mtl_id_attr, const GLuint mtl_id, const cone_options& options = {});
mesh_t* textured_torus(carve_vert_attrs& vert_attrs, attr_material_t& mtl_id_attr, const GLuint mtl_id, const torus_options& options = {});
mesh_t* textured_ellipsoid(carve_vert_attrs& vert_attrs, attr_material_t& mtl_id_attr, const GLuint mtl_id, const ellipsoid_options& options = {});
mesh_t* textured_heightmap(carve_vert_attrs& vert_attrs, attr_material_t& mtl_id_attr, const GLuint mtl_id, const heightmap_options& options = {});