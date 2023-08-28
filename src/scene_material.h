#pragma once
#include "pch.h"

struct scene_material
{
	std::string name;
	std::unordered_map<std::string, mgl::texture2d_rgb_u8*> texs;
	mgl::shaders* shaders;

	scene_material() :
		name(""),
		shaders(nullptr)
	{}
	scene_material(const std::string& n, const std::unordered_map<std::string, mgl::texture2d_rgb_u8*>& t, mgl::shaders* const s) :
		name(n),
		texs(t),
		shaders(s)
	{}
	scene_material(const scene_material& o) noexcept :
		name(o.name),
		texs(o.texs),
		shaders(o.shaders)
	{}
	~scene_material()
	{
		delete shaders;
		for (const auto& pair : texs)
			delete pair.second;
	}
};