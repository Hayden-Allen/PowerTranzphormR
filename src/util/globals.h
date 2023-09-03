#pragma once

class texture_library;

namespace g
{
	extern texture_library* texlib;
	extern mgl::shaders* shaders;
	constexpr static char null_mtl_name[] = "<NULL>";
	constexpr static char null_tex_fp[] = "<NULL>";

	extern void init();
	extern void clear();
	extern void destroy();
} // namespace g