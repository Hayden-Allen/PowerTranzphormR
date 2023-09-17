#pragma once
#include "pch.h"
#include "core/scene_ctx.h"
#include "app_ctx.h"
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
	void set_renaming_waypoint(waypoint* const w);
	const waypoint* get_renaming_waypoint() const;
private:
	rect handle_node(sgnode* const node, bool& performed_destructive_action);
	void handle_heightmap(smnode* const hmp, bool& performed_destructive_action);
	void handle_heightmaps(bool& performed_destructive_action);
	void handle_light(const u32 index, bool& performed_destructive_action);
	void handle_lights(bool& performed_destructive_action);
	void handle_waypoint(waypoint* const w, bool& performed_destructive_action);
	void handle_waypoints(bool& performed_destructive_action);
private:
	sgnode* m_show_add_child = nullptr;
	sgnode* m_renaming = nullptr;
	bool m_rename_needs_focus = false;
	smnode* m_renaming_sm = nullptr;
	bool m_rename_sm_needs_focus = false;
	light* m_renaming_light = nullptr;
	bool m_rename_light_needs_focus = false;
	waypoint* m_renaming_waypoint = nullptr;
	bool m_rename_waypoint_needs_focus = false;
	bool m_was_focused = false;
	global_selection_type m_prev_sel_type = global_selection_type::sgnode;
};
