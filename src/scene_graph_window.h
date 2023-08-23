#pragma once
#include "scene_ctx.h"
#include "imgui_window.h"

class scene_graph_window : public imgui_window
{
public:
	scene_graph_window(scene_ctx* scene);
	virtual ~scene_graph_window() {}
public:
	virtual void handle_frame() override;
private:
	void handle_node(sgnode* node);
	std::string operation_to_string(carve::csg::CSG::OP op);
private:
	scene_ctx* m_scene = nullptr;
};
