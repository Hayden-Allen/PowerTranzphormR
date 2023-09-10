#include "pch.h"
#include "xportable.h"

xportable::xportable() :
	m_id(std::string("xp") + std::to_string(s_next_id++)),
	m_name("Xportable")
{
}
xportable::xportable(const std::string& name) :
	m_id(std::string("xp") + std::to_string(s_next_id++)),
	m_name(name)
{
	//
}
xportable::xportable(const nlohmann::json& obj) :
	m_id(obj["id"]),
	m_name(obj["name"])
{
}
xportable::~xportable()
{
}

u32 xportable::get_next_id()
{
	return s_next_id;
}
void xportable::set_next_id(const u32 id)
{
	s_next_id = id;
}
void xportable::reset_next_id()
{
	s_next_id = s_first_id;
}

const std::string& xportable::get_id() const
{
	return m_id;
}
const std::string& xportable::get_name() const
{
	return m_name;
}
void xportable::set_name(const std::string& n)
{
	m_name = n;
}
nlohmann::json xportable::save() const
{
	nlohmann::json obj;
	obj["id"] = m_id;
	obj["name"] = m_name;
	return obj;
}
