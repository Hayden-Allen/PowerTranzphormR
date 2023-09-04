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
	f32 u0 = 1.f, v0 = 1.f, uo0 = 0.f, vo0 = 0.f;
	f32 u1 = 1.f, v1 = 1.f, uo1 = 0.f, vo1 = 0.f;
	f32 u2 = 1.f, v2 = 1.f, uo2 = 0.f, vo2 = 0.f;
	f32 u3 = 1.f, v3 = 1.f, uo3 = 0.f, vo3 = 0.f;
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
	u32 hash() const
	{
		// hash all elements except normals
		std::vector<f32> v = { x, y, z };
		for (u32 i = 0; i < 24; i++)
			v.push_back(*(&u0 + i));
		// https://cs.stackexchange.com/questions/37952/hash-function-floating-point-inputs-for-genetic-algorithm
		u32 h = 1;
		for (u64 i = 0; i < v.size(); i++)
		{
			s32 sf = *(s32*)&v[i];
			h = 31 * h + sf;
		}
		h ^= (h >> 20) ^ (h >> 12);
		return h ^ (h >> 7) ^ (h >> 4);
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
	//
}
