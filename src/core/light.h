#pragma once
#include "pch.h"
#include "util/color.h"
#include "geom/generated_mesh.h"

struct light
{
	static u32 get_next_id()
	{
		return s_next_id;
	}
	static void set_next_id(const u32 id)
	{
		s_next_id = id;
	}
	static void reset_next_id()
	{
		s_next_id = s_first_id;
	}


	light();
	light(const nlohmann::json& obj);

	std::string name = "Light";
	tmat<space::OBJECT, space::WORLD> mat;
	// ambient, diffuse, specular
	color_t ca, cd, cs;
	// coefficients
	f32 ka = 0.f, kd = 0.f, ks = 0.f;
	// specular power
	f32 sp = 0.f;

	const std::string& get_name() const { return name; }
	void set_name(const std::string& n) { name = n; }
	const std::string& get_id() const { return m_id; }

	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const;
	nlohmann::json save() const;

private:
	constexpr static inline u32 s_first_id = 0;
	static inline u32 s_next_id = s_first_id;
private:
	std::string m_id;
};
