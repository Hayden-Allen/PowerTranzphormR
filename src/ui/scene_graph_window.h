#pragma once
#include "scene_ctx.h"
#include "imgui_window.h"

class preview_layer;

class scene_graph_window : public imgui_window
{
	using Rect = std::pair<ImVec2, ImVec2>;
public:
	scene_graph_window(scene_ctx* const scene);
	virtual ~scene_graph_window() {}
public:
	virtual void handle_frame() override;
private:
	scene_ctx* m_scene;
private:
	static std::string operation_to_string(carve::csg::CSG::OP op);
private:
	Rect handle_node(sgnode* const node) const;
};
