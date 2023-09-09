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
	light();
	light(const mgl::light& ml, const std::string& _name);
	light(const nlohmann::json& obj);
public:
	static u32 get_next_id();
	static void set_next_id(const u32 id);
	static void reset_next_id();
public:
	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const;
	tmat<space::OBJECT, space::WORLD>& get_mat();
	const tmat<space::OBJECT, space::WORLD>& get_mat() const;
	const std::string& get_id() const;
	const std::string& get_name() const;
public:
	void set_mat(const tmat<space::OBJECT, space::WORLD>& m);
	void set_name(const std::string& n);
public:
	nlohmann::json save() const;
private:
	constexpr static inline u32 s_first_id = 0;
	static inline u32 s_next_id = s_first_id;
private:
	std::string m_id;
};
