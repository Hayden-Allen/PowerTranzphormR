#include "pch.h"
#include "shortcut_menus_layer.h"
#include "app_ctx.h"

shortcut_menus_layer::shortcut_menus_layer(app_ctx* const a_ctx) :
	m_app_ctx(a_ctx)
{
}

shortcut_menus_layer::~shortcut_menus_layer()
{
}



bool shortcut_menus_layer::on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods)
{
	// This should ideally be WantCaptureKeyboard, but that seems to be true pretty much all the time
	// The goal here is that at least we can prevent Delete/Backspace in text fields from triggering menu items
	if (!ImGui::GetIO().WantTextInput && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		for (const shortcut_menu& menu : m_app_ctx->shortcut_menus)
		{
			for (const shortcut_menu_item_group& group : menu.groups)
			{
				for (const shortcut_menu_item& item : group)
				{
					if (item.enabled())
					{
						if (handle_key_menu_item(key, mods, item))
						{
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

bool shortcut_menus_layer::handle_key_menu_item(const s32 key, const s32 mods, const shortcut_menu_item& item)
{
	if (item.groups.size())
	{
		for (const shortcut_menu_item_group& group : item.groups)
		{
			for (const shortcut_menu_item& inner : group)
			{
				if (inner.enabled())
				{
					handle_key_menu_item(key, mods, inner);
				}
			}
		}
	}
	else
	{
		if (key == item.key && item.mods == (mods & 0x7))
		{
			item.handler();
			return true;
		}
	}
	return false;
}
