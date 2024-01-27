#include "pch.h"
#include "scene_material.h"

scene_material::scene_material() :
	xportable(std::string("Material")),
	opaque_shaders(nullptr),
	alpha_shaders(nullptr)
{}
scene_material::scene_material(const std::string& n, mgl::shaders* const os, mgl::shaders* const as) :
	xportable(n),
	opaque_shaders(os),
	alpha_shaders(as)
{}
scene_material::scene_material(const scene_material& o) noexcept :
	xportable(o.get_name()),
	opaque_shaders(o.opaque_shaders),
	alpha_shaders(o.alpha_shaders)
{}
scene_material::scene_material(const std::string& phorm_fp, const nlohmann::json& obj, mgl::shaders* const os, mgl::shaders* const as) :
	xportable(obj),
	opaque_shaders(os),
	alpha_shaders(as)
{
	const nlohmann::json::array_t& texs = obj["texs"];
	for (const nlohmann::json::array_t& tex : texs)
	{
		const std::string& tname = tex[0];
		const std::string& fname = tex[1];
		if (fname == g::null_tex_fp)
		{
			m_tex_name_to_filename.insert({ tname, g::null_tex_fp });
		}
		else
		{
			const std::string& abs_fname = u::relative_to_absolute(fname, phorm_fp);
			m_tex_name_to_filename.insert({ tname, abs_fname });
			g::texlib->load(abs_fname);
		}
	}
	m_use_alpha = obj["use_alpha"];
	m_use_lighting = obj["use_lighting"];
	m_should_cull = obj["should_cull"];
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
const texture* scene_material::get_texture(const std::string& tex_name) const
{
	assert(m_tex_name_to_filename.contains(tex_name));
	return g::texlib->get(m_tex_name_to_filename.at(tex_name));
}
void scene_material::for_each_texture(const std::function<void(const std::string&, const texture*)>& l) const
{
	for (const auto& pair : m_tex_name_to_filename)
	{
		l(pair.first, get_texture(pair.first));
	}
}
nlohmann::json scene_material::save(std::ofstream& out, const std::string& out_fp) const
{
	nlohmann::json obj = xportable::save();

	std::vector<nlohmann::json::array_t> texs;
	for (const auto& pair : m_tex_name_to_filename)
	{
		// TODO hack
		if (pair.second != g::null_tex_fp)
			texs.push_back({ pair.first, u::absolute_to_relative(pair.second, out_fp) });
		else
			texs.push_back({ pair.first, pair.second });
	}
	obj["texs"] = texs;
	obj["use_alpha"] = m_use_alpha;
	obj["use_lighting"] = m_use_lighting;
	obj["should_cull"] = m_should_cull;
	return obj;
}
autotexture_params& scene_material::get_autotexture_params(const std::string& tex_name)
{
	return m_autotexture_params[tex_name];
}
bool scene_material::get_use_alpha() const
{
	return m_use_alpha;
}
void scene_material::set_use_alpha(bool alpha)
{
	m_use_alpha = alpha;
}
bool scene_material::get_use_lighting() const
{
	return m_use_lighting;
}
void scene_material::set_use_lighting(bool light)
{
	m_use_lighting = light;
}
bool scene_material::get_should_cull() const
{
	return m_should_cull;
}
void scene_material::set_should_cull(bool cull)
{
	m_should_cull = cull;
}
scene_material* scene_material::clone() const
{
	scene_material* cloned = new scene_material;
	cloned->copy_properties_from(*this);
	cloned->set_use_alpha(get_use_alpha());
	cloned->set_use_lighting(get_use_lighting());
	cloned->set_should_cull(get_should_cull());
	for (const auto& it : m_tex_name_to_filename)
	{
		cloned->set_texture(it.first, it.second);
	}
	cloned->opaque_shaders = opaque_shaders;
	cloned->alpha_shaders = alpha_shaders;
	return cloned;
}
void scene_material::xport(mgl::output_file& out, const std::unordered_map<std::string, u64>& texname2idx) const
{
	out.ubyte(m_use_alpha);
	out.ubyte(m_use_lighting);
	out.ubyte(m_should_cull);
	assert(m_tex_name_to_filename.size() == 4);
	std::map<std::string, std::string> sorted;
	for (const auto& pair : m_tex_name_to_filename)
	{
		sorted.insert(pair);
	}
	for (const auto& pair : sorted)
	{
		assert(texname2idx.contains(pair.second));
		out.ulong(texname2idx.at(pair.second));
	}
}
