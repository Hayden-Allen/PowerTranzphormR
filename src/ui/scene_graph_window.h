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
private:
	Rect handle_node(sgnode* const node, const bool parent_cutted_to_clipboard) const;
private:
	sgnode* m_show_add_child = nullptr;
};
