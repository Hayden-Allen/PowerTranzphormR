#pragma once
#include "pch.h"

class texture_library
{
public:
	texture_library() {}
	MGL_DCM(texture_library);
	virtual ~texture_library()
	{
		for (const auto& pair : m_lib)
			delete pair.second;
		delete m_deftex;
	}
public:
	void init_deftex()
	{
		u8 deftex_pixels[12] = { 255, 0, 255, 0, 0, 0, 0, 0, 0, 255, 0, 255 };
		m_deftex = new mgl::texture2d_rgb_u8(GL_RGB, 2, 2, deftex_pixels, { .min_filter = GL_NEAREST, .mag_filter = GL_NEAREST });
	}
	const mgl::texture2d_rgb_u8* get(const std::string& fp) const
	{
		if (fp == g::null_tex_fp)
		{
			return m_deftex;
		}
		const auto& it = m_lib.find(fp);
		assert(it != m_lib.end());
		return it->second;
	}
	mgl::texture2d_rgb_u8* get(const std::string& fp)
	{
		if (fp == g::null_tex_fp)
		{
			return m_deftex;
		}
		const auto& it = m_lib.find(fp);
		assert(it != m_lib.end());
		return it->second;
	}
	void load(const std::string& fp)
	{
		if (fp == g::null_tex_fp)
		{
			return; // default texture
		}
		// if we've already loaded a texture at this fp, delete it because we'll replace it with the newly loaded one
		const auto& it = m_lib.find(fp);
		if (it != m_lib.end())
		{
			m_counts[fp]++;
			delete it->second;
			m_lib.erase(it);
		}
		else
		{
			m_counts.insert({ fp, 1 });
		}
		// ALWAYS reload because file may have been resaved
		mgl::texture2d_rgb_u8* const new_tex = load_file(fp);
		m_lib.insert({ fp, new_tex });
	}
	void unload(const std::string& fp)
	{
		if (fp == g::null_tex_fp)
		{
			return; // default texture
		}
		assert(m_counts.contains(fp));
		m_counts[fp]--;
		if (m_counts[fp] == 0)
		{
			m_counts.erase(fp);
			delete m_lib[fp];
			m_lib.erase(fp);
		}
	}
private:
	static mgl::texture2d_rgb_u8* load_file(const std::string& fp)
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
		stbi_uc* const tex_data = stbi_load(fp.c_str(), &w, &h, &c, 3);
		mgl::texture2d_rgb_u8* tex = new mgl::texture2d_rgb_u8(GL_RGB, w, h, tex_data);
		stbi_image_free(tex_data);
		return tex;
	}
private:
	mgl::texture2d_rgb_u8* m_deftex = nullptr;
	std::unordered_map<std::string, mgl::texture2d_rgb_u8*> m_lib;
	std::unordered_map<std::string, u32> m_counts;
};
