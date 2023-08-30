#pragma once
#include "scene_ctx.h"
#include "imgui_window.h"

class scene_graph_window : public imgui_window
{
	using Rect = std::pair<ImVec2, ImVec2>;
public:
	scene_graph_window(app_ctx* const a_ctx);
	virtual ~scene_graph_window() {}
public:
	virtual void handle_frame() override;
	void set_renaming(sgnode* const node);
	const sgnode* get_renaming() const;
private:
	Rect handle_node(sgnode* const node);
private:
	sgnode* m_show_add_child = nullptr;
	sgnode* m_renaming = nullptr;
	bool m_rename_needs_focus = false;
	bool m_was_focused = false;
};
