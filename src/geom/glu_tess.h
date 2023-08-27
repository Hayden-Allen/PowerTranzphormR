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
	f32 u = 0.f, v = 0.f;
	f32 nx = 0.f, ny = 0.f, nz = 0.f;

	bool operator==(const mesh_vertex& o) const
	{
		return x == o.x && y == o.y && z == o.z && u == o.u && v == o.v && nx == o.nx && ny == o.ny && nz == o.nz;
	}
	std::string to_string() const
	{
		char buf[128] = { 0 };
		sprintf_s(buf, "%.6f %.6f %.6f", x, y, z);
		return buf;
	}
	void print() const
	{
		printf("(%f %f %f) (%f %f) (%f %f %f)\n", x, y, z, u, v, nx, ny, nz);
	}
};
struct tess_vtx
{
	f64 x = 0.0, y = 0.0, z = 0.0;
	f32 u = 0.0f, v = 0.0f;
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
	mv.u = v->u;
	mv.v = v->v;
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
