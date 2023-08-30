#pragma once
#include "texture_library.h"

namespace g
{
	extern texture_library* texlib;
	extern mgl::shaders* shaders;
	extern mgl::texture2d_rgb_u8* deftex;

	extern void init();
	extern void destroy();
} // namespace g