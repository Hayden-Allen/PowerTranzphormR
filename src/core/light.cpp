#include "pch.h"
#include "light.h"


light::light(const nlohmann::json& obj) :
	name(obj["n"]),
	mat(u::json2tmat<space::OBJECT, space::WORLD>(obj["t"])),
	ca(obj["ca"][0], obj["ca"][1], obj["ca"][2], obj["ca"][3]),
	cd(obj["cd"][0], obj["cd"][1], obj["cd"][2], obj["cd"][3]),
	cs(obj["cs"][0], obj["cs"][1], obj["cs"][2], obj["cs"][3]),
	ka(obj["ka"]),
	kd(obj["kd"]),
	ks(obj["ks"]),
	sp(obj["sp"])
{}



std::vector<std::pair<std::string, generated_mesh_param>> light::get_params() const
{
	return {
		{ "Ambient Color", { generated_mesh_param_type::COLOR_4, (void*)&ca, 0, 0, 0 } },
		{ "Diffuse Color", { generated_mesh_param_type::COLOR_4, (void*)&cd, 0, 0, 0 } },
		{ "Specular Color", { generated_mesh_param_type::COLOR_4, (void*)&cs, 0, 0, 0 } },
		{ "Ambient Coeff", { generated_mesh_param_type::FLOAT_1, (void*)&ka, 0, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "Diffuse Coeff", { generated_mesh_param_type::FLOAT_1, (void*)&kd, 0, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "Specular Coeff", { generated_mesh_param_type::FLOAT_1, (void*)&ks, 0, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
		{ "Specular Power", { generated_mesh_param_type::FLOAT_1, (void*)&sp, 0, MAX_PARAM_VALUE, DRAG_PARAM_STEP } },
	};
}
nlohmann::json light::save() const
{
	nlohmann::json obj;
	obj["n"] = name;
	obj["t"] = mat.e;
	obj["ca"] = nlohmann::json::array({ ca.r, ca.g, ca.b, ca.a });
	obj["cd"] = nlohmann::json::array({ cd.r, cd.g, cd.b, cd.a });
	obj["cs"] = nlohmann::json::array({ cs.r, cs.g, cs.b, cs.a });
	obj["ka"] = ka;
	obj["kd"] = kd;
	obj["ks"] = ks;
	obj["sp"] = sp;
	return obj;
}
