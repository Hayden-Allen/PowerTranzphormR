#pragma once
#include "pch.h"

struct shortcut_menu_item;
typedef std::vector<shortcut_menu_item> shortcut_menu_item_group;

struct shortcut_menu_item
{
	std::string name;
	std::function<void()> handler = []() {};
	std::string keys_text;
	s32 key = -1, mods = -1;
	std::vector<shortcut_menu_item_group> groups;
};

struct shortcut_menu
{
	std::string name;
	std::vector<shortcut_menu_item_group> groups;
};
