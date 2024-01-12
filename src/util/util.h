#pragma once
#include "hats/hats.h"
#include "nlohmann/json.hpp"

#define MAX_VALUE(x)	  std::numeric_limits<decltype(x)>::max()
#define MAX_VALUE_TYPE(x) std::numeric_limits<x>::max()

namespace u
{
	template<typename T>
	static int sign(const T t)
	{
		return t == 0 ? 0 : (t > 0 ? 1 : -1);
	}

	template<hats::space FROM, hats::space TO>
	static hats::tmat<FROM, TO> json2tmat(const nlohmann::json& obj)
	{
		nlohmann::json::array_t arr = obj;
		f32 e[16] = { 0.f };
		assert(arr.size() == 16);
		s32 i = 0;
		for (const auto& v : arr)
			e[i++] = v.get<f32>();
		return tmat<FROM, TO>(e);
	}

	static std::string operation_to_string(carve::csg::CSG::OP op)
	{
		switch (op)
		{
		case carve::csg::CSG::OP::ALL: return "All";
		case carve::csg::CSG::OP::A_MINUS_B: return "Subtract (A - B)";
		case carve::csg::CSG::OP::INTERSECTION: return "Intersect";
		case carve::csg::CSG::OP::UNION: return "Union";
		}
		assert(false);
		return "<ERROR>";
	}

	static std::string absolute_to_relative(const std::string& fp, const std::string& base)
	{
		return std::filesystem::relative(fp, base).string();
	}

	static std::string relative_to_absolute(const std::string& fp, const std::string& base)
	{
		return std::filesystem::absolute(std::filesystem::path(base) / std::filesystem::path(fp)).string();
	}

	static nlohmann::json next_line_json(std::ifstream& in)
	{
		std::string line;
		std::getline(in, line);
		return nlohmann::json::parse(line);
	}

	static bool rng_seeded = false;
	// https://stackoverflow.com/questions/9878965/rand-between-0-and-1
	static float rand(const f32 min = 0.f, const f32 max = 1.f)
	{
		static std::mt19937_64 rng;
		if (!rng_seeded)
		{
			uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
			std::seed_seq ss{ uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed >> 32) };
			rng.seed(ss);
			rng_seeded = true;
		}
		std::uniform_real_distribution<f32> unif(min, max);

