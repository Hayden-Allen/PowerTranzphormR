#include "pch.h"
#include "light.h"


light::light() :
	m_id(std::string("lit") + std::to_string(s_next_id++))
{
	set_type(light_type::POINT);
}
light::light(const mgl::light& ml, const std::string& _name) :
	mgl_light(ml),
	name(_name)
{
	set_type(light_type::POINT);
}
light::light(const nlohmann::json& obj) :
	name(obj["n"]),
	m_id(obj["id"])
{
	set_type(obj["ty"]);

	mgl_light.mat = u::json2tmat<space::OBJECT, space::WORLD>(obj["t"]);
	for (s32 i = 0; i < 4; i++)
	{
		mgl_light.ca[i] = obj["ca"][i];
		mgl_light.cd[i] = obj["cd"][i];
		mgl_light.cs[i] = obj["cs"][i];
	}
	mgl_light.sp = obj["sp"];
	mgl_light.rmax = obj["rm"];
}



u32 light::get_next_id()
{
	return s_next_id;
}
void light::set_next_id(const u32 id)
{
	s_next_id = id;
}
void light::reset_next_id()
{
	s_next_id = s_first_id;
}



std::vector<std::pair<std::string, generated_mesh_param>> light::get_params() const
{
	return {
		{ "Ambient Color", { generated_mesh_param_type::COLOR_4, (void*)mgl_light.ca, 0, 0, 0 } },
		{ "Diffuse Color", { generated_mesh_param_type::COLOR_4, (void*)mgl_light.cd, 0, 0, 0 } },
		{ "Specular Color", { generated_mesh_param_type::COLOR_4, (void*)mgl_light.cs, 0, 0, 0 } },
		{ "Specular Power", { generated_mesh_param_type::FLOAT_1_LOG, (void*)&mgl_light.sp, MIN_PARAM_VALUE, MAX_PARAM_VALUE, 1.f } },
		{ "Max Distance", { generated_mesh_param_type::FLOAT_1_LOG, (void*)&mgl_light.rmax, MIN_PARAM_VALUE, MAX_PARAM_VALUE, 1.f } },
	};
}
nlohmann::json light::save() const
{
	nlohmann::json obj;
	obj["id"] = m_id;
	obj["n"] = name;
	obj["ty"] = m_type;
	obj["t"] = mgl_light.mat.e;
	obj["ca"] = mgl_light.ca;
	obj["cd"] = mgl_light.cd;
	obj["cs"] = mgl_light.cs;
	obj["sp"] = mgl_light.sp;
	obj["rm"] = mgl_light.rmax;
	return obj;
}
tmat<space::OBJECT, space::WORLD>& light::get_mat()
{
	return mgl_light.mat;
}
const tmat<space::OBJECT, space::WORLD>& light::get_mat() const
{
	return mgl_light.mat;
}
void light::set_mat(const tmat<space::OBJECT, space::WORLD>& m)
{
	mgl_light.mat = m;
}
const std::string& light::get_name() const
{
	return name;
}
light_type light::get_type() const
{
	return m_type;
}
void light::set_name(const std::string& n)
{
	name = n;
}
void light::set_type(const light_type t)
{
	m_type = t;
	mgl_light.mat.t[3] = (f32)t;
}
const std::string& light::get_id() const
{
	return m_id;
}