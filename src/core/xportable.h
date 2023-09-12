#pragma once
#include "pch.h"

class xportable
{
public:
	struct tag
	{
		std::string name;
		u32 id;
	};
	struct tag_comparator
	{
		bool operator()(const tag& a, const tag& b) const
		{
			return strcasecmp(a.name.c_str(), b.name.c_str()) < 0;
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
	static u32 get_num_tags_created();
	static void set_num_tags_created(const u32 id);
	static void reset_num_tags_created();
	static std::vector<const char*> get_tag_suggestions(const std::string& s);
public:
	const std::string& get_id() const;
	const std::string& get_kustom_id() const;
	const std::string& get_kustom_display() const;
	const bool get_kustom_id_conflict() const;
	const std::string& get_name() const;
	const std::set<tag, tag_comparator>& get_tagz() const;
	void kustomize_display(const std::string& s);
	void push_tag(const std::string& s);
	void erase_tag(const tag& t);
	void set_name(const std::string& n);
	virtual nlohmann::json save() const;
private:
	constexpr static inline u32 s_first_id = 0;
	static inline u32 s_next_id = s_first_id;
	static inline u32 s_num_tags_created = 0;
	static inline std::unordered_set<std::string> s_used_kustomz = {};
	static inline std::map<tag, u32, tag_comparator> s_used_tagz = {};
private:
	std::string m_id;
	std::string m_kustom_id, m_kustom_display;
	std::string m_name;
	std::set<tag, tag_comparator> m_tagz;
private:
	static void decrement_tag_refcount(const tag& t);
	void push_tag(const tag& t);
};
