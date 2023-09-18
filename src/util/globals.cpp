#pragma once
#include "pch.h"
#include "globals.h"
#include "texture_library.h"

namespace g
{
	texture_library* texlib = nullptr;
	mgl::shaders* opaque_shaders = nullptr, *alpha_shaders = nullptr;
	f32 font_size = 0.0f;

	void init()
	{
		texlib = new texture_library();
		texlib->init_deftex();
		opaque_shaders = new mgl::shaders("src/glsl/csg_frag.vert", "src/glsl/csg_frag.frag");
		alpha_shaders = new mgl::shaders("src/glsl/csg_frag.vert", "src/glsl/csg_frag_alpha.frag");
	}
	void clear()
	{
		destroy();
		init();
	}
	void destroy()
	{
		delete opaque_shaders;
		delete alpha_shaders;
		delete texlib;
	}
} // namespace g