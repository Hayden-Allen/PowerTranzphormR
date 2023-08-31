#pragma once
#include "imgui_window.h"

struct scene_material;

class materials_list_window : public imgui_window
{
public:
	materials_list_window(app_ctx* const a_ctx);
	virtual ~materials_list_window() {}
public:
	virtual void handle_focused(bool focused) override;
	virtual void handle_frame() override;
	void set_renaming(scene_material* mtl);
	const scene_material* get_renaming() const;
private:
	scene_material* m_renaming = nullptr;
	bool m_rename_needs_focus = false;
	bool m_was_focused = false;
};
