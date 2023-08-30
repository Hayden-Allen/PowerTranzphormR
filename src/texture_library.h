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
	}
public:
	const mgl::texture2d_rgb_u8* get(const std::string& fp) const
	{
		const auto& it = m_lib.find(fp);
		assert(it != m_lib.end());
		return it->second;
	}
	const mgl::texture2d_rgb_u8* get_or_load(const std::string& fp)
	{
		const auto& it = m_lib.find(fp);
		if (it != m_lib.end())
			return it->second;
		mgl::texture2d_rgb_u8* const new_tex = load(fp);
		m_lib.insert({ fp, new_tex });
		return new_tex;
	}
private:
	static mgl::texture2d_rgb_u8* load(const std::string& fp)
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
	std::unordered_map<std::string, mgl::texture2d_rgb_u8*> m_lib;
};