		return unif(rng);
	}

	static void reset_imgui_io(GLFWwindow* const window)
	{
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		GLFWwindow* fake_window = glfwCreateWindow(1, 1, "PowerTranzphormR - Reset Focus", nullptr, nullptr);
		glfwFocusWindow(fake_window);
		glfwDestroyWindow(fake_window);
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		glfwFocusWindow(window);
	}

	static std::string open_save_dialog_base(GLFWwindow* const window, const std::string& label, const std::string& exts, const bool save)
	{
		nfdchar_t* nfd_path = nullptr;
		nfdfilteritem_t nfd_filters[1] = { { label.c_str(), exts.c_str() } };
		nfdresult_t nfd_res;
		if (save)
			nfd_res = NFD_SaveDialog(&nfd_path, nfd_filters, 1, nullptr, nullptr);
		else
			nfd_res = NFD_OpenDialog(&nfd_path, nfd_filters, 1, nullptr);
		std::string ret = "";
		if (nfd_res == NFD_OKAY)
		{
			ret = nfd_path;
			NFD_FreePath(nfd_path);
		}
		else if (nfd_res == NFD_ERROR)
		{
			std::cerr << "NFD Error: " << NFD_GetError() << "\n";
		}
		reset_imgui_io(window);
		return ret;
	}

	static std::string open_dialog(GLFWwindow* const window, const std::string& label, const std::string& exts)
	{
		return open_save_dialog_base(window, label, exts, false);
	}

	static std::string save_dialog(GLFWwindow* const window, const std::string& label, const std::string& exts)
	{
		return open_save_dialog_base(window, label, exts, true);
	}
	static std::string open_folder(GLFWwindow* const window)
	{
		nfdchar_t* nfd_path = nullptr;
		nfdresult_t nfd_res = NFD_PickFolder(&nfd_path, nullptr);
		std::string ret = "";
		if (nfd_res == NFD_OKAY)
		{
			ret = nfd_path;
			NFD_FreePath(nfd_path);
		}
		else if (nfd_res == NFD_ERROR)
		{
			std::cerr << "NFD Error: " << NFD_GetError() << "\n";
		}
		reset_imgui_io(window);
		return ret;
	}

	static void info_message_box(GLFWwindow* const window, LPCWSTR const msg, LPCWSTR const title)
	{
		MessageBox(glfwGetWin32Window(window), msg, title, MB_OK | MB_ICONINFORMATION);
		reset_imgui_io(window);
	}

	static void error_message_box(GLFWwindow* const window, LPCWSTR const msg, LPCWSTR const title)
	{
		MessageBox(glfwGetWin32Window(window), msg, title, MB_OK | MB_ICONERROR);
		reset_imgui_io(window);
	}

	static s32 confirm_message_box(GLFWwindow* const window, LPCWSTR const msg, LPCWSTR const title)
	{
		const s32 res = MessageBox(glfwGetWin32Window(window), msg, title, MB_YESNOCANCEL | MB_ICONQUESTION);
		reset_imgui_io(window);
		return res;
	}

	template<space SPACE>
	static carve::geom3d::Vector hats2carve(const point<SPACE>& p)
	{
		return carve::geom::VECTOR(p.x, p.y, p.z);
	}

	static mgl::texture2d_rgba_u8* load_texture2d_rgba_u8(const std::string& fp)
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
		stbi_uc* const tex_data = stbi_load(fp.c_str(), &w, &h, &c, 4);
		assert(c >= 3);
		mgl::texture2d_rgba_u8* tex = new mgl::texture2d_rgba_u8(GL_RGBA, w, h, tex_data);
		stbi_image_free(tex_data);
		return tex;
	}

	static mgl::retained_texture2d_rgba_u8* load_retained_texture2d_rgba_u8(const std::string& fp)
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
		stbi_uc* const tex_data = stbi_load(fp.c_str(), &w, &h, &c, 4);
		assert(c >= 3);
		mgl::retained_texture2d_rgba_u8* tex = new mgl::retained_texture2d_rgba_u8(GL_RGBA, w, h, tex_data);
		stbi_image_free(tex_data);
		return tex;
	}

	static mgl::skybox_rgb_u8* load_skybox_rgb_u8(const std::string& vert_fp, const std::string& frag_fp, const std::array<std::string, 6>& fps)
	{
		int aw = -1, ah = -1;
		const u8* tex[6] = { nullptr };
		for (u64 i = 0; i < fps.size(); i++)
		{
			std::ifstream test(fps[i]);
			if (!test.is_open())
			{
				LOG_FATAL("Invalid texture filepath {}", fps[i].c_str());
				MGL_ASSERT(false);
				return nullptr;
			}
			test.close();

			stbi_set_flip_vertically_on_load(true);
			int w = -1, h = -1, c = -1;
			tex[i] = stbi_load(fps[i].c_str(), &w, &h, &c, 3);
			if (aw == -1)
			{
				aw = w;
				ah = h;
			}
			else if (!(w == aw && h == ah))
			{
				LOG_ERROR("All skybox texture dimensions must match");
				for (s32 j = 0; j < 6; j++)
					delete tex[i];
				return nullptr;
			}
		}

		mgl::skybox_rgb_u8* ret = new mgl::skybox_rgb_u8(vert_fp, frag_fp, std::move(mgl::cubemap_rgb_u8(GL_RGB, aw, ah, tex)));
		for (s32 i = 0; i < 6; i++)
			delete tex[i];
		return ret;
	}

	static f32 hue2rgb(f32 v1, f32 v2, f32 vH)
	{
		if (vH < 0)
			vH += 1;

		if (vH > 1)
			vH -= 1;

		if ((6 * vH) < 1)
			return (v1 + (v2 - v1) * 6 * vH);

		if ((2 * vH) < 1)
			return v2;

		if ((3 * vH) < 2)
			return (v1 + (v2 - v1) * ((2.0f / 3) - vH) * 6);

		return v1;
	}

	static ImVec4 hsl2rgb(const f32 h, const f32 s, const f32 l)
	{
		f32 r, g, b;

		if (s == 0)
		{
			r = g = b = l;
		}
		else
		{
			f32 v1, v2;
			f32 hue = h / 360;

			v2 = (l < 0.5f) ? (l * (1 + s)) : ((l + s) - (l * s));
			v1 = 2 * l - v2;

			r = hue2rgb(v1, v2, hue + (1.0f / 3));
			g = hue2rgb(v1, v2, hue);
			b = hue2rgb(v1, v2, hue - (1.0f / 3));
		}

		return ImVec4(r, g, b, 1.f);
	}
} // namespace u
