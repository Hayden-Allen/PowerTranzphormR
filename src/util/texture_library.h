#pragma once
#include "pch.h"

typedef mgl::retained_texture_array<mgl::retained_texture2d_rgba_u8> texture;

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
	bool has(const std::string& fp) const
	{
		return (fp == g::null_tex_fp) || m_lib.contains(fp);
	}
	void init_deftex()
	{
		u8 deftex_pixels[16] = { 255, 0, 255, 255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 255, 255 };
		m_deftex = new texture(GL_RGBA, 2, 2, deftex_pixels, { .min_filter = GL_NEAREST, .mag_filter = GL_NEAREST });
	}
	const texture* get(const std::string& fp) const
	{
		if (fp == g::null_tex_fp)
		{
			return m_deftex;
		}
		const auto& it = m_lib.find(fp);
		assert(it != m_lib.end());
		return it->second;
	}
	texture* get(const std::string& fp)
	{
		if (fp == g::null_tex_fp)
		{
			return m_deftex;
		}
		const auto& it = m_lib.find(fp);
		assert(it != m_lib.end());
		return it->second;
	}
	const std::unordered_map<std::string, texture*>& get_all() const
	{
		return m_lib;
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
		texture* const new_tex = load_file(fp);
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

			if (m_autotex_cleanup_enabled)
			{
				bool should_delete = false;
				std::filesystem::path fspath(fp);
				while (!fspath.filename().empty())
				{
					if (std::filesystem::is_directory(fspath))
					{
						if (fspath.filename() == "_PowerTextuRe_")
						{
							should_delete = true;
						}
					}
					fspath = fspath.parent_path();
				}
				if (should_delete)
				{
					try
					{
						std::filesystem::remove(fp);
					}
					catch (const std::exception&)
					{
						std::cerr << "Unable to delete file: " << fp << "\n";
					}
				}
			}
		}
	}
	void set_autotexture_cleanup_enabled(const bool enabled)
	{
		m_autotex_cleanup_enabled = enabled;
	}
	const texture* get_deftex() const
	{
		return m_deftex;
	}
	void update(const f32 time)
	{
		for (auto& pair : m_lib)
			pair.second->update((u64)time);
	}
private:
	static texture* load_file(const std::string& fp)
	{
		return u::load_texture_array(fp);
	}
private:
	texture* m_deftex = nullptr;
	std::unordered_map<std::string, texture*> m_lib;
	std::unordered_map<std::string, u32> m_counts;
	bool m_autotex_cleanup_enabled = true;
};
