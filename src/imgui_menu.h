#pragma once
#include "pch.h"

struct imgui_menu_item {
	std::string name;
	std::function<void()> handler = [](){};
	std::string shortcut_text;
	std::vector<u32> shortcut_keys;
};

typedef std::vector<imgui_menu_item> imgui_menu_item_group;

struct imgui_menu {
	std::string name;
	std::vector<imgui_menu_item_group> groups;
};
