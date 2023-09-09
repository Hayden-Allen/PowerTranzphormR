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
	void handle_focused(const bool focused) override;
	void handle_frame() override;
	void set_renaming(sgnode* const node);
	const sgnode* get_renaming() const;
	void set_renaming_sm(smnode* const sm);
	const smnode* get_renaming_sm() const;
	void set_renaming_light(light* const l);
	const light* get_renaming_light() const;
private:
	rect handle_node(sgnode* const node);
	void handle_heightmap(smnode* const hmp);
	void handle_heightmaps();
	void handle_light(light* const l);
	void handle_lights();
private:
	sgnode* m_show_add_child = nullptr;
	sgnode* m_renaming = nullptr;
	bool m_rename_needs_focus = false;
	smnode* m_renaming_sm = nullptr;
	bool m_rename_sm_needs_focus = false;
	light* m_renaming_light = nullptr;
	bool m_rename_light_needs_focus = false;
	bool m_was_focused = false;
};
