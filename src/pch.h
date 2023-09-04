#pragma once
#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <functional>
#include <filesystem>
#include <random>

#include "carve/interpolator.hpp"
#include "carve/csg.hpp"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
// absolutely hilarious
#undef APIENTRY
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "ImGuizmo.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#include "nfd.h"
// absolutely hilarious
#undef min
#undef max
#undef near
#undef far
#include <gl/GLU.h>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "util/log.h"

#include "nlohmann/json.hpp"

#include "stb_image/stb_image.h"

#include "hats/hats.h"
#include "mingl/mingl.h"
#include "util/util.h"
#include "util/globals.h"
#include "util/texture_library.h"
