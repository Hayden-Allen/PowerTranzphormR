#pragma once
#include "imgui_window.h"
#include "preview_layer.h"
#include "imgui_layer.h"

class preview_window : public imgui_window
{
public:
	preview_window(const mgl::context& mgl_context, preview_layer* const pl, imgui_layer* const il);
	virtual ~preview_window() {}
public:
	virtual void handle_frame() override;
private:
	const mgl::context& m_mgl_context;
	preview_layer* m_preview_layer;
	imgui_layer* m_imgui_layer;
};