#pragma once
#include "texture_library.h"

namespace g
{
	extern texture_library* texlib;
	extern mgl::shaders* shaders;

	extern void init();
	extern void destroy();
} // namespace g