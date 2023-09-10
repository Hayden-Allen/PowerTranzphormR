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
	m_name(obj["name"]),
	m_kustom_id(obj["kustom_id"]),
	m_kustom_display(obj["kustom_id"])
{
	assert(!s_used_kustomz.contains(m_kustom_id));
	s_used_kustomz.insert(m_kustom_id);
	for (const auto& s : obj["tagz"])
	{
		m_tagz.emplace_back(s);
	}
}
xportable::~xportable()
{
	if (!m_kustom_id.empty())
	{
		s_used_kustomz.erase(m_kustom_id);
	}
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
const std::string& xportable::get_kustom_id() const
{
	return m_kustom_id;
}
const std::string& xportable::get_kustom_display() const
{
	return m_kustom_display;
}
const bool xportable::get_kustom_id_conflict() const
{
	return m_kustom_display != m_kustom_id;
}
const std::string& xportable::get_name() const
{
	return m_name;
}
const std::vector<std::string>& xportable::get_tagz() const
{
	return m_tagz;
}
void xportable::kustomize_display(const std::string& s)
{
	if (!s_used_kustomz.contains(s))
	{
		if (!m_kustom_id.empty())
		{
			s_used_kustomz.erase(m_kustom_id);
		}
		if (!s.empty())
		{
			s_used_kustomz.insert(s);
		}
		m_kustom_id = s;
	}
	m_kustom_display = s;
}
void xportable::set_name(const std::string& n)
{
	m_name = n;
}
nlohmann::json xportable::save() const
{
	nlohmann::json obj;
	obj["id"] = m_id;
	obj["kustom_id"] = m_kustom_id;
	obj["tagz"] = m_tagz;
	obj["name"] = m_name;
	return obj;
}
