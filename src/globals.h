#pragma once
#include "texture_library.h"

namespace g
{
	static texture_library* texlib;

	static void init()
	{
		texlib = new texture_library();
	}
	static void destroy()
	{
		delete texlib;
	}
} // namespace g