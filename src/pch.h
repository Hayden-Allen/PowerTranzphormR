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

template<typename T>
static int sign(const T t)
{
	return t == 0 ? 0 : (t > 0 ? 1 : -1);
}