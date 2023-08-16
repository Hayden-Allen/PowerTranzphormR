#pragma once
#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <functional>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
// absolutely hilarious
#undef APIENTRY
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "stb_image/stb_image.h"

#include "carve/interpolator.hpp"
#include "carve/csg.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
// absolutely hilarious
#undef min
#undef max
#undef near
#undef far

#include <gl/GLU.h>

#include "hats/hats.h"
#include "mingl/mingl.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "log.h"

#define MAX_VALUE(x) std::numeric_limits<decltype(x)>::max()

// HATODO move all this crap
template<typename T>
static int sign(const T t)
{
	return t == 0 ? 0 : (t > 0 ? 1 : -1);
}
static mgl::texture2d_rgb_u8* load_texture_rgb_u8(const std::string& fp)
{
	std::ifstream test(fp);
	if (!test.is_open())
	{
		LOG_FATAL("Invalid texture filepath {}", fp.c_str());
		MGL_ASSERT(false);
		return nullptr;
	}
	test.close();

	stbi_set_flip_vertically_on_load(true);
	int w = -1, h = -1, c = -1;
	stbi_uc* tex_data = stbi_load(fp.c_str(), &w, &h, &c, 3);
	mgl::texture2d_rgb_u8* tex = new mgl::texture2d_rgb_u8(GL_RGB, w, h, tex_data);
	stbi_image_free(tex_data);
	return tex;
}
static mgl::retained_texture2d_rgb_u8* load_retained_texture_rgb_u8(const std::string& fp)
{
	std::ifstream test(fp);
	if (!test.is_open())
	{
		LOG_FATAL("Invalid texture filepath {}", fp.c_str());
		MGL_ASSERT(false);
		return nullptr;
	}
	test.close();

	stbi_set_flip_vertically_on_load(true);
	int w = -1, h = -1, c = -1;
	stbi_uc* tex_data = stbi_load(fp.c_str(), &w, &h, &c, 3);
	mgl::retained_texture2d_rgb_u8* tex = new mgl::retained_texture2d_rgb_u8(GL_RGB, w, h, tex_data);
	stbi_image_free(tex_data);
	return tex;
}
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
		sprintf_s(buf, "%.4f %.4f %.4f", x, y, z);
		return buf;
	}
	void print() const
	{
		printf("(%f %f %f) (%f %f) (%f %f %f)\n", x, y, z, u, v, nx, ny, nz);
	}
};
