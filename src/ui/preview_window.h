#pragma once
#include "imgui_window.h"
#include "preview_layer.h"

class preview_window : public imgui_window
{
public:
	preview_window(const mgl::context& mgl_context, preview_layer* const layer);
	virtual ~preview_window() {}
public:
	virtual void handle_frame() override;
private:
	const mgl::context& m_mgl_context;
	preview_layer* m_layer;
};