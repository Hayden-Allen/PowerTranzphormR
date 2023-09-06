#pragma once
#include "hats/hats.h"
#include "nlohmann/json.hpp"

#define MAX_VALUE(x) std::numeric_limits<decltype(x)>::max()
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

	static std::string absolute_to_relative(const std::string& fp)
	{
		const auto& cwd = std::filesystem::current_path();
		return std::filesystem::relative(fp, cwd).string();
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
} // namespace u
