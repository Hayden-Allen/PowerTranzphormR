#pragma once
#include "pch.h"
#include "util/color.h"
#include "geom/generated_mesh.h"

struct light
{
	std::string name = "Light";
	tmat<space::OBJECT, space::WORLD> mat;
	// ambient, diffuse, specular
	color_t ca, cd, cs;
	// coefficients
	f32 ka = 0.f, kd = 0.f, ks = 0.f;
	// specular power
	f32 sp = 0.f;

	light() {}
	light(const nlohmann::json& obj);

	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const;
	nlohmann::json save() const;
};
