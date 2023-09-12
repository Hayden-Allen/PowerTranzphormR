#include "pch.h"
#include "waypoint.h"


waypoint::waypoint() :
	visibility_xportable(std::string("Waypoint"))
{
}
waypoint::waypoint(const nlohmann::json& obj) :
	visibility_xportable(obj)
{
	m_mat = u::json2tmat<space::OBJECT, space::WORLD>(obj["t"]);
}



nlohmann::json waypoint::save() const
{
	nlohmann::json obj = visibility_xportable::save();
	obj["t"] = m_mat.e;
	return obj;
}
tmat<space::OBJECT, space::WORLD>& waypoint::get_mat()
{
	return m_mat;
}
const tmat<space::OBJECT, space::WORLD>& waypoint::get_mat() const
{
	return m_mat;
}
void waypoint::set_mat(const tmat<space::OBJECT, space::WORLD>& m)
{
	m_mat = m;
}