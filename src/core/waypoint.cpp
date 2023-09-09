#include "pch.h"
#include "waypoint.h"


waypoint::waypoint() :
	m_id(std::string("way") + std::to_string(s_next_id++))
{
}
waypoint::waypoint(const nlohmann::json& obj) :
	name(obj["n"]),
	m_id(obj["id"])
{
	m_mat = u::json2tmat<space::OBJECT, space::WORLD>(obj["t"]);
}



u32 waypoint::get_next_id()
{
	return s_next_id;
}
void waypoint::set_next_id(const u32 id)
{
	s_next_id = id;
}
void waypoint::reset_next_id()
{
	s_next_id = s_first_id;
}



nlohmann::json waypoint::save() const
{
	nlohmann::json obj;
	obj["id"] = m_id;
	obj["n"] = name;
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
const std::string& waypoint::get_name() const
{
	return name;
}
void waypoint::set_name(const std::string& n)
{
	name = n;
}
const std::string& waypoint::get_id() const
{
	return m_id;
}