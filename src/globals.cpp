#pragma once
#include "pch.h"
#include "globals.h"
#include "texture_library.h"

namespace g
{
	texture_library* texlib = nullptr;
	mgl::shaders* shaders = nullptr;
	mgl::texture2d_rgb_u8* deftex = nullptr;

	void init()
	{
		texlib = new texture_library();
		shaders = new mgl::shaders("src/glsl/csg.vert", "src/glsl/csg.frag");
		u8 deftex_pixels[12] = { 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255 };
		deftex = new mgl::texture2d_rgb_u8(GL_RGB, 2, 2, deftex_pixels, { .min_filter = GL_NEAREST, .mag_filter = GL_NEAREST });
	}
	void destroy()
	{
		delete deftex;
		delete shaders;
		delete texlib;
	}
} // namespace g