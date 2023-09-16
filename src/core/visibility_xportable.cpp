#include "pch.h"
#include "visibility_xportable.h"

visibility_xportable::visibility_xportable()
{}
visibility_xportable::visibility_xportable(const std::string& name) :
	xportable(name)
{}
visibility_xportable::visibility_xportable(const nlohmann::json& obj) :
	xportable(obj)
{
	m_visible = obj["visible"];
}



void visibility_xportable::set_visibility(const bool v)
{
	m_visible = v;
}
bool visibility_xportable::is_visible() const
{
	return m_visible;
}
nlohmann::json visibility_xportable::save() const
{
	nlohmann::json obj = xportable::save();
	obj["visible"] = m_visible;
	return obj;
}
void visibility_xportable::copy_properties_from(const visibility_xportable& src)
{
	xportable::copy_properties_from(src);
	set_visibility(src.is_visible());
}
