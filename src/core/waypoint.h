#pragma once
#include "pch.h"
#include "visibility_xportable.h"

class waypoint : public visibility_xportable
{
public:
	waypoint();
	waypoint(const nlohmann::json& obj);
public:
	tmat<space::OBJECT, space::WORLD>& get_mat();
	const tmat<space::OBJECT, space::WORLD>& get_mat() const;
	waypoint* clone() const;
public:
	void set_mat(const tmat<space::OBJECT, space::WORLD>& m);
	void xport(mgl::output_file& out) const;
public:
	nlohmann::json save() const override;
private:
	tmat<space::OBJECT, space::WORLD> m_mat;
};
