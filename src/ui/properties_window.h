#pragma once
#include "imgui_window.h"

class properties_window : public imgui_window
{
public:
	properties_window(app_ctx* const a_ctx);
	virtual ~properties_window() {}
public:
	virtual void handle_frame() override;
private:
	void handle_snapping_angle();
	void handle_transform(sgnode* const selected);
	void handle_mesh(sgnode* const selected);
};
