#pragma once
#include "pch.h"
#include "visibility_xportable.h"
#include "util/color.h"
#include "geom/generated_mesh.h"

enum class light_type
{
	NONE = -1,
	DIRECTION,
	POINT,
	AREA,
	SPOT,
	COUNT
};
static std::string light_type_string(const light_type l)
{
	switch (l)
	{
	case light_type::DIRECTION: return "Directional";
	case light_type::POINT: return "Point";
	case light_type::AREA: return "Area";
	case light_type::SPOT: return "Spotlight";
	}
	assert(false);
	return "";
}

class light : public visibility_xportable
{
public:
	mgl::light mgl_light;
public:
	light();
	light(const mgl::light& ml, const std::string& _name);
	light(const nlohmann::json& obj);
public:
	std::vector<std::pair<std::string, generated_mesh_param>> get_params() const;
	tmat<space::OBJECT, space::WORLD>& get_mat();
	const tmat<space::OBJECT, space::WORLD>& get_mat() const;
	light_type get_type() const;
	light* clone() const;
public:
	void set_mat(const tmat<space::OBJECT, space::WORLD>& m);
	void set_type(const light_type t);
public:
	nlohmann::json save() const override;
	void xport(mgl::output_file& out) const override;
private:
	light_type m_type;
};
