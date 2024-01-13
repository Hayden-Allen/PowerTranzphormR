#pragma once
#include "pch.h"
#include "geom/carve.h"
#if defined(GLU_TESS_CALLBACK_VARARGS)
typedef GLvoid(__stdcall* GLUTessCallback)(...);
#else
typedef void(__stdcall* GLUTessCallback)();
#endif

struct mesh_vertex
{
	f32 x = 0.f, y = 0.f, z = 0.f;
	f32 nx = 0.f, ny = 0.f, nz = 0.f;
	f32 u0 = 1.f, v0 = 1.f, uo0 = 0.f, vo0 = 0.f;
	f32 u1 = 1.f, v1 = 1.f, uo1 = 0.f, vo1 = 0.f;
	f32 u2 = 1.f, v2 = 1.f, uo2 = 0.f, vo2 = 0.f;
	f32 u3 = 1.f, v3 = 1.f, uo3 = 0.f, vo3 = 0.f;
	f32 w0 = 0.f, w1 = 0.f, w2 = 0.f, w3 = 0.f;
	f32 r = 0.f, g = 0.f, b = 0.f, a = 0.f;

	mesh_vertex() {}
	mesh_vertex(const f32 _x, const f32 _y, const f32 _z, const tex_coord_t& uv0, const tex_coord_t& uv1, const tex_coord_t& uv2, const tex_coord_t& uv3, const f32 _w0, const f32 _w1, const f32 _w2, const f32 _w3, const color_t& color) :
		x(_x), y(_y), z(_z),
		u0(uv0.u), v0(uv0.v), uo0(uv0.uo), vo0(uv0.vo),
		u1(uv1.u), v1(uv1.v), uo1(uv1.uo), vo1(uv1.vo),
		u2(uv2.u), v2(uv2.v), uo2(uv2.uo), vo2(uv2.vo),
		u3(uv3.u), v3(uv3.v), uo3(uv3.uo), vo3(uv3.vo),
		w0(_w0), w1(_w1), w2(_w2), w3(_w3),
		r(color.r), g(color.g), b(color.b), a(color.a)
	{}

	bool operator==(const mesh_vertex& o) const
	{
		return x == o.x && y == o.y && z == o.z &&
			   nx == o.nx && ny == o.ny && nz == o.nz &&
			   u0 == o.u0 && v0 == o.v0 &&
			   u1 == o.u1 && v1 == o.v1 &&
			   u2 == o.u2 && v2 == o.v2 &&
			   u3 == o.u3 && v3 == o.v3 &&
			   w0 == o.w0 && w1 == o.w1 && w2 == o.w2 && w3 == o.w3 &&
			   r == o.r && g == o.g && b == o.b && a == o.a;
	}
	std::string hash_all() const
	{
		char buf[512] = { 0 };
		sprintf_s(buf, 512, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", x, y, z, nx, ny, nz, u0, v0, uo0, vo0, u1, v1, uo1, vo1, u2, v2, uo2, vo2, u3, v3, uo3, vo3, w0, w1, w2, w3, r, g, b, a);
		return buf;
	}
	std::string hash_pos() const
	{
		char buf[512] = { 0 };
		sprintf_s(buf, 512, "%f %f %f", x, y, z);
		return buf;
	}
	void transform(const tmat<space::OBJECT, space::WORLD>& mat)
	{
		const tmat<space::OBJECT, space::WORLD>& normal_mat = mat.invert_copy().transpose_copy();
		const point<space::OBJECT> op(x, y, z);
		const direction<space::OBJECT> on(nx, ny, nz);
		const point<space::WORLD>& wp = op.transform_copy(mat);
		const direction<space::WORLD>& wn = on.transform_copy(normal_mat);
		x = wp.x;
		y = wp.y;
		z = wp.z;
		nx = wn.x;
		ny = wn.y;
		nz = wn.z;
	}
	void apply_uv_offset(const tex_coord_t* const off)
	{
		u0 *= off[0].u;
		v0 *= off[0].v;
		uo0 += off[0].uo;
		vo0 += off[0].vo;
		u1 *= off[1].u;
		v1 *= off[1].v;
		uo1 += off[1].uo;
		vo1 += off[1].vo;
		u2 *= off[2].u;
		v2 *= off[2].v;
		uo2 += off[2].uo;
		vo2 += off[2].vo;
		u3 *= off[3].u;
		v3 *= off[3].v;
		uo3 += off[3].uo;
		vo3 += off[3].vo;
	}
};
struct tess_vtx
{
	f64 x = 0.0, y = 0.0, z = 0.0;
	f32 u0 = 1.f, v0 = 1.f, uo0 = 0.f, vo0 = 0.f;
	f32 u1 = 1.f, v1 = 1.f, uo1 = 0.f, vo1 = 0.f;
	f32 u2 = 1.f, v2 = 1.f, uo2 = 0.f, vo2 = 0.f;
	f32 u3 = 1.f, v3 = 1.f, uo3 = 0.f, vo3 = 0.f;
	f64 w0 = 0.f, w1 = 0.f, w2 = 0.f, w3 = 0.f;
	f32 r = 0.f, g = 0.f, b = 0.f, a = 0.f;
	std::vector<mesh_vertex>* target = nullptr;

