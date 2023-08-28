#pragma once
#include "imgui_layer.h"

struct app_ctx;

class preview_layer : public mgl::layer
{
public:
	preview_layer(app_ctx* const a_ctx);
public:
	bool on_frame(const f32 dt) override;
	bool on_key(const s32 key, const s32 scancode, const s32 action, const s32 mods) override;
private:
	app_ctx* const m_app_ctx = nullptr;
};
