#pragma once
#include "app_ctx.h"

class shortcut_menus_layer : public mgl::layer
{
public:
	shortcut_menus_layer(app_ctx* const a_ctx);
	virtual ~shortcut_menus_layer();
public:
	virtual void on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods) override;
private:
	app_ctx* const m_app_ctx = nullptr;
};
