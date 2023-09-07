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

	static void info_message_box(GLFWwindow* const window, LPCWSTR const msg, LPCWSTR const title)
	{
		MessageBox(glfwGetWin32Window(window), msg, title, MB_OK | MB_ICONINFORMATION);
		reset_imgui_io(window);
	}

	static s32 confirm_message_box(GLFWwindow* const window, LPCWSTR const msg, LPCWSTR const title)
	{
		const s32 res = MessageBox(glfwGetWin32Window(window), msg, title, MB_YESNOCANCEL | MB_ICONQUESTION);
		reset_imgui_io(window);
		return res;
	}

} // namespace u
