#pragma once
#include "pch.h"
#include "xportable.h"

class waypoint : public xportable
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
	nlohmann::json save() const;
private:
	tmat<space::OBJECT, space::WORLD> m_mat;
};
