#pragma once
#include "pch.h"

struct waypoint
{
public:
	std::string name = "Waypoint";
public:
	waypoint();
	waypoint(const nlohmann::json& obj);
public:
	static u32 get_next_id();
	static void set_next_id(const u32 id);
	static void reset_next_id();
public:
	tmat<space::OBJECT, space::WORLD>& get_mat();
	const tmat<space::OBJECT, space::WORLD>& get_mat() const;
	const std::string& get_id() const;
	const std::string& get_name() const;
public:
	void set_mat(const tmat<space::OBJECT, space::WORLD>& m);
	void set_name(const std::string& n);
public:
	nlohmann::json save() const;
private:
	constexpr static inline u32 s_first_id = 0;
	static inline u32 s_next_id = s_first_id;
private:
	std::string m_id;
	tmat<space::OBJECT, space::WORLD> m_mat;
};