	tess_vtx() {}
	tess_vtx(const f64 _x, const f64 _y, const f64 _z, const tex_coord_t& uv0, const tex_coord_t& uv1, const tex_coord_t& uv2, const tex_coord_t& uv3, const f64 _w0, const f64 _w1, const f64 _w2, const f64 _w3, const color_t& color) :
		x(_x), y(_y), z(_z),
		u0(uv0.u), v0(uv0.v), uo0(uv0.uo), vo0(uv0.vo),
		u1(uv1.u), v1(uv1.v), uo1(uv1.uo), vo1(uv1.vo),
		u2(uv2.u), v2(uv2.v), uo2(uv2.uo), vo2(uv2.vo),
		u3(uv3.u), v3(uv3.v), uo3(uv3.uo), vo3(uv3.vo),
		w0(_w0), w1(_w1), w2(_w2), w3(_w3),
		r(color.r), g(color.g), b(color.b), a(color.a)
	{}
};

static void tess_callback_begin(GLenum type)
{
	if (type != GL_TRIANGLES)
	{
		std::cerr << "Error: tess_callback_begin received value: " << type << "\n";
	}
}

static void tess_callback_vertex_data(void* vertex_data, void* user_data)
{
	tess_vtx* v = (tess_vtx*)vertex_data;
	mesh_vertex mv;
	mv.x = (f32)v->x;
	mv.y = (f32)v->y;
	mv.z = (f32)v->z;
	mv.u0 = v->u0;
	mv.v0 = v->v0;
	mv.uo0 = v->uo0;
	mv.vo0 = v->vo0;
	mv.u1 = v->u1;
	mv.v1 = v->v1;
	mv.uo1 = v->uo1;
	mv.vo1 = v->vo1;
	mv.u2 = v->u2;
	mv.v2 = v->v2;
	mv.uo2 = v->uo2;
	mv.vo2 = v->vo2;
	mv.u3 = v->u3;
	mv.v3 = v->v3;
	mv.uo3 = v->uo3;
	mv.vo3 = v->vo3;
	mv.w0 = (f32)v->w0;
	mv.w1 = (f32)v->w1;
	mv.w2 = (f32)v->w2;
	mv.w3 = (f32)v->w3;
	mv.r = v->r;
	mv.g = v->g;
	mv.b = v->b;
	mv.a = v->a;
	v->target->push_back(mv);
}

static void tess_callback_end(void)
{
	//
}

static void tess_callback_edge_flag(GLboolean flag)
{
}

static void tess_callback_error(GLenum err)
{
	printf("TESS ERROR: '%s'\n", gluErrorString(err));
}
