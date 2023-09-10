#pragma once

#include "pch.h"

class xportable
{
public:
	xportable();
	xportable(const std::string& name);
	xportable(const nlohmann::json& obj);
	~xportable();
public:
	static u32 get_next_id();
	static void set_next_id(const u32 id);
	static void reset_next_id();
public:
	const std::string& get_id() const;
	const std::string& get_name() const;
	void set_name(const std::string& n);
	nlohmann::json save() const;
private:
	constexpr static inline u32 s_first_id = 0;
	static inline u32 s_next_id = s_first_id;
private:
	std::string m_id;
	std::string m_name;
};
