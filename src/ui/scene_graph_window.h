#pragma once
#include "pch.h"
#include "core/scene_ctx.h"
#include "imgui_window.h"

class scene_graph_window : public imgui_window
{
	using rect = std::pair<ImVec2, ImVec2>;
public:
	scene_graph_window(app_ctx* const a_ctx);
	virtual ~scene_graph_window() {}
public:
	virtual void handle_focused(bool focused) override;
	virtual void handle_frame() override;
	void set_renaming(sgnode* const node);
	const sgnode* get_renaming() const;
private:
	rect handle_node(sgnode* const node);
	void handle_lights();
private:
	sgnode* m_show_add_child = nullptr;
	sgnode* m_renaming = nullptr;
	bool m_rename_needs_focus = false;
	bool m_was_focused = false;
};
