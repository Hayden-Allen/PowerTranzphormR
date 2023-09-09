#pragma once
#include "pch.h"
#include "util/color.h"
#include "geom/generated_mesh.h"

struct light
{
public:
	mgl::light mgl_light;
	std::string name = "Light";
public:
	light() {}
	light(const nlohmann::json& obj);
public:
	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const;
	nlohmann::json save() const;
	tmat<space::OBJECT, space::WORLD>& get_mat();
	const tmat<space::OBJECT, space::WORLD>& get_mat() const;
	void set_mat(const tmat<space::OBJECT, space::WORLD>& m);
};
