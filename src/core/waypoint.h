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
public:
	void set_mat(const tmat<space::OBJECT, space::WORLD>& m);
public:
	nlohmann::json save() const override;
private:
	tmat<space::OBJECT, space::WORLD> m_mat;
};
