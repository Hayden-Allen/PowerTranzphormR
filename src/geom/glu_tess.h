#pragma once
#include "pch.h"
#if defined(GLU_TESS_CALLBACK_VARARGS)
typedef GLvoid(__stdcall* GLUTessCallback)(...);
#else
typedef void(__stdcall* GLUTessCallback)();
#endif

struct mesh_vertex
{
	f32 x = 0.f, y = 0.f, z = 0.f;
	f32 nx = 0.f, ny = 0.f, nz = 0.f;
	f32 u0 = 0.f, v0 = 0.f;
	f32 u1 = 0.f, v1 = 0.f;
	f32 u2 = 0.f, v2 = 0.f;
	f32 u3 = 0.f, v3 = 0.f;
	f32 w0 = 0.f, w1 = 0.f, w2 = 0.f, w3 = 0.f;
	f32 r = 0.f, g = 0.f, b = 0.f, a = 0.f;

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
	std::string to_string() const
	{
		char buf[128] = { 0 };
		sprintf_s(buf, "%.6f %.6f %.6f", x, y, z);
		return buf;
	}
};
struct tess_vtx
{
	f64 x = 0.0, y = 0.0, z = 0.0;
	f32 u0 = 0.0f, v0 = 0.0f;
	f32 u1 = 0.0f, v1 = 0.0f;
	f32 u2 = 0.0f, v2 = 0.0f;
	f32 u3 = 0.0f, v3 = 0.0f;
	f64 w0 = 0.f, w1 = 0.f, w2 = 0.f, w3 = 0.f;
	f32 r = 0.f, g = 0.f, b = 0.f, a = 0.f;
	std::vector<mesh_vertex>* target = nullptr;
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
	mv.u1 = v->u1;
	mv.v1 = v->v1;
	mv.u2 = v->u2;
	mv.v2 = v->v2;
	mv.u3 = v->u3;
	mv.v3 = v->v3;
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
	//
}
