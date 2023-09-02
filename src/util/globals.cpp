#pragma once
#include "pch.h"
#include "globals.h"
#include "texture_library.h"

namespace g
{
	texture_library* texlib = nullptr;
	mgl::shaders* shaders = nullptr;

	void init()
	{
		texlib = new texture_library();
		texlib->init_deftex();
		shaders = new mgl::shaders("src/glsl/csg.vert", "src/glsl/csg.frag");
	}
	void destroy()
	{
		delete shaders;
		delete texlib;
	}
} // namespace g