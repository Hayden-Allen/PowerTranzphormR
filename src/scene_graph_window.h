#pragma once
#include "scene_ctx.h"
#include "imgui_window.h"

class scene_graph_window : public imgui_window
{
public:
	scene_graph_window();
	virtual ~scene_graph_window() {}
public:
	virtual void handle_frame() override;
private:
	//
};
