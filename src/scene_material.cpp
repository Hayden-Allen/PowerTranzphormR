#include "pch.h"
#include "scene_material.h"

scene_material::scene_material() :
	name(""),
	shaders(nullptr)
{}
scene_material::scene_material(const std::string& n, mgl::shaders* const s) :
	name(n),
	shaders(s)
{}
scene_material::scene_material(const scene_material& o) noexcept :
	name(o.name),
	shaders(o.shaders)
{}
scene_material::scene_material(const nlohmann::json& obj, mgl::shaders* const s) :
	name(obj["name"]),
	shaders(s)
{
	const nlohmann::json::array_t& texs = obj["texs"];
	for (const nlohmann::json::array_t& tex : texs)
	{
		const std::string& tname = tex[0];
		const std::string& fname = tex[1];
		m_tex_name_to_filename.insert({ tname, fname });
		// TODO hack
		if (fname != "<NULL>")
			g::texlib->load(fname);
	}
}
scene_material::~scene_material()
{
	for (auto& pair : m_tex_name_to_filename)
	{
		g::texlib->unload(pair.second);
	}
}



void scene_material::remove_texture(const std::string& tex_name)
{
	const auto& old_it = m_tex_name_to_filename.find(tex_name);
	if (old_it != m_tex_name_to_filename.end())
	{
		g::texlib->unload(m_tex_name_to_filename.at(tex_name));
		m_tex_name_to_filename.erase(tex_name);
	}
}
void scene_material::set_texture(const std::string& tex_name, const std::string& fp)
{
	const auto& old_it = m_tex_name_to_filename.find(tex_name);
	if (old_it != m_tex_name_to_filename.end())
	{
		g::texlib->unload(m_tex_name_to_filename.at(tex_name));
	}
	g::texlib->load(fp);
	m_tex_name_to_filename[tex_name] = fp;
}
const mgl::texture2d_rgb_u8* scene_material::get_texture(const std::string& tex_name) const
{
	assert(m_tex_name_to_filename.contains(tex_name));
	return g::texlib->get(m_tex_name_to_filename.at(tex_name));
}
void scene_material::for_each_texture(const std::function<void(const std::string&, const mgl::texture2d_rgb_u8*)>& l) const
{
	for (const auto& pair : m_tex_name_to_filename)
	{
		l(pair.first, get_texture(pair.first));
	}
}
nlohmann::json scene_material::save(std::ofstream& out) const
{
	nlohmann::json obj;
	obj["name"] = name;
	std::vector<nlohmann::json::array_t> texs;
	for (const auto& pair : m_tex_name_to_filename)
	{
		texs.push_back({ pair.first, pair.second });
	}
	obj["texs"] = texs;
	return obj;
}
