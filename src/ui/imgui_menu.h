#pragma once
#include "pch.h"

struct imgui_menu_item
{
	std::string name;
	std::function<void()> handler = []() {};
	std::string shortcut_text;
	s32 shortcut_key = -1, shortcut_mods = -1;
};

typedef std::vector<imgui_menu_item> imgui_menu_item_group;

struct imgui_menu
{
	std::string name;
	std::vector<imgui_menu_item_group> groups;
};
