#include "pch.h"
#include "light.h"


light::light() :
	visibility_xportable(std::string("Light"))
{
	set_type(light_type::POINT);
}
light::light(const mgl::light& ml, const std::string& _name) :
	visibility_xportable(_name),
	mgl_light(ml)
{
	set_type(light_type::POINT);
}
light::light(const nlohmann::json& obj) :
	visibility_xportable(obj),
	m_type(obj["ty"])
{
	set_mat(u::json2tmat<space::OBJECT, space::WORLD>(obj["t"]));
	for (s32 i = 0; i < 4; i++)
	{
		mgl_light.ca[i] = obj["ca"][i];
		mgl_light.cd[i] = obj["cd"][i];
		mgl_light.cs[i] = obj["cs"][i];
	}
	mgl_light.sp = obj["sp"];
	mgl_light.rmax = obj["rm"];
	mgl_light.cos_tmin = obj["tmin"];
	mgl_light.cos_tmax = obj["tmax"];
}



std::vector<std::pair<std::string, generated_mesh_param>> light::get_params() const
{
	if (m_type == light_type::AREA)
	{
		return {
			{ "Ambient Color", { generated_mesh_param_type::COLOR_4, (void*)mgl_light.ca, 0, 0, 0 } },
			{ "Max Distance", { generated_mesh_param_type::FLOAT_1_LOG, (void*)&mgl_light.rmax, MIN_PARAM_VALUE, MAX_PARAM_VALUE, 1.f } },
		};
	}
	else if (m_type == light_type::SPOT)
	{
		return {
			{ "Ambient Color", { generated_mesh_param_type::COLOR_4, (void*)mgl_light.ca, 0, 0, 0 } },
			{ "Diffuse Color", { generated_mesh_param_type::COLOR_4, (void*)mgl_light.cd, 0, 0, 0 } },
			{ "Specular Color", { generated_mesh_param_type::COLOR_4, (void*)mgl_light.cs, 0, 0, 0 } },
			{ "Specular Power", { generated_mesh_param_type::FLOAT_1_LOG, (void*)&mgl_light.sp, MIN_PARAM_VALUE, MAX_PARAM_VALUE, 1.f } },
			{ "Max Distance", { generated_mesh_param_type::FLOAT_1_LOG, (void*)&mgl_light.rmax, MIN_PARAM_VALUE, MAX_PARAM_VALUE, 1.f } },
			{ "cos(theta_min)", { generated_mesh_param_type::FLOAT_1, (void*)&mgl_light.cos_tmin, 0.f, 1.f, DRAG_PARAM_STEP } },
			{ "cos(theta_max)", { generated_mesh_param_type::FLOAT_1, (void*)&mgl_light.cos_tmax, 0.f, 1.f, DRAG_PARAM_STEP } },
		};
	}
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
	nlohmann::json obj = visibility_xportable::save();
	obj["ty"] = m_type;
	obj["t"] = mgl_light.mat.e;
	obj["ca"] = mgl_light.ca;
	obj["cd"] = mgl_light.cd;
	obj["cs"] = mgl_light.cs;
	obj["sp"] = mgl_light.sp;
	obj["rm"] = mgl_light.rmax;
	obj["tmin"] = mgl_light.cos_tmin;
	obj["tmax"] = mgl_light.cos_tmax;
	return obj;
}
void light::xport(mgl::output_file& out) const
{
	xportable::xport(out);
	mgl_light.save(&out);
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
	mgl_light.inv_mat = m.invert_copy();
	// encode type in t.w (in both mats for consistency)
	mgl_light.mat.t[3] = mgl_light.inv_mat.t[3] = (f32)m_type;
}
light_type light::get_type() const
{
	return m_type;
}
light* light::clone() const
{
	light* cloned = new light;
	cloned->copy_properties_from(*this);
	cloned->m_type = m_type;
	cloned->mgl_light = mgl_light;
	return cloned;
}
void light::set_type(const light_type t)
{
	m_type = t;
	mgl_light.mat.t[3] = (f32)t;
}
