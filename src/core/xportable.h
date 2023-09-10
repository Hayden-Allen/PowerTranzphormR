#pragma once

#include "pch.h"

class xportable
{
private:
private:
	struct CaseInsensitiveCompare
	{
		bool operator()(const std::string& a, const std::string& b) const
		{
			return strcasecmp(a.c_str(), b.c_str()) < 0;
		}
	};
public:
	xportable();
	xportable(const std::string& name);
	xportable(const nlohmann::json& obj);
	virtual ~xportable();
public:
	static u32 get_next_id();
	static void set_next_id(const u32 id);
	static void reset_next_id();
public:
	const std::string& get_id() const;
	const std::string& get_kustom_id() const;
	const std::string& get_kustom_display() const;
	const bool get_kustom_id_conflict() const;
	const std::string& get_name() const;
	const std::set<std::string, CaseInsensitiveCompare>& get_tagz() const;
	void kustomize_display(const std::string& s);
	void push_tag(const std::string& s);
	void erase_tag(const std::string& s);
	void set_name(const std::string& n);
	nlohmann::json save() const;
private:
	constexpr static inline u32 s_first_id = 0;
	static inline u32 s_next_id = s_first_id;
	static inline std::unordered_set<std::string> s_used_kustomz = {};
	static inline std::unordered_map<std::string, u32> s_used_tagz = {};
private:
	static void decrement_tag_refcount(const std::string& s);
private:
	std::string m_id;
	std::string m_kustom_id, m_kustom_display;
	std::string m_name;
	std::set<std::string, CaseInsensitiveCompare> m_tagz;
};
