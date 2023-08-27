#pragma once
#include "hats/hats.h"
#include "nlohmann/json.hpp"

#define MAX_VALUE(x) std::numeric_limits<decltype(x)>::max()

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
