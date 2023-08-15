#pragma once
#include "pch.h"
#if defined(GLU_TESS_CALLBACK_VARARGS)
typedef GLvoid(__stdcall* GLUTessCallback)(...);
#else
typedef void(__stdcall* GLUTessCallback)();
#endif

struct tess_vtx
{
	f64 x = 0.0, y = 0.0, z = 0.0;
	f32 u = 0.0f, v = 0.0f;
	std::vector<GLfloat>* target = nullptr;
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
	v->target->push_back((GLfloat)v->x);
	v->target->push_back((GLfloat)v->y);
	v->target->push_back((GLfloat)v->z);
	v->target->push_back((GLfloat)v->u);
	v->target->push_back((GLfloat)v->v);
}

static void tess_callback_end(void)
{
	//
}

static void tess_callback_edge_flag(GLboolean flag)
{
	//
}
