#pragma once
#include "pch.h"
#include "xportable.h"

struct autotexture_params
{
	bool enabled = false, tiling = true;
	std::string prompt, neg_prompt;
	std::string sampler = "Euler";
	s32 seed = 0, steps = 20;
	f32 cfg_scale = 7.5f;
	s32 dims[2] = { 512, 512 };
	s32 post_dims[2] = { 32, 32 };
};

struct scene_material : public xportable
{
public:
	mgl::shaders* opaque_shaders = nullptr, *alpha_shaders = nullptr;
public:
	scene_material();
	scene_material(const std::string& n, mgl::shaders* const os, mgl::shaders* const as);
	scene_material(const scene_material& o) noexcept;
	scene_material(const std::string& phorm_fp, const nlohmann::json& obj, mgl::shaders* const os, mgl::shaders* const as);
	~scene_material();
public:
	void remove_texture(const std::string& tex_name);
	void set_texture(const std::string& tex_name, const std::string& fp);
	const mgl::texture2d_rgb_u8* get_texture(const std::string& tex_name) const;
	void for_each_texture(const std::function<void(const std::string&, const mgl::texture2d_rgb_u8*)>& l) const;
	nlohmann::json save(std::ofstream& out, const std::string &out_fp) const;
	autotexture_params& get_autotexture_params(const std::string& tex_name);
	bool get_use_alpha() const;
	void set_use_alpha(bool alpha);
private:
	std::unordered_map<std::string, std::string> m_tex_name_to_filename;
	std::unordered_map<std::string, autotexture_params> m_autotexture_params;
	bool m_use_alpha = false;
};