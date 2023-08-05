#pragma once
#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <chrono>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
// absolutely hilarious
#undef APIENTRY

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

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "hats/hats.h"
