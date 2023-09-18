#include "pch.h"
#include "xportable.h"

xportable::xportable() :
	m_id(std::string("xp") + std::to_string(s_next_id++)),
	m_name("Xportable")
{}
xportable::xportable(const std::string& name) :
	m_id(std::string("xp") + std::to_string(s_next_id++)),
	m_name(name)
{}
xportable::xportable(const nlohmann::json& obj) :
	m_id(obj["id"]),
	m_name(obj["name"])
{
	assert(!s_used_kustomz.contains(obj["kustom_id"]));
	kustomize_display(obj["kustom_id"]);
	for (const auto& s : obj["tagz"])
	{
		tag new_tag = { s[0], s[1] };
		push_tag(new_tag);
	}
}
xportable::~xportable()
{
	for (const auto& t : m_tagz)
	{
		decrement_tag_refcount(t);
	}
	kustomize_display("");
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
u32 xportable::get_num_tags_created()
{
	return s_num_tags_created;
}
void xportable::set_num_tags_created(const u32 id)
{
	s_num_tags_created = id;
}
void xportable::reset_num_tags_created()
{
	s_num_tags_created = 0;
}
std::vector<const char*> xportable::get_tag_suggestions(const std::string& s)
{
	std::vector<const char*> result;
	if (s.size() == 0)
	{
		return result;
	}
	for (const auto& c : s_used_tagz)
	{
		bool matched = false;
		const size_t s_size = s.size();
		if (s_size <= c.first.name.size())
		{
			matched = true;
			for (size_t i = 0; i < s_size; ++i)
			{
				if (std::tolower(c.first.name[i]) != std::tolower(s[i]))
				{
					matched = false;
					break;
				}
			}
		}
		if (matched)
		{
			result.emplace_back(c.first.name.c_str());
		}
	}
	return result;
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
const std::set<xportable::tag, xportable::tag_comparator>& xportable::get_tagz() const
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
void xportable::push_tag(const std::string& s)
{
	// number doesn't matter, comparator doesn't care
	tag new_tag = {
		s,
		s_num_tags_created + 1,
	};
	const auto& it = s_used_tagz.find(new_tag);
	if (it != s_used_tagz.end())
	{
		new_tag.id = it->first.id;
	}
	else
		s_num_tags_created++;

	if (m_tagz.contains(new_tag))
	{
		return;
	}
	++s_used_tagz[new_tag];
	m_tagz.insert(new_tag);
}
void xportable::erase_tag(const tag& t)
{
	decrement_tag_refcount(t);
	m_tagz.erase(t);
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
	nlohmann::json::array_t tagz;
	for (const auto& tag : m_tagz)
	{
		tagz.push_back(nlohmann::json::array({ tag.name, tag.id }));
	}
	obj["tagz"] = tagz;
	obj["name"] = m_name;
	return obj;
}
void xportable::copy_properties_from(const xportable& src)
{
	for (const auto& t : src.get_tagz())
	{
		push_tag(t);
	}
	set_name(src.get_name());
}



void xportable::decrement_tag_refcount(const tag& t)
{
	--s_used_tagz[t];
	if (s_used_tagz[t] == 0)
	{
		s_used_tagz.erase(t);
	}
}



void xportable::push_tag(const tag& t)
{
	++s_used_tagz[t];
	m_tagz.insert(t);
}
