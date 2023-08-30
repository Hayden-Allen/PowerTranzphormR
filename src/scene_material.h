#pragma once
#include "pch.h"

struct scene_material
{
	std::string name;
	mgl::shaders* shaders;

	scene_material() :
		name(""),
		shaders(nullptr)
	{}
	scene_material(const std::string& n, mgl::shaders* const s) :
		name(n),
		shaders(s)
	{}
	scene_material(const scene_material& o) noexcept :
		name(o.name),
		shaders(o.shaders)
	{}
	~scene_material()
	{
		for (auto& pair : texs)
		{
			g::texlib->unload(&pair.second);
		}
	}

	void remove_texture(const std::string& name)
	{
		const auto& old_it = texs.find(name);
		if (old_it != texs.end())
		{
			g::texlib->unload(&old_it->second);
			texs.erase(name);
		}
	}
	void set_texture(const std::string& name, const std::string& fp)
	{
		const auto& old_it = texs.find(name);
		if (old_it != texs.end())
		{
			g::texlib->unload(&old_it->second);
		}
		g::texlib->load(fp);
		texs[name] = g::texlib->get(fp);
	}
	const mgl::texture2d_rgb_u8* get_texture(const std::string& name) const
	{
		return texs.at(name);
	}

	void for_each_texture(const std::function<void(const std::string&, const mgl::texture2d_rgb_u8*)>& l) const
	{
		for (const auto& pair : texs)
		{
			l(pair.first, pair.second);
		}
	}

private:
	std::unordered_map<std::string, mgl::texture2d_rgb_u8*> texs;
};