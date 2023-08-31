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
		for (auto& pair : m_tex_name_to_filename)
		{
			g::texlib->unload(pair.first);
		}
	}

	void remove_texture(const std::string& tex_name)
	{
		const auto& old_it = m_tex_name_to_filename.find(tex_name);
		if (old_it != m_tex_name_to_filename.end())
		{
			g::texlib->unload(m_tex_name_to_filename.at(tex_name));
			m_tex_name_to_filename.erase(tex_name);
		}
	}
	void set_texture(const std::string& tex_name, const std::string& fp)
	{
		const auto& old_it = m_tex_name_to_filename.find(tex_name);
		if (old_it != m_tex_name_to_filename.end())
		{
			g::texlib->unload(m_tex_name_to_filename.at(tex_name));
		}
		g::texlib->load(fp);
		m_tex_name_to_filename[tex_name] = fp;
	}
	const mgl::texture2d_rgb_u8* get_texture(const std::string& tex_name) const
	{
		assert(m_tex_name_to_filename.contains(tex_name));
		return g::texlib->get(m_tex_name_to_filename.at(tex_name));
	}

	void for_each_texture(const std::function<void(const std::string&, const mgl::texture2d_rgb_u8*)>& l) const
	{
		for (const auto& pair : m_tex_name_to_filename)
		{
			l(pair.first, get_texture(pair.first));
		}
	}

private:
	std::unordered_map<std::string, std::string> m_tex_name_to_filename;
};