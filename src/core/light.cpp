#include "pch.h"
#include "light.h"


light::light() : xportable(std::string("Light"))
{
	set_type(light_type::POINT);
}
light::light(const mgl::light& ml, const std::string& _name) :
	xportable(_name),
	mgl_light(ml)
{
	set_type(light_type::POINT);
}
light::light(const nlohmann::json& obj) :
	xportable(obj)
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
	nlohmann::json obj = xportable::save();
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
light_type light::get_type() const
{
	return m_type;
}
void light::set_type(const light_type t)
{
	m_type = t;
	mgl_light.mat.t[3] = (f32)t;
}