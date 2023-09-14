#pragma once
#include "pch.h"
#include "globals.h"
#include "texture_library.h"

namespace g
{
	texture_library* texlib = nullptr;
	mgl::shaders* shaders = nullptr;
	f32 font_size = 0.0f;

	void init()
	{
		texlib = new texture_library();
		texlib->init_deftex();
		// shaders = new mgl::shaders("src/glsl/csg_vert.vert", "src/glsl/csg_vert.frag");
		shaders = new mgl::shaders("src/glsl/csg_frag.vert", "src/glsl/csg_frag.frag");
	}
	void clear()
	{
		destroy();
		init();
	}
	void destroy()
	{
		delete shaders;
		delete texlib;
	}
} // namespace g