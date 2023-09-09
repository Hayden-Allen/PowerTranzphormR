#include "pch.h"
#include "light.h"


light::light(const nlohmann::json& obj) :
	name(obj["n"])
{
	mgl_light.mat = u::json2tmat<space::OBJECT, space::WORLD>(obj["t"]);
	for (s32 i = 0; i < 4; i++)
	{
		mgl_light.ca[i] = obj["ca"][i];
		mgl_light.cd[i] = obj["cd"][i];
		mgl_light.cs[i] = obj["cs"][i];
	}
	mgl_light.sp = obj["sp"];
}



std::vector<std::pair<std::string, generated_mesh_param>> light::get_params() const
{
	return {
		{ "Ambient Color", { generated_mesh_param_type::COLOR_4, (void*)mgl_light.ca, 0, 0, 0 } },
		{ "Diffuse Color", { generated_mesh_param_type::COLOR_4, (void*)mgl_light.cd, 0, 0, 0 } },
		{ "Specular Color", { generated_mesh_param_type::COLOR_4, (void*)mgl_light.cs, 0, 0, 0 } },
		{ "Specular Power", { generated_mesh_param_type::FLOAT_1, (void*)&mgl_light.sp, 0, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
	};
}
nlohmann::json light::save() const
{
	nlohmann::json obj;
	obj["n"] = name;
	obj["t"] = mgl_light.mat.e;
	obj["ca"] = mgl_light.ca;
	obj["cd"] = mgl_light.cd;
	obj["cs"] = mgl_light.cs;
	obj["sp"] = mgl_light.sp;
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
