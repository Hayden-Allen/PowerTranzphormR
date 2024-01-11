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
waypoint* waypoint::clone() const
{
	waypoint* cloned = new waypoint;
	cloned->copy_properties_from(*this);
	cloned->set_mat(get_mat());
	return cloned;
}
void waypoint::set_mat(const tmat<space::OBJECT, space::WORLD>& m)
{
	m_mat = m;
}
void waypoint::xport(mgl::output_file& out) const
{
	out.write(m_mat.e, 16);
}
